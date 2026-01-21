#include "openhaldex.h"

// --- Global Variables ---
// Initialize the variables
openhaldex_state state;
byte haldex_engagement;
byte haldex_engagement_raw;
TaskHandle_t haldex_status_update_task;
TaskHandle_t serial_command_task;

uint8_t received_haldex_state = 0;
uint8_t received_haldex_engagement_raw = 0;
uint8_t received_haldex_engagement = 0;
uint8_t appliedTorque = 0;

bool received_report_clutch1;
bool received_report_clutch2;
bool received_temp_protection;
bool received_coupling_open;
bool received_speed_limit;
bool received_status_bit_4;
bool received_status_bit_5;
bool received_status_bit_7;

byte vehicle_speed = 0;
int vehicle_RPM = 0;
float lock_target = 0;
float ped_value = 0;

uint8_t disableSpeed = 244;
uint8_t BRAKES1_counter = 0;
long lastCANChassisTick;
long lastCANHaldexTick;
byte debug_speed = 0;

TaskHandle_t replay_task = NULL;
bool replayActive = false;
bool replayWithoutSending = false;

uint8_t haldex_inbox_history_index = 0;
uint8_t haldex_outbox_history_index = 0;
uint8_t body_inbox_history_index = 0;
uint8_t body_outbox_history_index= 0;

can_frame haldex_inbox_history[200] = {0};
can_frame haldex_outbox_history[200] = {0};
can_frame body_inbox_history[200] = {0};
can_frame body_outbox_history[200] = {0};

// Define the main CAN structure
can_s body_can = {
    0,                            // CAN status
    NULL,                         // SPI interface
    new MCP_CAN(BODY_CAN_CS_PIN), // CAN interface
    NULL,                         // Comms task
    NULL,                         // Outbox
    NULL,                         // Inbox
    false, // Inited flag                       // Name
};

can_s haldex_can = {
    0,                              // CAN status
    NULL,                           // SPI interface
    new MCP_CAN(HALDEX_CAN_CS_PIN), // CAN interface
    NULL,                           // Comms task
    NULL,                           // Outbox
    NULL,                           // Inbox
    false,                          // Inited flag

};

void setup()
{

    Serial.begin(115200);
    EEPROM.begin(EEPROM_SIZE);
    loadCustomMode();
    connectWifi();

    setUpUI();
    // Create queues for the CAN interfaces
    body_can.outbox = xQueueCreate(10, sizeof(can_frame));
    haldex_can.outbox = xQueueCreate(10, sizeof(can_frame));
    body_can.inbox = xQueueCreate(10, sizeof(can_frame));
    haldex_can.inbox = xQueueCreate(10, sizeof(can_frame));

    // Assign the correct SPI busses to the instance
    body_can.spi_interface = new SPIClass(VSPI);
    haldex_can.spi_interface = new SPIClass(HSPI);

        body_can.spi_interface->setFrequency(8000000);
    haldex_can.spi_interface->setFrequency(8000000);

    // Start SPI bus
    body_can.spi_interface->begin(BODY_CAN_SCK_PIN, BODY_CAN_SO_PIN, BODY_CAN_SI_PIN, BODY_CAN_CS_PIN);
    haldex_can.spi_interface->begin(HALDEX_CAN_SCK_PIN, HALDEX_CAN_SO_PIN, HALDEX_CAN_SI_PIN, HALDEX_CAN_CS_PIN);

    body_can.can_interface->setSpiInstance(body_can.spi_interface);
    haldex_can.can_interface->setSpiInstance(haldex_can.spi_interface);

    // Start CAN bus interface
    body_can.status = body_can.can_interface->begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ);

    // Filters could be switched on to take off the load from the bus
    if (body_can.status == CAN_OK)
    {
#if 0
        body_can.can_interface->init_Filt(0, 0, 0x080);
        body_can.can_interface->init_Filt(1, 0, 0x080);
        body_can.can_interface->init_Mask(0, 0, 0x0E0);

        body_can.can_interface->init_Filt(2, 0, 0x0A0);
        body_can.can_interface->init_Filt(3, 0, 0x0A0);
        body_can.can_interface->init_Filt(4, 0, 0x0A0);
        body_can.can_interface->init_Filt(5, 0, 0x0A0);
        body_can.can_interface->init_Mask(1, 0, 0x0E0);
#endif
        // Alternative masks with a tighter filters
#if 0
        body_can.can_interface->init_Mask(0, 0, 0x7FF); // RXB0 mask
        body_can.can_interface->init_Filt(0, 0, 0x280); // RXF0
        body_can.can_interface->init_Filt(1, 0, 0x288); // RXF1

        body_can.can_interface->init_Mask(1, 0, 0x7FF); // RXB1 mask
        body_can.can_interface->init_Filt(2, 0, 0x380); // RXF2
        body_can.can_interface->init_Filt(3, 0, 0x1A0); // RXF3
        body_can.can_interface->init_Filt(4, 0, 0x4A0); // RXF4
#endif
        body_can.can_interface->setMode(MCP_NORMAL);
        Serial.println("Body CAN: Init OK!");
    }
    else
    {
        Serial.printf("Body CAN: Init Fail!!! (%d)\n", body_can.status);
    }

    // Start Haldex CAN Bus
    haldex_can.status = haldex_can.can_interface->begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ);

    // Again filters could be used to take off the load
    if (haldex_can.status == CAN_OK)
    {
#if 0
        haldex_can.can_interface->init_Filt(0, 0, 0x2C0);
        haldex_can.can_interface->init_Filt(1, 0, 0x2C0);
        haldex_can.can_interface->init_Mask(0, 0, 0x7FF);

        haldex_can.can_interface->init_Filt(2, 0, 0x2C0);
        haldex_can.can_interface->init_Filt(3, 0, 0x2C0);
        haldex_can.can_interface->init_Filt(4, 0, 0x2C0);
        haldex_can.can_interface->init_Filt(5, 0, 0x2C0);
        haldex_can.can_interface->init_Mask(1, 0, 0x7FF);
#endif
        haldex_can.can_interface->setMode(MCP_NORMAL);
        Serial.println("Haldex CAN: Init OK!");
    }
    else
    {
        Serial.printf("Haldex CAN: Init Fail!!! (%d)\n", haldex_can.status);
    }

    if (haldex_can.status == CAN_OK)
    {
        // Create the task first
        xTaskCreatePinnedToCore(
            parseCAN_hdx,
            "parseCAN_hdx",
            CAN_STACK,
            NULL,
            4,
            &haldex_can.comms_task,
            1);

        // Setup interrupt pin and attach ISR
        pinMode(HALDEX_CAN_INT_PIN, INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(HALDEX_CAN_INT_PIN),
                        parseCan_chs_handler,
                        FALLING); // MCP2515 INT pin goes LOW when message available
    }

    if (body_can.status == CAN_OK)
    {

        // Create the task first
        xTaskCreatePinnedToCore(
            parseCAN_chs,
            "parseCAN_chs",
            CAN_STACK,
            NULL,
            5,
            &body_can.comms_task, // Make sure this exists in your body_can struct
            0                     // Or Core 0 if you prefer separation
        );

        // Setup interrupt pin and attach ISR
        pinMode(BODY_CAN_INT_PIN, INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(BODY_CAN_INT_PIN),
                        parseCan_chs_handler,
                        FALLING);
    }

    // Test data function to simulate bus
