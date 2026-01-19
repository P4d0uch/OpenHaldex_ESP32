#include "openhaldex.h"

// Debug printing (disable in production by commenting out)
#define DEBUG_PRINT_ENABLED
#ifdef DEBUG_PRINT_ENABLED
#define DEBUG_PRINT(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...) \
  do                          \
  {                           \
  } while (0)
#endif

/**
 * Print CAN frame data for debugging
 */
void printCanFrame(const char *prefix, const can_frame &frame)
{
  DEBUG_PRINT("%s ID[0x%03X]: ", prefix, frame.id);
  for (int i = 0; i < frame.len; i++)
  {
    DEBUG_PRINT("%02X ", frame.data.bytes[i]);
  }
  DEBUG_PRINT("\n");
}

/**
 * Process incoming Chassis CAN frame and get data from it
 * Here the modification to the data for haldex are made
 */
void processChassisFrame(can_frame &frame)
{
  lastCANChassisTick = millis();
  switch (frame.id)
  {
  case MOTOR1_ID:
    // Pedal is 0-250, scale to 0-100%
    ped_value = frame.data.bytes[5] * 0.4;
    vehicle_RPM = ((frame.data.bytes[3] << 8) | frame.data.bytes[2]) * 0.25;
    DEBUG_PRINT("Vehicle RPM: %d, Pedal: %d\n", vehicle_RPM, ped_value);
    break;

  case MOTOR2_ID:
    // Speed is in km/h directly, byte 3 
    vehicle_speed = frame.data.bytes[3] * 128 / 100;
    DEBUG_PRINT("Vehicle speed: %d km/h\n", vehicle_speed);
    break;
  }

  // Apply lock data modifications if not in STOCK mode
  if (state.mode != MODE_STOCK)
  {
    modifyHaldexFrame(frame);
  }
  else
  {
    // In STOCK mode, the frame is not modified - this value is just for HTML UI
    lock_target = 0;
  }
}

/**
 * Process incoming Haldex CAN frame just to get data from it.
 * No modifications are made here.
 */
void processHaldexFrame(can_frame &frame)
{
  lastCANHaldexTick = millis();

  // Just for safety and testing
  if (frame.id == 0)
    return;

  // Scale engagement :127 is when stationary
  received_haldex_engagement_raw = frame.data.bytes[1];
  haldex_engagement = map(received_haldex_engagement_raw, 127, 198, 0, 100);
    

  // Decode state byte
  received_haldex_state = frame.data.bytes[0];
  received_report_clutch1 = (received_haldex_state & (1 << 0));
  received_temp_protection = (received_haldex_state & (1 << 1));
  received_report_clutch2 = (received_haldex_state & (1 << 2));
  received_coupling_open = (received_haldex_state & (1 << 3));
  received_status_bit_4 = (received_haldex_state & (1 << 4));
  received_status_bit_5 = (received_haldex_state & (1 << 5));
  received_speed_limit = (received_haldex_state & (1 << 6));
  received_status_bit_7 = (received_haldex_state & (1 << 7));
}

/**
 * CAN Chassis interrupt handler
 * Just signal the main task to start processing new frame
 */
 
void IRAM_ATTR parseCan_chs_handler()
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  vTaskNotifyGiveFromISR(body_can.comms_task, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**
 * Main CAN Chassis processing task
 * It waits for interrupt signals and processes incoming frames
 * It also handles queue for the outgoing frames to the Haldex unit
 */
void parseCAN_chs(void *arg)
{
  can_frame frame;
  while (1)
  {
    // Wait for interrupt signal OR timeout every 10 ms
    ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(10));
    // New message from ISR
    if (body_can.can_interface->readMsgBuf(&frame.id, &frame.len, frame.data.bytes) == CAN_OK)
    {
      printCanFrame("RX Body - Original", frame);
      processChassisFrame(frame);
      printCanFrame("RX Body - Modified", frame);
      xQueueSendToBack(haldex_can.outbox, &frame, 0);
    }

    // Always check the outbox periodically
    while (xQueueReceive(body_can.outbox, &frame, 0) == pdTRUE && !replayWithoutSending)
    {
      byte result = body_can.can_interface->sendMsgBuf(frame.id, frame.len, frame.data.bytes);
      if (result != CAN_OK)
      {
        DEBUG_PRINT("[%03X]: TX NOK Body (%d)\n", frame.id, result);
        xQueueSendToBack(body_can.outbox, &frame, 0); // requeue for retry
      }
    }
  }
}

/**
 * CAN Haldex interrupt handler
 * Just signal the main task to start processing new frame
 */
