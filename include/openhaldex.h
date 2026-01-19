#include <Arduino.h>

#include <SPI.h>
#include <mcp_can.h>
#include <Preferences.h>

#include "openhaldex_defs.h"
#include "openhaldex_can.h"
#include "openhaldex_HTML.h"
#include "openhaldex_CANIDs.h"

#include <WiFi.h>
#include <ESPmDNS.h>
#include <EEPROM.h>

#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <algorithm>

/* Globals */

extern openhaldex_state state;
extern byte vehicle_speed;
extern int vehicle_RPM;
extern float lock_target;
extern float ped_value;
extern byte haldex_engagement;

extern bool received_report_clutch1;
extern bool received_report_clutch2;
extern bool received_temp_protection;
extern bool received_coupling_open;
extern bool received_speed_limit;
extern bool received_status_bit_4;
extern bool received_status_bit_5;
extern bool received_status_bit_7;
extern byte haldex_engagement_raw;
extern can_s body_can;
extern can_s haldex_can;
extern TaskHandle_t haldex_status_update_task;


extern uint8_t received_haldex_state;
extern uint8_t received_haldex_engagement_raw;
extern uint8_t received_haldex_engagement;
extern uint8_t appliedTorque;

extern uint8_t disableSpeed;

extern long lastCANChassisTick;
extern long lastCANHaldexTick;

extern uint8_t BRAKES1_counter;
extern TaskHandle_t replay_task;
extern TaskHandle_t serial_command_task;

extern bool replayActive;
extern bool replayWithoutSending;
extern int replayMsgCount;



#ifdef CAN_TEST_DATA
extern void send_test_data(void *params);

#endif
extern byte debug_speed;

// Function Prototypes

//CAN processing functions
extern void parseCAN_hdx(void *arg);
extern void parseCAN_chs(void *arg);
extern void processChassisFrame(can_frame& frame);
extern void processHaldexFrame(can_frame& frame);

extern void modifyHaldexFrame(can_frame &frameToModify);
extern byte getLockForSpeed(const openhaldex_custom_mode &mode, byte speed);

extern uint8_t get_lock_target_adjusted_value(uint8_t value, bool invert);
extern float get_lock_target_adjustment(void);

extern void IRAM_ATTR parseCan_hdx_handler(void);
extern void IRAM_ATTR parseCan_chs_handler(void);


//Functions for replay mode
extern void replayTask(void *param);
extern void stopReplayMode(void);
extern void startReplayMode(void);
extern void serialCommandTask(void *param);
extern void handleReplayInput(const String &line);
extern void stopAllCanTasks(void);
extern void startAllCanTasks(void);


//Setup functions
extern void connectWifi();
extern void setUpUI();
extern void sortLockpoints(openhaldex_custom_mode &mode);
extern byte getLockForSpeed(const openhaldex_custom_mode &mode, byte speed);
extern void saveCustomMode();
extern void loadCustomMode();
extern void printCustomMode();
extern void showHaldexState(void *arg);
extern AsyncWebServer server;
