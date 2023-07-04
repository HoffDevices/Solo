
typedef enum {
    EVENT_NONE,
    EVENT_INF,
    EVENT_PRS,
    EVENT_RLS,
    EVENT_RLL,
    EVENT_TIM,
    EVENT_DSC,
    EVENT_CLK,
    EVENT_ENT,
    EVENT_EXT
} EVENT_TYPE;

typedef struct {
    char *LBL;
    char *INF;
    char *PRS;
    char *RLS;
    char *RLL;
    char *TIM;
    char *DSC;
    char *CLK;
    char *ENT;
    char *EXT;
} CONTROL;

typedef enum {
    VIEW_MAIN,
    VIEW_HELP,
    VIEW_BPM
} VIEW_TYPE;

typedef struct {
	VIEW_TYPE type;
    bool draw;  //to indicate when view has to be drawn once, so as to not draw it continuously
    bool active;
    uint8_t startLine;
    EVENT_TYPE event;
    uint8_t lastPreset;
    CONTROL *control;
    uint8_t page;
} VIEW;

typedef struct {
	uint16_t top;
	uint16_t left;
	uint16_t width;
    uint16_t height;
    uint16_t radius;
    uint16_t color;
    uint16_t chars;  //max characters to fill
    bool active;
    bool frame;
    bool buffer;
    char name[14];
    sFONT *font;
    void (*pressed)();
    void (*releasedShort)();
    void (*releasedLong)();
    void (*timedOut)();
} BUTTON;

typedef struct {
    uint16_t color;
    char *name;
    void (*show)();
    CONTROL PST;
    CONTROL LS0;
    CONTROL LS1;
    CONTROL LS2;
    CONTROL RS0;
    CONTROL RS1;
    CONTROL RS2;
} SCREEN;

typedef enum  {
    TUSB_NOT_MOUNTED,
    TUSB_MOUNTED,
    TUSB_SUSPENDED
} TUSB;

typedef struct {
    void (*init)();  //init method
    void (*send)(uint8_t *msg, uint32_t len);  //send method
} MIDI;

typedef struct {
    void (*init)();  //init method
    void (*pulse)(uint8_t level, int64_t duration);  //pulse method
} SYNC;

typedef enum  {
    CORE_REQ_PRESET_ENTER,
    CORE_ACK_PRESET_ENTER,
    CORE_REQ_PRESET_EXIT,
    CORE_ACK_PRESET_EXIT,
    CORE_REQ_TIMER_DISABLE,
    CORE_ACK_TIMER_DISABLE,
    CORE_REQ_SYSEX_SEND,
    CORE_ACK_SYSEX_SEND_STARTED,
    CORE_ACK_SYSEX_SEND_FINISHED,
    CORE_REQ_SYSEX_RECV,
    CORE_ACK_SYSEX_RECV_STARTED,
    CORE_ACK_SYSEX_RECV_FINISHED,
    CORE_REQ_BPM_PLUSONE,
    CORE_REQ_BPM_PLUSTEN,
    CORE_REQ_BPM_MINUSONE,
    CORE_REQ_BPM_MINUSTEN
} COREFLAG;

typedef struct {
    uint32_t gpio;  //hardware switch gpio
	uint16_t x;  //touch screen coordinate
    uint16_t y;  //touch screen coordinate
    void (*init)();  //init method
    void (*check)();  //determine if pressed, short or long released
    bool (*state)();  //get state
    //actuation properties
    int64_t debounceTime;
    bool debounceWait;
    bool currState;
    bool prevState;
    bool pressed;  //pressed since last checked
    bool releasedShort;  //releasedShort since last checked
    bool releasedLong;  //releasedLong since last checked
    bool timedOut;  //long press timed out
    bool timedBusy;  //prevent event from firing continuously while held down
    uint64_t counter;  //count presses
    absolute_time_t currPressedTime;
    absolute_time_t currReleasedTime;
    absolute_time_t prevPressedTime;
    absolute_time_t prevReleasedTime;
} SWITCH;

typedef enum  {
    COLOR_BLACK,
    COLOR_WHITE,
    COLOR_GREY,
    COLOR_BLUE,
    COLOR_CYAN,
    COLOR_GREEN,
    COLOR_RED,
    COLOR_GOLD
} COLOR_MAP;

extern SWITCH SWITCH_LS0;
extern SWITCH SWITCH_LS1;
extern SWITCH SWITCH_LS2;
extern SWITCH SWITCH_RS0;
extern SWITCH SWITCH_RS1;
extern SWITCH SWITCH_RS2;

extern SWITCH SWITCH_TOUCH;  //only one touch event for all screens

extern MIDI MIDI_RIGHT;
extern MIDI MIDI_LEFT;
extern MIDI MIDI_USB;
extern MIDI MIDI_BLE;