#ifdef CAN_TEST_DATA
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    if (body_can.status == CAN_OK && haldex_can.status == CAN_OK)
    {
        xTaskCreate(
            send_test_data,
            "send_test_data",
            8000,
            NULL,
            6,
            NULL);
    }
#endif

#if detailedDebug
    xTaskCreate(
        showHaldexState,
        "showHaldexState",
        4096,
        NULL,
        1,
        &haldex_status_update_task);
#endif

    // Create task for testing CAN data manipulations
    // Should be probably switched off in production to not waste resources
    xTaskCreatePinnedToCore(
        serialCommandTask,
        "serialCommandTask",
        4096,
        NULL,
        1,
        &serial_command_task,
        1);

    Serial.println("Setup done");
    vTaskDelete(NULL);
}

void loop()
{

    delay(350);
}

void showHaldexState(void *params)
{
    while (1)
    {

        Serial.printf("Mode: %s \n", state.mode == MODE_STOCK ? "STOCK" : state.mode == MODE_FWD  ? "FWD"
                                                                      : state.mode == MODE_7525   ? "75/25"
                                                                      : state.mode == MODE_5050   ? "50/50"
                                                                      : state.mode == MODE_CUSTOM ? "CUSTOM"
                                                                                                  : "UNKNOWN");
        Serial.printf("    haldexEngagement: %d\n", haldex_engagement); // this is the lock %
        Serial.printf("    vehicleSpeed: %d km/h\n", vehicle_speed);    // this is the lock %
        Serial.printf("    vehicleRPM: %d RPM\n", vehicle_RPM);         // this is the lock %
        Serial.printf("    lockTarget: %d %%\n", (int)lock_target);
        Serial.printf("    pedalValue: %.1f %%\n", ped_value);

#if detailedDebugCAN

        byte err = body_can.can_interface->checkError();
        Serial.println("");                 // this is the lock %
        Serial.println("CAN-BUS Details:"); // this is the lock %
        Serial.printf("    RX errors Body: %lu\t", body_can.can_interface->errorCountRX());
        Serial.printf("    TX errors Body: %lu\t", body_can.can_interface->errorCountTX());
        Serial.printf("    RX errors Haldex %lu\t", haldex_can.can_interface->errorCountRX());
        Serial.printf("    TX errors Haldex %lu\n", haldex_can.can_interface->errorCountTX());

#endif

        vTaskDelay(serialMonitorRefresh / portTICK_PERIOD_MS);
    }
}
