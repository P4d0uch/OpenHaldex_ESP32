/* Defines */
#define CAN_DEBUG
#define CAN_PRINT_DATA
// #define CAN_TEST_DATA
// #define BT_SERIAL_DEBUG_RX
// #define BT_SERIAL_DEBUG_TX
// #define STATE_DEBUG
// #define STACK_DEBUG

#define INVERT_SPIS 0
#define NUM_LOCK_POINTS 10

#define CAN_STACK 2176
#define TASK_DELAY_MS 5
#define EEPROM_SIZE 512
#define EEPROM_ADDR_CUSTOM_MODE 0 // starting address for custom mode data

#define USE_INTERRUPTS 0
#define SLOW_BOOT 0
#define USE_HOTSPOT 1
#define HOSTNAME "VJ_AP_E"
#define MY_SSID "MS-Free.NET.Jisovi_2.4"
#define MY_PASSWORD "MalaSkala526"
#define FORCE_USE_HOTSPOT 0

#define enableDebug 0
#define detailedDebug 0
#define detailedDebugStack 0
#define detailedDebugRuntimeStats 0
#define detailedDebugCAN 0
#define detailedDebugWiFi 0
#define detailedDebugEEP 0
#define detailedDebugIO 0
#define serialMonitorRefresh 1000 // Serial Monitor refresh rate in ms



#define BODY_CAN_SCK_PIN 18
#define BODY_CAN_CS_PIN 5
#define BODY_CAN_SO_PIN 19
#define BODY_CAN_SI_PIN 23
#define BODY_CAN_INT_PIN 17

#define HALDEX_CAN_SCK_PIN 14
#define HALDEX_CAN_CS_PIN 15
#define HALDEX_CAN_SO_PIN 12
#define HALDEX_CAN_SI_PIN 13
#define HALDEX_CAN_INT_PIN 16


#define ARRAY_SIZE(array) (sizeof((array)) / sizeof((array)[0]))

typedef struct can_s
{
    byte status;
    SPIClass *spi_interface;
    MCP_CAN *can_interface;
    TaskHandle_t comms_task;
    QueueHandle_t outbox;
    QueueHandle_t inbox;
    bool inited;
} can_s;

typedef enum openhaldex_mode_id
{
    MODE_STOCK,
    MODE_FWD,
    MODE_7525,
    MODE_5050,
    MODE_CUSTOM
} openhaldex_mode_id;

typedef struct lockpoint
{
    byte speed;
    byte lock;
} lockpoint;

typedef struct openhaldex_custom_mode
{
    lockpoint lockpoints[NUM_LOCK_POINTS];
    byte lockpoint_count;
} openhaldex_custom_mode;

typedef struct openhaldex_state
{
    openhaldex_mode_id mode;
    openhaldex_custom_mode custom_mode;
    float ped_threshold;
    bool mode_override;
} openhaldex_state;