extern SYNC SYNC_1;
extern SYNC SYNC_2;

static uint64_t MIDI_CLOCK_COUNTER = 0;
static uint8_t MIDI_CLOCK_PPQ = 24;  //pulses per quarter note
static int64_t MIDI_CLOCK_TIMER_US = (int64_t) (-60000000 / 100 / 24);  //Negative delay means we will call repeating_timer_callback, and call it again exactly ???ms later regardless of how long the callback took to execute
static struct repeating_timer MIDI_CLOCK_TIMER;

alarm_pool_t *SYNC_ALARM_POOL;
static alarm_id_t SYNC_1_ALARM_ID;
static alarm_id_t SYNC_2_ALARM_ID;

static bool MIDI_SYSEX_BUSY = false;
static uint32_t MIDI_SYSEX_INDEX = 0;

static SCREEN SCREENS[64];  //somewhat arbitrary limit
static uint8_t CURR_SCREEN_INDEX = 0;
static uint8_t MAX_SCREEN_INDEX = 0;

static TUSB TUSB_STATUS;
static VIEW CURR_VIEW;  //only ever one VIEW per SCREEN active at a time - this way the active scan can continue while showing help

static uint8_t USER_VAR[10] = {0,0,0,0,0,0,0,0,0,0};  //ten user variables - v0..v9 (single digits)

static int8_t SNAKE_DIRECTION = 0;

#define BPM_BUFFER_WIDTH (61*3/2)  //3 characters wide
#define BPM_BUFFER_HEIGHT 104  //1 character high
static uint8_t BPM_BUFFER[BPM_BUFFER_WIDTH][BPM_BUFFER_HEIGHT];  //hard coded limit depends on size of B_BPM_BIG_HELP

// Flash-based address of the FACTORY and USER confis
#define PRESETS_SIZE (FLASH_SECTOR_SIZE * 32)  //4096 * 32 = 131076
#define FACTORY_FLASH_OFFSET (PICO_FLASH_SIZE_BYTES - PRESETS_SIZE * 2)
const char *FACTORY_FLASH = (const char *) (XIP_BASE + FACTORY_FLASH_OFFSET);
#define USER_FLASH_OFFSET (PICO_FLASH_SIZE_BYTES - PRESETS_SIZE * 1)
const char *USER_FLASH = (const char *) (XIP_BASE + USER_FLASH_OFFSET);

#define LCD_WIDTH 320
#define LCD_HEIGHT 240
#define SNAKE_SIZE 100

//these functions use buttons and are called by buttons, so difficult to order code sequentially
void writeSysexConfigToUserFlash();
void writeFactoryFlashToUserFlash();
void sendSysexConfig();
void loadFirmware();

//config and versioning settings
static char FIRMWARE_VERSION[5] = "0.26";  //hardcoded firmware version, to be updated manually by author
static char FACTORY_VERSION[5] = "----";  //placeholder value, will be updated by updateFactoryFlash() 
static char *PRESETS_VERSION;  //presets version, assigned when parsing PRESETS_TEXT from USER_FLASH
static char *PRESETS_DEFAULT;  //default preset, assigned when parsing PRESETS_TEXT from USER_FLASH
static char PRESETS_TEXT[PRESETS_SIZE] = ""  //factory presets
"<SYSTEM>"
"<VERSION>0.26"
"<DEFAULT>TRANSPRT"

"<PRESET>"
"<PST_LBL>SNAKE"
"<PST_DSC>A fun game. Use the main\nfoot switches to turn left\nor right."
"<LS0_LBL>LEFT"
"<RS0_LBL>RIGHT"