void IRAM_ATTR parseCan_hdx_handler()
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  vTaskNotifyGiveFromISR(haldex_can.comms_task, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**
 * Main CAN Haldex processing task
 * It waits for interrupt signals and basically just resends the frame to oubox for body CAN.
 */
void parseCAN_hdx(void *arg)
{
  can_frame frame;

  while (1)
  {
    // Wait for interrupt signal OR timeout every 10 ms
    ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(10));
    // New Haldex message available
    if (haldex_can.can_interface->readMsgBuf(&frame.id, &frame.len, frame.data.bytes) == CAN_OK)
    {
      printCanFrame("RX Haldex - Original", frame);
      processHaldexFrame(frame);
      printCanFrame("RX Haldex - Modified", frame);
      xQueueSendToBack(body_can.outbox, &frame, 0);
    }

    // Always check the outbox periodically
    while (xQueueReceive(haldex_can.outbox, &frame, 0) == pdTRUE && !replayWithoutSending)
    {
      byte result = haldex_can.can_interface->sendMsgBuf(frame.id, frame.len, frame.data.bytes);
      if (result != CAN_OK)
      {
        DEBUG_PRINT("[%03X]: TX NOK Haldex (%d)\n", frame.id, result);
        xQueueSendToBack(haldex_can.outbox, &frame, 0);
      }
    }
  }
}

