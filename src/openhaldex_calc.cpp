#include "openhaldex.h"


/**
 * Sort the lockpoints from the UI to ensure rising speed
 */
void sortLockpoints(openhaldex_custom_mode &mode)
{
  std::sort(mode.lockpoints, mode.lockpoints + mode.lockpoint_count, [](const lockpoint &a, const lockpoint &b)
            { return a.speed < b.speed; });
}

/**
 * Returns a desired lockpoint based on the closest nodes
 * Just interpolate between the two points 
 */
byte getLockForSpeed(const openhaldex_custom_mode &mode, byte speed)
{
  if (mode.lockpoint_count == 0)
    return 0;

  // Clamp below and above
  if (speed <= mode.lockpoints[0].speed)
    return mode.lockpoints[0].lock;
  if (speed >= mode.lockpoints[mode.lockpoint_count - 1].speed)
    return mode.lockpoints[mode.lockpoint_count - 1].lock;

  // Find interval
  for (int i = 0; i < mode.lockpoint_count - 1; i++)
  {
    byte s1 = mode.lockpoints[i].speed;
    byte s2 = mode.lockpoints[i + 1].speed;
    if (speed >= s1 && speed <= s2)
    {
      byte l1 = mode.lockpoints[i].lock;
      byte l2 = mode.lockpoints[i + 1].lock;

      // Linear interpolation
      float t = float(speed - s1) / float(s2 - s1);
      return byte(l1 + t * (l2 - l1));
    }
  }

  return 0; // Should never reach here but for safety
}


/**
 * First filter to get the desired value????
 * Stock mode returns zero. Others desired value in int
 */
float get_lock_target_adjustment()
{
  switch (state.mode)
  {
  case MODE_FWD:
    return 0;

  case MODE_5050:
    if (state.ped_threshold == 0 && disableSpeed == 0)
    {
      return 100;
    }

    if (int(ped_value) >= state.ped_threshold || state.ped_threshold == 0 || vehicle_speed < disableSpeed || state.mode_override)
    {
      return 100;
    }
    return 0;

  case MODE_7525:
    if (int(ped_value) >= state.ped_threshold || state.ped_threshold == 0 || vehicle_speed < disableSpeed || state.mode_override)
    {
      return 30;
    }
    return 0;

  case MODE_CUSTOM:
    if (int(ped_value) >= state.ped_threshold || state.ped_threshold == 0 || vehicle_speed < disableSpeed || state.mode_override)
    {
      return getLockForSpeed(state.custom_mode, vehicle_speed);
    }
    return 0;

  default:
    // In stock mode
    return 0;
  }

 
}

/**
 * This function is the main trickery. It calculates the lock value (hex) from the desired int value
 * The locking is non linear and not 0-100% ???
 */
uint8_t get_lock_target_adjusted_value(uint8_t value, bool invert)
{
  // Handle 5050 mode - It just sets fixed value
  if (lock_target == 100)
  {
    // is this needed?  Should be caught in get_lock_target_adjustment
    if (int(ped_value) >= state.ped_threshold || vehicle_speed < disableSpeed || state.ped_threshold == 0)
    {
      return (invert ? (0xFE - value) : value);
    }
    return (invert ? 0xFE : 0x00);
  }

  // Handle FWD and STOCK mode.
  // No correction is necessary if the target is already 0.
  if (lock_target == 0)
  {
    return (invert ? 0xFE : 0x00);
  }

  // Some tricks to get the correct value ?? :)
  /*
    * Hackery to get the response closer to the target... we are trying to control the
    * Haldex as if it's linear.. but it's not. In future, I'd like to implement some sort
    * of feedback loop to trim the calculation being made here but this will do for now.
    */
  float correction_factor = ((float)lock_target / 2) + 20;
  uint8_t corrected_value = value * (correction_factor / 100);
  if (int(ped_value) >= state.ped_threshold || vehicle_speed < disableSpeed || state.ped_threshold == 0)
  {
    return (invert ? (0xFE - corrected_value) : corrected_value);
  }
  return (invert ? 0xFE : 0x00);
}