"<PRESET>"
"<PST_LBL>VERIFY"
"<PST_DSC>Verify that all switches and MIDI ports work, by sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages to all MIDI ports for each switch that is PRESSED, RELEASED or TIMED OUT. Also sends a MIDI system real time clock message at 120 BPM."
"<PST_CLK>[1,1]MSG_USB(0xF8)"
"<PST_ENT>[1,1]SET_BPM(10)"
"<PST_EXT>[1,1]SET_BPM(120)"
"<LS0_LBL>LFT SW0"
"<LS0_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS0_PRS>[1,1]MSG_USB(0X90,1,1) [1,1]MSG_LFT(0X90,2,1) [1,1]MSG_RGT(0X90,3,1) [1,1]MSG_BLE(0X90,4,1) [1,1]SET_SC1(1,0)"
"<LS0_RLS>[1,1]MSG_USB(0X80,1,1) [1,1]MSG_LFT(0X80,2,1) [1,1]MSG_RGT(0X80,3,1) [1,1]MSG_BLE(0X80,4,1) [1,1]SET_SC1(0,0)"
"<LS0_RLL>[1,1]MSG_USB(0XA0,1,1) [1,1]MSG_LFT(0XA0,2,1) [1,1]MSG_RGT(0XA0,3,1) [1,1]MSG_RGT(0XA0,4,1)"
"<LS0_TIM>[1,1]MSG_USB(0XB0,1,1) [1,1]MSG_LFT(0XB0,2,1) [1,1]MSG_RGT(0XB0,3,1) [1,1]MSG_RGT(0XB0,4,1) [1,1]SET_SC1(0,250)"
"<LS1_LBL>LFT SW1"
"<LS1_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS1_PRS>[1,1]MSG_USB(0X90,1,2) [1,1]MSG_LFT(0X90,2,2) [1,1]MSG_RGT(0X90,3,2) [1,1]MSG_BLE(0X90,4,2)"
"<LS1_RLS>[1,1]MSG_USB(0X80,1,2) [1,1]MSG_LFT(0X80,2,2) [1,1]MSG_RGT(0X80,3,2) [1,1]MSG_BLE(0X80,4,2)"
"<LS1_RLL>[1,1]MSG_USB(0XA0,1,2) [1,1]MSG_LFT(0XA0,2,2) [1,1]MSG_RGT(0XA0,3,2) [1,1]MSG_RGT(0XA0,4,2)"
"<LS1_TIM>[1,1]MSG_USB(0XB0,1,2) [1,1]MSG_LFT(0XB0,2,2) [1,1]MSG_RGT(0XB0,3,2) [1,1]MSG_RGT(0XB0,4,2)"
"<LS2_LBL>LFT SW2"
"<LS2_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS2_PRS>[1,1]MSG_USB(0X90,1,3) [1,1]MSG_LFT(0X90,2,3) [1,1]MSG_RGT(0X90,3,3) [1,1]MSG_BLE(0X90,4,3)"
"<LS2_RLS>[1,1]MSG_USB(0X80,1,3) [1,1]MSG_LFT(0X80,2,3) [1,1]MSG_RGT(0X80,3,3) [1,1]MSG_BLE(0X80,4,3)"
"<LS2_RLL>[1,1]MSG_USB(0XA0,1,3) [1,1]MSG_LFT(0XA0,2,3) [1,1]MSG_RGT(0XA0,3,3) [1,1]MSG_RGT(0XA0,4,3)"
"<LS2_TIM>[1,1]MSG_USB(0XB0,1,3) [1,1]MSG_LFT(0XB0,2,3) [1,1]MSG_RGT(0XB0,3,3) [1,1]MSG_RGT(0XB0,4,3)"
"<RS0_LBL>RGT SW0"
"<RS0_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS0_PRS>[1,1]MSG_USB(0X90,17,1) [1,1]MSG_LFT(0X90,18,1) [1,1]MSG_RGT(0X90,19,1) [1,1]MSG_BLE(0X90,20,1) [1,1]SET_SC2(1,0)"
"<RS0_RLS>[1,1]MSG_USB(0X80,17,1) [1,1]MSG_LFT(0X80,18,1) [1,1]MSG_RGT(0X80,19,1) [1,1]MSG_BLE(0X80,20,1) [1,1]SET_SC2(0,0)"
"<RS0_RLL>[1,1]MSG_USB(0XA0,17,1) [1,1]MSG_LFT(0XA0,18,1) [1,1]MSG_RGT(0XA0,19,1) [1,1]MSG_RGT(0XA0,20,1)"
"<RS0_TIM>[1,1]MSG_USB(0XB0,17,1) [1,1]MSG_LFT(0XB0,18,1) [1,1]MSG_RGT(0XB0,19,1) [1,1]MSG_RGT(0XB0,20,1) [1,1]SET_SC2(0,250)"
"<RS1_LBL>RGT SW1"
"<RS1_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS1_PRS>[1,1]MSG_USB(0X90,17,2) [1,1]MSG_LFT(0X90,18,2) [1,1]MSG_RGT(0X90,19,2) [1,1]MSG_BLE(0X90,20,2)"
"<RS1_RLS>[1,1]MSG_USB(0X80,17,2) [1,1]MSG_LFT(0X80,18,2) [1,1]MSG_RGT(0X80,19,2) [1,1]MSG_BLE(0X80,20,2)"
"<RS1_RLL>[1,1]MSG_USB(0XA0,17,2) [1,1]MSG_LFT(0XA0,18,2) [1,1]MSG_RGT(0XA0,19,2) [1,1]MSG_RGT(0XA0,20,2)"
"<RS1_TIM>[1,1]MSG_USB(0XB0,17,2) [1,1]MSG_LFT(0XB0,18,2) [1,1]MSG_RGT(0XB0,19,2) [1,1]MSG_RGT(0XB0,20,2)"
"<RS2_LBL>RGT SW2"
"<RS2_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS2_PRS>[1,1]MSG_USB(0X90,17,3) [1,1]MSG_LFT(0X90,18,3) [1,1]MSG_RGT(0X90,19,3) [1,1]MSG_BLE(0X90,20,3)"
"<RS2_RLS>[1,1]MSG_USB(0X80,17,3) [1,1]MSG_LFT(0X80,18,3) [1,1]MSG_RGT(0X80,19,3) [1,1]MSG_BLE(0X80,20,3)"
"<RS2_RLL>[1,1]MSG_USB(0XA0,17,3) [1,1]MSG_LFT(0XA0,18,3) [1,1]MSG_RGT(0XA0,19,3) [1,1]MSG_RGT(0XA0,20,3)"
"<RS2_TIM>[1,1]MSG_USB(0XB0,17,3) [1,1]MSG_LFT(0XB0,18,3) [1,1]MSG_RGT(0XB0,19,3) [1,1]MSG_RGT(0XB0,20,3)"