void send_test_data(void *params)
{
  while (1)
  {

    can_frame frame;
    byte result;
    frame.id = MOTOR1_ID;                                              // 0x1A0
    frame.len = 8;                                                     // DLC 8
    frame.data.bytes[0] = 0x00;                                        // various individual bits ('space gas', driving pedal, kick down, clutch, timeout brake, brake intervention, drinks-torque intervention?) was 0x01 - ignored
    frame.data.bytes[1] = get_lock_target_adjusted_value(0xFE, false); // inner engine moment (%): 0.39*(0xF0 / 250) = 97.5% (93.6% <> 0xf0) - used - from 0x00>0xFE adds 30%
    frame.data.bytes[2] = 0x21;                                        // motor speed (rpm): 32 > (low byte) - ignored
    frame.data.bytes[3] = get_lock_target_adjusted_value(0x4E, false); // motor speed (rpm): 78 > (high byte) : 0.25 * (78 32) = 1,958 RPM (was 0x4e)  Runs pre-charge pump if >0 has a good impact on lock.  Used
    frame.data.bytes[4] = get_lock_target_adjusted_value(0xFE, false); // inner moment (%): 0.39*(0xF0) = 93.6%  (make FE?) - ignored
    frame.data.bytes[5] = get_lock_target_adjusted_value(0xFE, false); // driving pedal (%): 0.39*(0xF0) = 93.6%  (make FE?) - ignored
    // rx_message_chs.data.bytes[6] = get_lock_target_adjusted_value(0x16, false);  // set to a low value to control the req. transfer torque.  Main control value for Gen1

    switch (state.mode)
    {
    case MODE_FWD:
      appliedTorque = get_lock_target_adjusted_value(0xFE, true); // return 0xFE to disable
      break;
    case MODE_5050:
      appliedTorque = get_lock_target_adjusted_value(0x16, false); // return 0x16 to fully lock
      break;
    case MODE_7525:
      appliedTorque = get_lock_target_adjusted_value(0x50, false); // set to ~30% lock (0x96 = 15%) (0x96 = 15%, 0x56 = 27%)
      break;
    }

    frame.data.bytes[6] = appliedTorque; // was 0x00
    frame.data.bytes[7] = 0x00;          // drivers moment (%): 0.39*(0xF0) = 93.6%  (make FE?) - ignored

    xQueueSendToBack(haldex_can.outbox, &frame, portMAX_DELAY);
    xQueueSendToBack(body_can.outbox, &frame, portMAX_DELAY);

    frame.id = MOTOR2_ID;                                              // 0x1A0
    frame.len = 8;                                                     // DLC 8
    frame.data.bytes[0] = 0x00;                                        // various individual bits ('space gas', driving pedal, kick down, clutch, timeout brake, brake intervention, drinks-torque intervention?) was 0x01 - ignored
    frame.data.bytes[1] = get_lock_target_adjusted_value(0xFE, false); // inner engine moment (%): 0.39*(0xF0 / 250) = 97.5% (93.6% <> 0xf0) - used - from 0x00>0xFE adds 30%
    frame.data.bytes[2] = 0x21;                                        // motor speed (rpm): 32 > (low byte) - ignored
    frame.data.bytes[3] = debug_speed++;                               // motor speed (rpm): 78 > (high byte) : 0.25 * (78 32) = 1,958 RPM (was 0x4e)  Runs pre-charge pump if >0 has a good impact on lock.  Used
    if (debug_speed > 200)
    {
      debug_speed = 0;
    }
    frame.data.bytes[4] = get_lock_target_adjusted_value(0xFE, false); // inner moment (%): 0.39*(0xF0) = 93.6%  (make FE?) - ignored
    frame.data.bytes[5] = get_lock_target_adjusted_value(0xFE, false); // driving pedal (%): 0.39*(0xF0) = 93.6%  (make FE?) - ignored
    // rx_message_chs.data.bytes[6] = get_lock_target_adjusted_value(0x16, false);  // set to a low value to control the req. transfer torque.  Main control value for Gen1

    switch (state.mode)
    {
    case MODE_FWD:
      appliedTorque = get_lock_target_adjusted_value(0xFE, true); // return 0xFE to disable
      break;
    case MODE_5050:
      appliedTorque = get_lock_target_adjusted_value(0x16, false); // return 0x16 to fully lock
      break;
    case MODE_7525:
      appliedTorque = get_lock_target_adjusted_value(0x50, false); // set to ~30% lock (0x96 = 15%) (0x96 = 15%, 0x56 = 27%)
      break;
    case MODE_CUSTOM:
      appliedTorque = getLockForSpeed(state.custom_mode, vehicle_speed); // set to ~30% lock (0x96 = 15%) (0x96 = 15%, 0x56 = 27%)
      break;
    }

    frame.data.bytes[6] = appliedTorque; // was 0x00
    frame.data.bytes[7] = 0x00;          // drivers moment (%): 0.39*(0xF0) = 93.6%  (make FE?) - ignored
    xQueueSendToBack(haldex_can.outbox, &frame, portMAX_DELAY);
    xQueueSendToBack(body_can.outbox, &frame, portMAX_DELAY);

    frame.id = MOTOR3_ID;       // 0x1A0
    frame.len = 8;              // DLC 8
    frame.data.bytes[0] = 0x00; // various individual bits ('motor has been launched, only in diesel') - ignored
    frame.data.bytes[1] = 0x50; // outdoor temperature - ignored
    frame.data.bytes[2] = 0x00; // pedal - ignored
    frame.data.bytes[3] = 0x00; // wheel command torque (low byte).  If SY_ASG - ignored
    frame.data.bytes[4] = 0x00; // wheel command torque (high byte).  If SY_ASG - ignored
    frame.data.bytes[5] = 0x00; // wheel command torque (0 = positive, 1 = negative).  If SY_ASG - ignored
    frame.data.bytes[6] = 0x00; // req. torque.  If SY_ASG - ignored
    frame.data.bytes[7] = 0xFE; // throttle angle (100%), ignored
    xQueueSendToBack(haldex_can.outbox, &frame, portMAX_DELAY);
    xQueueSendToBack(body_can.outbox, &frame, portMAX_DELAY);

    frame.id = BRAKES1_ID;                                             // 0x1A0
    frame.len = 8;                                                     // DLC 8
    frame.data.bytes[0] = 0x80;                                        // asr req
    frame.data.bytes[1] = get_lock_target_adjusted_value(0x00, false); // also controlling slippage.  Brake force can add 20%
    frame.data.bytes[2] = 0x00;                                        //  ignored
    frame.data.bytes[3] = 0x0A;                                        // 0xA ignored?
    frame.data.bytes[4] = 0xFE;                                        // ignored
    frame.data.bytes[5] = 0xFE;                                        // ignored
    frame.data.bytes[6] = 0x00;                                        // ignored
    frame.data.bytes[7] = BRAKES1_counter;                             // checksum
    if (++BRAKES1_counter > 0xF)
    {                      // 0xF
      BRAKES1_counter = 0; // 0
    }

    xQueueSendToBack(haldex_can.outbox, &frame, portMAX_DELAY);
    xQueueSendToBack(body_can.outbox, &frame, portMAX_DELAY);

    /*
    Bremse 3 has massive effect - deviation between front/rear = block 011 'rpm changing'
    too high a speed causes 'pulsing'
    slip control changes from 0 to 1 IF rear > front
    */

    frame.id = BRAKES3_ID;                                             // 0x4A0 - deviation between front/rear = block 011 'rpm'
    frame.len = 8;                                                     // DLC 8
    frame.data.bytes[0] = get_lock_target_adjusted_value(0xFE, false); // low byte, LEFT Front // affects slightly +2
    frame.data.bytes[1] = 0x0A;                                        // high byte, LEFT Front big effect
    frame.data.bytes[2] = get_lock_target_adjusted_value(0xFE, false); // low byte, RIGHT Front// affects slightly +2
    frame.data.bytes[3] = 0x0A;                                        // front right high
    frame.data.bytes[4] = 0x00;                                        // low byte, LEFT Rear
    frame.data.bytes[5] = 0x0A;                                        // high byte, LEFT Rear // 254+10? (5050 returns 0xA)
    frame.data.bytes[6] = 0x00;                                        // low byte, RIGHT Rear
    frame.data.bytes[7] = 0x0A;                                        // high byte, RIGHT Rear  // 254+10?
    xQueueSendToBack(haldex_can.outbox, &frame, portMAX_DELAY);
    xQueueSendToBack(body_can.outbox, &frame, portMAX_DELAY);

    vTaskDelay(50 / portTICK_PERIOD_MS); // wait 200ms before sending next set of test data
  }
}
