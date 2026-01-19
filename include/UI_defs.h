#define NUM_LOCK_POINTS 10

uint16_t int16_customSelect;

uint16_t speedArray[] = { 0, 0, 0, 0, 0 };
uint8_t throttleArray[] = { 0, 0, 0, 0, 0 };
uint8_t lockArray[] = { 0, 0, 0, 0, 0 };

uint16_t customSet_1, customSet_2, customSet_3, customSet_4, customSet_5;


typedef enum openhaldex_mode_id{
    MODE_Stock,
    MODE_FWD,
    MODE_7525,
    MODE_5050,
    MODE_Custom
}openhaldex_mode_id;

typedef struct lockpoint{
    uint16_t id;
    byte speed;
    byte lock;
    byte intensity;
}lockpoint;

typedef struct openhaldex_custom_mode{
    lockpoint lockpoints[NUM_LOCK_POINTS];
    byte lockpoint_rx_h;
    byte lockpoint_rx_l;
    byte lockpoint_count;
}openhaldex_custom_mode;

typedef struct openhaldex_state {
    openhaldex_mode_id mode;
    openhaldex_custom_mode custom_mode;
    float ped_threshold;
    bool mode_override;
}openhaldex_state;

//UI handles
uint16_t wifi_ssid_text, wifi_pass_text;
uint16_t current_haldex_state_label,current_lock_label,wanted_lock_label,current_speed_label;
uint16_t main_tab, custom_mode_tab;
volatile bool updates = false;
openhaldex_custom_mode custom_mode;


openhaldex_state wanted_haldex_state;