"<PRESET>"
"<PST_LBL>STRESS"
"<PST_DSC>Stress test the number of MIDI messages to be sent - 20BPM seems to be the limit for CLK."
"<PST_CLK>[@7,1]MSG_LFT(0X80,1,1)"
"<PST_ENT>SET_VAR(1,0)"
"<LS0_LBL>TEST"
"<LS0_INF>Send 127 messages to the left MIDI DIN out"
"<LS0_PRS>[1,1]MSG_LFT(0X90,1,1) [1,1]MSG_LFT(0X90,1,2) [1,1]MSG_LFT(0X90,1,3) [1,1]MSG_LFT(0X90,1,4) [1,1]MSG_LFT(0X90,1,5) [1,1]MSG_LFT(0X90,1,6) [1,1]MSG_LFT(0X90,1,7) [1,1]MSG_LFT(0X90,1,8) [1,1]MSG_LFT(0X90,1,9) [1,1]MSG_LFT(0X90,1,10) [1,1]MSG_LFT(0X90,1,11) [1,1]MSG_LFT(0X90,1,12) [1,1]MSG_LFT(0X90,1,13) [1,1]MSG_LFT(0X90,1,14) [1,1]MSG_LFT(0X90,1,15) [1,1]MSG_LFT(0X90,1,16) [1,1]MSG_LFT(0X90,1,17) [1,1]MSG_LFT(0X90,1,18) [1,1]MSG_LFT(0X90,1,19) [1,1]MSG_LFT(0X90,1,20) [1,1]MSG_LFT(0X90,1,21) [1,1]MSG_LFT(0X90,1,22) [1,1]MSG_LFT(0X90,1,23) [1,1]MSG_LFT(0X90,1,24) [1,1]MSG_LFT(0X90,1,25) [1,1]MSG_LFT(0X90,1,26) [1,1]MSG_LFT(0X90,1,27) [1,1]MSG_LFT(0X90,1,28) [1,1]MSG_LFT(0X90,1,29) [1,1]MSG_LFT(0X90,1,30) [1,1]MSG_LFT(0X90,1,31) [1,1]MSG_LFT(0X90,1,32) [1,1]MSG_LFT(0X90,1,33) [1,1]MSG_LFT(0X90,1,34) [1,1]MSG_LFT(0X90,1,35) [1,1]MSG_LFT(0X90,1,36) [1,1]MSG_LFT(0X90,1,37) [1,1]MSG_LFT(0X90,1,38) [1,1]MSG_LFT(0X90,1,39) [1,1]MSG_LFT(0X90,1,40) [1,1]MSG_LFT(0X90,1,41) [1,1]MSG_LFT(0X90,1,42) [1,1]MSG_LFT(0X90,1,43) [1,1]MSG_LFT(0X90,1,44) [1,1]MSG_LFT(0X90,1,45) [1,1]MSG_LFT(0X90,1,46) [1,1]MSG_LFT(0X90,1,47) [1,1]MSG_LFT(0X90,1,48) [1,1]MSG_LFT(0X90,1,49) [1,1]MSG_LFT(0X90,1,50) [1,1]MSG_LFT(0X90,1,51) [1,1]MSG_LFT(0X90,1,52) [1,1]MSG_LFT(0X90,1,53) [1,1]MSG_LFT(0X90,1,54) [1,1]MSG_LFT(0X90,1,55) [1,1]MSG_LFT(0X90,1,56) [1,1]MSG_LFT(0X90,1,57) [1,1]MSG_LFT(0X90,1,58) [1,1]MSG_LFT(0X90,1,59) [1,1]MSG_LFT(0X90,1,60) [1,1]MSG_LFT(0X90,1,61) [1,1]MSG_LFT(0X90,1,62) [1,1]MSG_LFT(0X90,1,63) [1,1]MSG_LFT(0X90,1,64) [1,1]MSG_LFT(0X90,1,65) [1,1]MSG_LFT(0X90,1,66) [1,1]MSG_LFT(0X90,1,67) [1,1]MSG_LFT(0X90,1,68) [1,1]MSG_LFT(0X90,1,69) [1,1]MSG_LFT(0X90,1,70) [1,1]MSG_LFT(0X90,1,71) [1,1]MSG_LFT(0X90,1,72) [1,1]MSG_LFT(0X90,1,73) [1,1]MSG_LFT(0X90,1,74) [1,1]MSG_LFT(0X90,1,75) [1,1]MSG_LFT(0X90,1,76) [1,1]MSG_LFT(0X90,1,77) [1,1]MSG_LFT(0X90,1,78) [1,1]MSG_LFT(0X90,1,79) [1,1]MSG_LFT(0X90,1,80) [1,1]MSG_LFT(0X90,1,81) [1,1]MSG_LFT(0X90,1,82) [1,1]MSG_LFT(0X90,1,83) [1,1]MSG_LFT(0X90,1,84) [1,1]MSG_LFT(0X90,1,85) [1,1]MSG_LFT(0X90,1,86) [1,1]MSG_LFT(0X90,1,87) [1,1]MSG_LFT(0X90,1,88) [1,1]MSG_LFT(0X90,1,89) [1,1]MSG_LFT(0X90,1,90) [1,1]MSG_LFT(0X90,1,91) [1,1]MSG_LFT(0X90,1,92) [1,1]MSG_LFT(0X90,1,93) [1,1]MSG_LFT(0X90,1,94) [1,1]MSG_LFT(0X90,1,95) [1,1]MSG_LFT(0X90,1,96) [1,1]MSG_LFT(0X90,1,97) [1,1]MSG_LFT(0X90,1,98) [1,1]MSG_LFT(0X90,1,99) [1,1]MSG_LFT(0X90,1,100) [1,1]MSG_LFT(0X90,1,101) [1,1]MSG_LFT(0X90,1,102) [1,1]MSG_LFT(0X90,1,103) [1,1]MSG_LFT(0X90,1,104) [1,1]MSG_LFT(0X90,1,105) [1,1]MSG_LFT(0X90,1,106) [1,1]MSG_LFT(0X90,1,107) [1,1]MSG_LFT(0X90,1,108) [1,1]MSG_LFT(0X90,1,109) [1,1]MSG_LFT(0X90,1,110) [1,1]MSG_LFT(0X90,1,111) [1,1]MSG_LFT(0X90,1,112) [1,1]MSG_LFT(0X90,1,113) [1,1]MSG_LFT(0X90,1,114) [1,1]MSG_LFT(0X90,1,115) [1,1]MSG_LFT(0X90,1,116) [1,1]MSG_LFT(0X90,1,117) [1,1]MSG_LFT(0X90,1,118) [1,1]MSG_LFT(0X90,1,119) [1,1]MSG_LFT(0X90,1,120) [1,1]MSG_LFT(0X90,1,121) [1,1]MSG_LFT(0X90,1,122) [1,1]MSG_LFT(0X90,1,123) [1,1]MSG_LFT(0X90,1,124) [1,1]MSG_LFT(0X90,1,125) [1,1]MSG_LFT(0X90,1,126) [1,1]MSG_LFT(0X90,1,127)"
"<RS0_LBL>CLK TEST"
"<RS0_INF>Enable/disable sending 127 messages to the left MIDI DIN out with every clock pulse"
"<RS0_PRS>[1,2]SET_VAR(7,1) [2,2]SET_VAR(7,0)"
"<RS1_LBL>TEST"
"<RS1_INF>TEST"
"<RS1_PRS>[@0,1]MSG_LFT(0X80,1,1) [@0,1]MSG_LFT(0X80,1,2) [@0,1]MSG_LFT(0X80,1,3) [@0,1]MSG_LFT(0X80,1,4) [@0,1]MSG_LFT(0X80,1,5) [@0,1]MSG_LFT(0X80,1,6) [@0,1]MSG_LFT(0X80,1,7) [@0,1]MSG_LFT(0X80,1,8) [@0,1]MSG_LFT(0X80,1,9) [@0,1]MSG_LFT(0X80,1,10) [@0,1]MSG_LFT(0X80,1,11) [@0,1]MSG_LFT(0X80,1,12) [@0,1]MSG_LFT(0X80,1,13) [@0,1]MSG_LFT(0X80,1,14) [@0,1]MSG_LFT(0X80,1,15) [@0,1]MSG_LFT(0X80,1,16) [@0,1]MSG_LFT(0X80,1,17) [@0,1]MSG_LFT(0X80,1,18) [@0,1]MSG_LFT(0X80,1,19) [@0,1]MSG_LFT(0X80,1,20) [@0,1]MSG_LFT(0X80,1,21) [@0,1]MSG_LFT(0X80,1,22) [@0,1]MSG_LFT(0X80,1,23) [@0,1]MSG_LFT(0X80,1,24) [@0,1]MSG_LFT(0X80,1,25) [@0,1]MSG_LFT(0X80,1,26) [@0,1]MSG_LFT(0X80,1,27) [@0,1]MSG_LFT(0X80,1,28) [@0,1]MSG_LFT(0X80,1,29) [@0,1]MSG_LFT(0X80,1,30) [@0,1]MSG_LFT(0X80,1,31) [@0,1]MSG_LFT(0X80,1,32) [@0,1]MSG_LFT(0X80,1,33) [@0,1]MSG_LFT(0X80,1,34) [@0,1]MSG_LFT(0X80,1,35) [@0,1]MSG_LFT(0X80,1,36) [@0,1]MSG_LFT(0X80,1,37) [@0,1]MSG_LFT(0X80,1,38) [@0,1]MSG_LFT(0X80,1,39) [@0,1]MSG_LFT(0X80,1,40) [@0,1]MSG_LFT(0X80,1,41) [@0,1]MSG_LFT(0X80,1,42) [@0,1]MSG_LFT(0X80,1,43) [@0,1]MSG_LFT(0X80,1,44) [@0,1]MSG_LFT(0X80,1,45) [@0,1]MSG_LFT(0X80,1,46) [@0,1]MSG_LFT(0X80,1,47) [@0,1]MSG_LFT(0X80,1,48) [@0,1]MSG_LFT(0X80,1,49) [@0,1]MSG_LFT(0X80,1,50) [@0,1]MSG_LFT(0X80,1,51) [@0,1]MSG_LFT(0X80,1,52) [@0,1]MSG_LFT(0X80,1,53) [@0,1]MSG_LFT(0X80,1,54) [@0,1]MSG_LFT(0X80,1,55) [@0,1]MSG_LFT(0X80,1,56) [@0,1]MSG_LFT(0X80,1,57) [@0,1]MSG_LFT(0X80,1,58) [@0,1]MSG_LFT(0X80,1,59) [@0,1]MSG_LFT(0X80,1,60) [@0,1]MSG_LFT(0X80,1,61) [@0,1]MSG_LFT(0X80,1,62) [@0,1]MSG_LFT(0X80,1,63) [@0,1]MSG_LFT(0X80,1,64) [@0,1]MSG_LFT(0X80,1,65) [@0,1]MSG_LFT(0X80,1,66) [@0,1]MSG_LFT(0X80,1,67) [@0,1]MSG_LFT(0X80,1,68) [@0,1]MSG_LFT(0X80,1,69) [@0,1]MSG_LFT(0X80,1,70) [@0,1]MSG_LFT(0X80,1,71) [@0,1]MSG_LFT(0X80,1,72) [@0,1]MSG_LFT(0X80,1,73) [@0,1]MSG_LFT(0X80,1,74) [@0,1]MSG_LFT(0X80,1,75) [@0,1]MSG_LFT(0X80,1,76) [@0,1]MSG_LFT(0X80,1,77) [@0,1]MSG_LFT(0X80,1,78) [@0,1]MSG_LFT(0X80,1,79) [@0,1]MSG_LFT(0X80,1,80) [@0,1]MSG_LFT(0X80,1,81) [@0,1]MSG_LFT(0X80,1,82) [@0,1]MSG_LFT(0X80,1,83) [@0,1]MSG_LFT(0X80,1,84) [@0,1]MSG_LFT(0X80,1,85) [@0,1]MSG_LFT(0X80,1,86) [@0,1]MSG_LFT(0X80,1,87) [@0,1]MSG_LFT(0X80,1,88) [@0,1]MSG_LFT(0X80,1,89) [@0,1]MSG_LFT(0X80,1,90) [@0,1]MSG_LFT(0X80,1,91) [@0,1]MSG_LFT(0X80,1,92) [@0,1]MSG_LFT(0X80,1,93) [@0,1]MSG_LFT(0X80,1,94) [@0,1]MSG_LFT(0X80,1,95) [@0,1]MSG_LFT(0X80,1,96) [@0,1]MSG_LFT(0X80,1,97) [@0,1]MSG_LFT(0X80,1,98) [@0,1]MSG_LFT(0X80,1,99) [@0,1]MSG_LFT(0X80,1,100) [@0,1]MSG_LFT(0X80,1,101) [@0,1]MSG_LFT(0X80,1,102) [@0,1]MSG_LFT(0X80,1,103) [@0,1]MSG_LFT(0X80,1,104) [@0,1]MSG_LFT(0X80,1,105) [@0,1]MSG_LFT(0X80,1,106) [@0,1]MSG_LFT(0X80,1,107) [@0,1]MSG_LFT(0X80,1,108) [@0,1]MSG_LFT(0X80,1,109) [@0,1]MSG_LFT(0X80,1,110) [@0,1]MSG_LFT(0X80,1,111) [@0,1]MSG_LFT(0X80,1,112) [@0,1]MSG_LFT(0X80,1,113) [@0,1]MSG_LFT(0X80,1,114) [@0,1]MSG_LFT(0X80,1,115) [@0,1]MSG_LFT(0X80,1,116) [@0,1]MSG_LFT(0X80,1,117) [@0,1]MSG_LFT(0X80,1,118) [@0,1]MSG_LFT(0X80,1,119) [@0,1]MSG_LFT(0X80,1,120) [@0,1]MSG_LFT(0X80,1,121) [@0,1]MSG_LFT(0X80,1,122) [@0,1]MSG_LFT(0X80,1,123) [@0,1]MSG_LFT(0X80,1,124) [@0,1]MSG_LFT(0X80,1,125) [@0,1]MSG_LFT(0X80,1,126) [@0,1]MSG_LFT(0X80,1,127)"