/**
 * Frame manipulation
 * It runs in every mode except STOCK
 */
void modifyHaldexFrame(can_frame &frameToModify)
{
  // Get the initial lock target. - It seems redundant?????
  lock_target = get_lock_target_adjustment();

  
  // Each frame has a different modification
  switch (frameToModify.id)
  {
      case MOTOR1_ID:
        frameToModify.data.bytes[0] = 0x00;                                         // various individual bits ('space gas', driving pedal, kick down, clutch, timeout brake, brake intervention, drinks-torque intervention?) was 0x01 - ignored
        frameToModify.data.bytes[1] = get_lock_target_adjusted_value(0xFE, false);  // rpm low byte
        frameToModify.data.bytes[2] = 0x21;                                         // rpm high byte
        frameToModify.data.bytes[3] = get_lock_target_adjusted_value(0x4E, false);  // set RPM to a value so the pre-charge pump runs
        frameToModify.data.bytes[4] = get_lock_target_adjusted_value(0xFE, false);  // inner moment (%): 0.39*(0xF0) = 93.6%  (make FE?) - ignored
        frameToModify.data.bytes[5] = get_lock_target_adjusted_value(0xFE, false);  // driving pedal (%): 0.39*(0xF0) = 93.6%  (make FE?) - ignored
                                                                               // rx_message_chs.data[6] = get_lock_target_adjusted_value(0x16, false);  // set to a low value to control the req. transfer torque.  Main control value for Gen1
        switch (state.mode) {
          case MODE_FWD:
            appliedTorque = get_lock_target_adjusted_value(0xFE, true);  // return 0xFE to disable
            break;
          case MODE_5050:
            appliedTorque = get_lock_target_adjusted_value(0x16, false);  // return 0x16 to fully lock
            break;
          case MODE_7525:
            appliedTorque = get_lock_target_adjusted_value(0x50, false);  // set to ~30% lock (0x96 = 15%, 0x56 = 27%)
            break;
          case MODE_CUSTOM:
          // NOT DONE YET JUST LOCK FULLY
          // I have to convert int value to non linear hex value somehow
            appliedTorque = get_lock_target_adjusted_value(0x16, false);
            break;
        }

       frameToModify.data.bytes[6] = appliedTorque;  // was 0x00
        frameToModify.data.bytes[7] = 0x00;           // these must play a factor - achieves ~169 without
        break;
      case MOTOR3_ID:
        frameToModify.data.bytes[2] = get_lock_target_adjusted_value(0xFE, false);  // pedal - ignored
        frameToModify.data.bytes[7] = get_lock_target_adjusted_value(0xFE, false);  // throttle angle (100%), ignored
        break;
      case BRAKES1_ID:
        frameToModify.data.bytes[1] = get_lock_target_adjusted_value(0x00, false);  // also controlling slippage.  Brake force can add 20%
        frameToModify.data.bytes[2] = 0x00;                                         //  ignored
        frameToModify.data.bytes[3] = get_lock_target_adjusted_value(0x0A, false);  // 0xA ignored?
        break;
      case BRAKES3_ID:
        frameToModify.data.bytes[0] = get_lock_target_adjusted_value(0xFE, false);  // low byte, LEFT Front // affects slightly +2
        frameToModify.data.bytes[1] = 0x0A;                                         // high byte, LEFT Front big effect
        frameToModify.data.bytes[2] = get_lock_target_adjusted_value(0xFE, false);  // low byte, RIGHT Front// affects slightly +2
        frameToModify.data.bytes[3] = 0x0A;                                         // high byte, RIGHT Front big effect
        frameToModify.data.bytes[4] = 0x00;                                         // low byte, LEFT Rear
        frameToModify.data.bytes[5] = 0x0A;                                         // high byte, LEFT Rear // 254+10? (5050 returns 0xA)
        frameToModify.data.bytes[6] = 0x00;                                         // low byte, RIGHT Rear
        frameToModify.data.bytes[7] = 0x0A;                                         // high byte, RIGHT Rear  // 254+10?
        break;
    }
}