"<PRESET>"
"<PST_LBL>TRANSPRT"
"<PST_DSC>Set BPM using the left\nfoot switch. The right\nfoot switch alternately\nstarts or stops the\ntransport. The default\nBPM is set to 110 when\nentering the preset."
"<PST_CLK>[1,1]MSG_USB(0xF8) [1,1]MSG_LFT(0xF8) [1,1]MSG_RGT(0xF8)"
"<PST_ENT>[1,1]SET_BPM(110)"
"<LS0_LBL>TAP BPM"
"<LS0_INF>Tap the left foot switch\nto set the BPM."
"<LS0_PRS>[1,1]TAP_BPM(1) [1,1]MSG_BLE(0x90,0x3c,0x80)"
"<RS0_LBL>TRNSPRT"
"<RS0_INF>Tap the right foot switch\nto start or stop the\ntransport. Each alternate\npress sends either start\nor stop MIDI realtime\ncommands."
"<RS0_PRS>[1,2]MSG_LFT(0XFA) [2,2]MSG_LFT(0XFC) [1,2]MSG_RGT(0XB1,72,127) [2,2]MSG_RGT(0XB1,72,0)"

"<PRESET>"
"<PST_LBL>LOOPER1"
"<PST_DSC>Control a looper over Bluetooth LE, by sending MIDI CC commands using the foot switches, on MIDI channel 0."
"<PST_CLK>"
"<PST_ENT>[1,1]SET_BPM(120)"
"<PST_EXT>"
"<LS0_LBL>LOOP"
"<LS0_INF>Press to (rec / play / stop)"
"<LS0_PRS>[1,1]MSG_BLE(0xB0,0x10,0xff)"
"<RS0_LBL>NEXT"
"<RS0_INF>Next clip. Press and hold to clear."
"<RS0_RLS>[1,1]MSG_BLE(0xB0,0x11,0xff)"
"<RS0_TIM>[1,1]MSG_BLE(0xB0,0x12,0xff)"

"<PRESET>"
"<PST_LBL>VOLCA D1"
"<PST_DSC>The main foot switches\ntrigger channels 1 & 2 on\nthe VOLCA DRUM."
"<LS0_LBL>KICK"
"<LS0_INF>Trigger channel 1 on the\nVOLCA DRUM."
"<LS0_PRS>[1,1]MSG_LFT(0X90,1,127) [1,1]MSG_RGT(0X90,1,127)"
"<LS0_RLS>[1,1]MSG_LFT(0X80,1,127) [1,1]MSG_RGT(0X80,1,127)"
"<RS0_LBL>SNARE"
"<RS0_INF>Trigger channel 2 on the\nVOLCA DRUM."
"<RS0_PRS>[1,1]MSG_LFT(0X91,1,127) [1,1]MSG_RGT(0X91,1,127)"
"<RS0_RLS>[1,1]MSG_LFT(0X81,1,127) [1,1]MSG_RGT(0X81,1,127)"

"<PRESET>"
"<PST_LBL>VOLCA D2"
"<PST_DSC>Start/stop the transport using LS0 and unmute/mute additional voices on the Volca Drum using RS0. The same messages are sent to both DIN-5 MIDI ports."
"<PST_CLK>[1,1]MSG_LFT(0xF8) [1,1]MSG_RGT(0xF8)"
"<LS0_LBL>ON/OFF"
"<LS0_INF>Tap the left foot switch to start/stop transport using MIDI realtime commands"
"<LS0_PRS>[1,2]MSG_LFT(0XFA) [2,2]MSG_LFT(0XFC) [1,2]MSG_RGT(0XFA) [2,2]MSG_RGT(0XFC)"
"<RS0_LBL>LEVEL"
"<RS0_INF>Mute/unmute Korg Volca Drum voices 4,5,6"
"<RS0_PRS>[1,1]MSG_LFT(0XB6,56,127) [1,1]MSG_LFT(0XB6,81,127) [1,1]MSG_LFT(0XB6,90,127)"
"<RS0_RLS>[1,1]MSG_LFT(0XB6,56,0) [1,1]MSG_LFT(0XB6,81,0) [1,1]MSG_LFT(0XB6,90,0)"
"<RS0_RLL>[1,1]MSG_LFT(0XB6,56,0) [1,1]MSG_LFT(0XB6,81,0) [1,1]MSG_LFT(0XB6,90,0)"

"<PRESET>"
"<PST_LBL>KAOSS V1"
"<PST_DSC>Set BPM using the left\nfoot switch. The right\nfoot switch alternately\nstarts or stops the\ntransport. The default\nBPM is set to 110 when\nentering the preset.\nDoes NOT re-sync."
"<PST_CLK>[1,1]MSG_USB(0xF8) [1,1]MSG_LFT(0xF8) [1,1]MSG_RGT(0xF8)"
"<PST_ENT>[1,1]SET_BPM(110)"
"<LS0_LBL>TAP BPM"
"<LS0_INF>Tap the left foot switch\nto set the BPM."
"<LS0_PRS>[1,1]TAP_BPM(1)"
"<RS0_LBL>TRNSPRT"
"<RS0_INF>Tap the right foot switch\nto start or stop the\ntransport. Each alternate\npress sends either start\nor stop MIDI realtime\ncommands."
"<RS0_PRS>[1,2]MSG_LFT(0XFA) [2,2]MSG_LFT(0XFC) [1,2]MSG_RGT(0XB1,72,127) [2,2]MSG_RGT(0XB1,72,0)"

"<PRESET>"
"<PST_LBL>KAOSS V2"
"<PST_DSC>Set BPM using the left\nfoot switch. The right\nfoot switch alternately\nstarts or stops the\ntransport. The default\nBPM is set to 110 when\nentering the preset.\nThe MIDI clock re-syncs\nevery 16 beats: on the\nleft DIN-5 MIDI realtime\nstart/stop commands are\n sent; on the right DIN-5 MIDI touchpad off/on messages are sent."
"<PST_CLK>[1,1]MSG_USB(0xF8) [1,1]MSG_LFT(0xF8) [1,1]MSG_RGT(0xF8) [@0,96]MSG_LFT(0XFC) [@0,96]MSG_LFT(0XFA) [@0,96]MSG_RGT(0XB1,72,0) [@0,96]MSG_RGT(0XB1,72,127)"
"<PST_ENT>[1,1]SET_BPM(1) [1,1]SET_VAR(0,0)"
"<LS0_LBL>TAP BPM"
"<LS0_INF>Tap the left foot switch\nto set the BPM."
"<LS0_PRS>[1,1]TAP_BPM(1)"
"<RS0_LBL>TRNSPRT"
"<RS0_INF>Tap the right foot switch\nto start or stop the\ntransport. Each alternate\npress either sets or\nresets a variable that \ncontrols whether sync\nMIDI messages are sent."
"<RS0_PRS>[1,2]SET_VAR(0,1) [1,2]SET_BPM(0) [2,2]SET_VAR(0,0) [2,2]MSG_LFT(0XFC) [2,2]MSG_RGT(0XB1,72,0)"

"<PRESET>"
"<PST_LBL>EMPTY"
"<PST_DSC>"
"<PST_CLK>"
"<PST_ENT>"
"<PST_EXT>"
"<LS0_LBL>"
"<LS0_INF>"
"<LS0_PRS>"
"<LS0_RLS>"
"<LS0_RLL>"
"<LS0_TIM>"
"<LS1_LBL>"
"<LS1_INF>"
"<LS1_PRS>"
"<LS1_RLS>"
"<LS1_RLL>"
"<LS1_TIM>"
"<LS2_LBL>"
"<LS2_INF>"
"<LS2_PRS>"
"<LS2_RLS>"
"<LS2_RLL>"
"<LS2_TIM>"
"<RS0_LBL>"
"<RS0_INF>"
"<RS0_PRS>"
"<RS0_RLS>"
"<RS0_RLL>"
"<RS0_TIM>"
"<RS1_LBL>"
"<RS1_INF>"
"<RS1_PRS>"
"<RS1_RLS>"
"<RS1_RLL>"
"<RS1_TIM>"
"<RS2_LBL>"
"<RS2_INF>"
"<RS2_PRS>"
"<RS2_RLS>"
"<RS2_RLL>"
"<RS2_TIM>"

"\0";  //always end string with 0x00
