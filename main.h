
typedef enum {
    EVENT_NONE,
    EVENT_INF,
    EVENT_PRS,
    EVENT_RLS,
    EVENT_RLL,
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
    bool system;
    bool preset;
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
    COLOR_RED
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

static uint64_t MIDI_CLOCK_COUNTER = 0;
static struct repeating_timer MIDI_CLOCK_TIMER;
static int64_t MIDI_CLOCK_TIMER_US = (int64_t) (-60000000 / 100 / 24);  //Negative delay means we will call repeating_timer_callback, and call it again exactly ???ms later regardless of how long the callback took to execute

//static char MIDI_SYSEX_MESSAGE[100];
static bool MIDI_SYSEX_BUSY = false;
static uint32_t MIDI_SYSEX_INDEX = 0;

static SCREEN SCREENS[64];  //somewhat arbitrary limit
static uint8_t CURR_SCREEN_INDEX = 0;
static uint8_t MAX_SCREEN_INDEX = 0;

static TUSB TUSB_STATUS;
static VIEW CURR_VIEW;  //only ever one VIEW per SCREEN active at a time - this way the active scan can continue while showing help

static uint8_t USER_VAR[8] = {0,0,0,0,0,0,0,0};  //eight user variables - eight sounds like a good number?

static int8_t SNAKE_DIRECTION = 0;

#define BPM_BUFFER_WIDTH (61*3/2)  //3 characters wide
#define BPM_BUFFER_HEIGHT 104  //1 character high
static uint8_t BPM_BUFFER[BPM_BUFFER_WIDTH][BPM_BUFFER_HEIGHT];  //hard coded limit depends on size of B_BPM_BIG_HELP

// Flash-based address of the FACTORY and USER confis
#define PRESETS_SIZE (FLASH_SECTOR_SIZE * 32)  //4096 * 32 = 131076
#define FACTORY_FLASH_OFFSET (PICO_FLASH_SIZE_BYTES - PRESETS_SIZE * 2)
const uint8_t *FACTORY_FLASH = (const uint8_t *) (XIP_BASE + FACTORY_FLASH_OFFSET);
#define USER_FLASH_OFFSET (PICO_FLASH_SIZE_BYTES - PRESETS_SIZE * 1)
const uint8_t *USER_FLASH = (const uint8_t *) (XIP_BASE + USER_FLASH_OFFSET);

#define LCD_WIDTH 320
#define LCD_HEIGHT 240
#define SNAKE_SIZE 100

//these functions use buttons and are called by buttons, so difficult to order code sequentially
void writeSysexConfigToUserFlash();
void writeFactoryFlashToUserFlash();
void sendSysexConfig();
void loadFirmware();

//config and versioning settings
static char FIRMWARE_VERSION[5] = "0.21";  //hardcoded firmware version, to be updated manually by author
static char FACTORY_VERSION[5] = "----";  //placeholder value, will be updated by updateFactoryFlash() 
static char *PRESETS_VERSION;  //presets version, assigned when parsing PRESETS_TEXT from USER_FLASH
static char *PRESETS_DEFAULT;  //default preset, assigned when parsing PRESETS_TEXT from USER_FLASH
static char PRESETS_TEXT[PRESETS_SIZE] = ""  //factory presets
"<SYSTEM>"
"<VERSION>0.21"
"<DEFAULT>TRANSPRT"

"<PRESET>"
"<PST_LBL>SNAKE"
"<PST_DSC>A fun game. Use the main\nfoot switches to turn left\nor right."
"<PST_CLK>"
"<PST_ENT>"
"<PST_EXT>"
"<LS0_LBL>LEFT"
"<LS0_INF>"
"<LS0_PRS>"
"<LS0_RLS>"
"<LS0_RLL>"
"<LS1_LBL>"
"<LS1_INF>"
"<LS1_PRS>"
"<LS1_RLS>"
"<LS1_RLL>"
"<LS2_LBL>"
"<LS2_INF>"
"<LS2_PRS>"
"<LS2_RLS>"
"<LS2_RLL>"
"<RS0_LBL>RIGHT"
"<RS0_INF>"
"<RS0_PRS>"
"<RS0_RLS>"
"<RS0_RLL>"
"<RS1_LBL>"
"<RS1_INF>"
"<RS1_PRS>"
"<RS1_RLS>"
"<RS1_RLL>"
"<RS2_LBL>"
"<RS2_INF>"
"<RS2_PRS>"
"<RS2_RLS>"
"<RS2_RLL>"

"<PRESET>"
"<PST_LBL>VERIFY"
"<PST_DSC>Verify that all switches and MIDI ports work, by sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages to all MIDI ports for each switch that is PRESSED, RELEASED or TIMED OUT. Also sends a MIDI system real time clock message at 120 BPM."
"<PST_CLK>[1,1]MSG_USB(0xF8)"
"<PST_ENT>[1,1]SET_BPM(120)"
"<PST_EXT>[1,1]SET_BPM(120)"
"<LS0_LBL>LFT SW0"
"<LS0_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS0_PRS>[1,1]MSG_USB(0X90,1,127) [1,1]MSG_LFT(0X90,4,127) [1,1]MSG_RGT(0X90,7,127)"
"<LS0_RLS>[1,1]MSG_USB(0X80,1,127) [1,1]MSG_LFT(0X80,4,127) [1,1]MSG_RGT(0X80,7,127)"
"<LS0_RLL>[1,1]MSG_USB(0XA0,1,127) [1,1]MSG_LFT(0XA0,4,127) [1,1]MSG_RGT(0XA0,7,127)"
"<LS1_LBL>LFT SW1"
"<LS1_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS1_PRS>[1,1]MSG_USB(0X90,2,127) [1,1]MSG_LFT(0X90,5,127) [1,1]MSG_RGT(0X90,8,127)"
"<LS1_RLS>[1,1]MSG_USB(0X80,2,127) [1,1]MSG_LFT(0X80,5,127) [1,1]MSG_RGT(0X80,8,127)"
"<LS1_RLL>[1,1]MSG_USB(0XA0,2,127) [1,1]MSG_LFT(0XA0,5,127) [1,1]MSG_RGT(0XA0,8,127)"
"<LS2_LBL>LFT SW2"
"<LS2_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS2_PRS>[1,1]MSG_USB(0X90,3,127) [1,1]MSG_LFT(0X90,6,127) [1,1]MSG_RGT(0X90,9,127)"
"<LS2_RLS>[1,1]MSG_USB(0X80,3,127) [1,1]MSG_LFT(0X80,6,127) [1,1]MSG_RGT(0X80,9,127)"
"<LS2_RLL>[1,1]MSG_USB(0XA0,3,127) [1,1]MSG_LFT(0XA0,6,127) [1,1]MSG_RGT(0XA0,9,127)"
"<RS0_LBL>RGT SW0"
"<RS0_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS0_PRS>[1,1]MSG_USB(0X90,11,127) [1,1]MSG_LFT(0X90,14,127) [1,1]MSG_RGT(0X90,17,127)"
"<RS0_RLS>[1,1]MSG_USB(0X80,11,127) [1,1]MSG_LFT(0X80,14,127) [1,1]MSG_RGT(0X80,17,127)"
"<RS0_RLL>[1,1]MSG_USB(0XA0,11,127) [1,1]MSG_LFT(0XA0,14,127) [1,1]MSG_RGT(0XA0,17,127)"
"<RS1_LBL>RGT SW1"
"<RS1_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS1_PRS>[1,1]MSG_USB(0X90,12,127) [1,1]MSG_LFT(0X90,15,127) [1,1]MSG_RGT(0X90,18,127)"
"<RS1_RLS>[1,1]MSG_USB(0X80,12,127) [1,1]MSG_LFT(0X80,15,127) [1,1]MSG_RGT(0X80,18,127)"
"<RS1_RLL>[1,1]MSG_USB(0XA0,12,127) [1,1]MSG_LFT(0XA0,15,127) [1,1]MSG_RGT(0XA0,18,127)"
"<RS2_LBL>RGT SW2"
"<RS2_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS2_PRS>[1,1]MSG_USB(0X90,13,127) [1,1]MSG_LFT(0X90,16,127) [1,1]MSG_RGT(0X90,19,127)"
"<RS2_RLS>[1,1]MSG_USB(0X80,13,127) [1,1]MSG_LFT(0X80,16,127) [1,1]MSG_RGT(0X80,19,127)"
"<RS2_RLL>[1,1]MSG_USB(0XA0,13,127) [1,1]MSG_LFT(0XA0,16,127) [1,1]MSG_RGT(0XA0,19,127)"

"<PRESET>"
"<PST_LBL>TRANSPRT"
"<PST_DSC>Set BPM using the left\nfoot switch. The right\nfoot switch alternately\nstarts or stops the\ntransport. The default\nBPM is set to 110 when\nentering the preset."
"<PST_CLK>[1,1]MSG_USB(0xF8) [1,1]MSG_LFT(0xF8) [1,1]MSG_RGT(0xF8)"
"<PST_ENT>[1,1]SET_BPM(110)"
"<PST_EXT>"
"<LS0_LBL>TAP BPM"
"<LS0_INF>Tap the left foot switch\nto set the BPM."
"<LS0_PRS>[1,1]TAP_BPM(1)"
"<LS0_RLS>"
"<LS0_RLL>"
"<LS1_LBL>"
"<LS1_INF>"
"<LS1_PRS>"
"<LS1_RLS>"
"<LS1_RLL>"
"<LS2_LBL>"
"<LS2_INF>"
"<LS2_PRS>"
"<LS2_RLS>"
"<LS2_RLL>"
"<RS0_LBL>TRNSPRT"
"<RS0_INF>Tap the right foot switch\nto start or stop the\ntransport. Each alternate\npress sends either start\nor stop MIDI realtime\ncommands."
"<RS0_PRS>[1,2]MSG_LFT(0XFA) [2,2]MSG_LFT(0XFC) [1,2]MSG_RGT(0XB1,72,127) [2,2]MSG_RGT(0XB1,72,0)"
"<RS0_RLS>"
"<RS0_RLL>"
"<RS1_LBL>"
"<RS1_INF>"
"<RS1_PRS>"
"<RS1_RLS>"
"<RS1_RLL>"
"<RS2_LBL>"
"<RS2_INF>"
"<RS2_PRS>"
"<RS2_RLS>"
"<RS2_RLL>"

"<PRESET>"
"<PST_LBL>VOLCA D1"
"<PST_DSC>The main foot switches\ntrigger channels 1 & 2 on\nthe VOLCA DRUM."
"<PST_CLK>"
"<PST_ENT>"
"<PST_EXT>"
"<LS0_LBL>KICK"
"<LS0_INF>Trigger channel 1 on the\nVOLCA DRUM."
"<LS0_PRS>[1,1]MSG_LFT(0X90,1,127) [1,1]MSG_RGT(0X90,1,127)"
"<LS0_RLS>[1,1]MSG_LFT(0X80,1,127) [1,1]MSG_RGT(0X80,1,127)"
"<LS0_RLL>"
"<LS1_LBL>"
"<LS1_INF>"
"<LS1_PRS>"
"<LS1_RLS>"
"<LS1_RLL>"
"<LS2_LBL>"
"<LS2_INF>"
"<LS2_PRS>"
"<LS2_RLS>"
"<LS2_RLL>"
"<RS0_LBL>SNARE"
"<RS0_INF>Trigger channel 2 on the\nVOLCA DRUM."
"<RS0_PRS>[1,1]MSG_LFT(0X91,1,127) [1,1]MSG_RGT(0X91,1,127)"
"<RS0_RLS>[1,1]MSG_LFT(0X81,1,127) [1,1]MSG_RGT(0X81,1,127)"
"<RS0_RLL>"
"<RS1_LBL>"
"<RS1_INF>"
"<RS1_PRS>"
"<RS1_RLS>"
"<RS1_RLL>"
"<RS2_LBL>"
"<RS2_INF>"
"<RS2_PRS>"
"<RS2_RLS>"
"<RS2_RLL>"

"<PRESET>"
"<PST_LBL>VOLCA D2"
"<PST_DSC>Start/stop the transport using LS0 and unmute/mute additional voices on the Volca Drum using RS0. The same messages are sent to both DIN-5 MIDI ports."
"<PST_CLK>[1,1]MSG_LFT(0xF8) [1,1]MSG_RGT(0xF8)"
"<PST_ENT>"
"<PST_EXT>"
"<LS0_LBL>ON/OFF"
"<LS0_INF>Tap the left foot switch to start/stop transport using MIDI realtime commands"
"<LS0_PRS>[1,2]MSG_LFT(0XFA) [2,2]MSG_LFT(0XFC) [1,2]MSG_RGT(0XFA) [2,2]MSG_RGT(0XFC)"
"<LS0_RLS>"
"<LS0_RLL>"
"<LS1_LBL>"
"<LS1_INF>"
"<LS1_PRS>"
"<LS1_RLS>"
"<LS1_RLL>"
"<LS2_LBL>"
"<LS2_INF>"
"<LS2_PRS>"
"<LS2_RLS>"
"<LS2_RLL>"
"<RS0_LBL>LEVEL"
"<RS0_INF>Mute/unmute Korg Volca Drum voices 4,5,6"
"<RS0_PRS>[1,1]MSG_LFT(0XB6,56,127) [1,1]MSG_LFT(0XB6,81,127) [1,1]MSG_LFT(0XB6,90,127)"
"<RS0_RLS>[1,1]MSG_LFT(0XB6,56,0) [1,1]MSG_LFT(0XB6,81,0) [1,1]MSG_LFT(0XB6,90,0)"
"<RS0_RLL>[1,1]MSG_LFT(0XB6,56,0) [1,1]MSG_LFT(0XB6,81,0) [1,1]MSG_LFT(0XB6,90,0)"
"<RS1_LBL>"
"<RS1_INF>"
"<RS1_PRS>"
"<RS1_RLS>"
"<RS1_RLL>"
"<RS2_LBL>"
"<RS2_INF>"
"<RS2_PRS>"
"<RS2_RLS>"
"<RS2_RLL>"

"<PRESET>"
"<PST_LBL>KAOSS V1"
"<PST_DSC>Set BPM using the left\nfoot switch. The right\nfoot switch alternately\nstarts or stops the\ntransport. The default\nBPM is set to 110 when\nentering the preset.\nDoes NOT re-sync."
"<PST_CLK>[1,1]MSG_USB(0xF8) [1,1]MSG_LFT(0xF8) [1,1]MSG_RGT(0xF8)"
"<PST_ENT>[1,1]SET_BPM(110)"
"<PST_EXT>"
"<LS0_LBL>TAP BPM"
"<LS0_INF>Tap the left foot switch\nto set the BPM."
"<LS0_PRS>[1,1]TAP_BPM(1)"
"<LS0_RLS>"
"<LS0_RLL>"
"<LS1_LBL>"
"<LS1_INF>"
"<LS1_PRS>"
"<LS1_RLS>"
"<LS1_RLL>"
"<LS2_LBL>"
"<LS2_INF>"
"<LS2_PRS>"
"<LS2_RLS>"
"<LS2_RLL>"
"<RS0_LBL>TRNSPRT"
"<RS0_INF>Tap the right foot switch\nto start or stop the\ntransport. Each alternate\npress sends either start\nor stop MIDI realtime\ncommands."
"<RS0_PRS>[1,2]MSG_LFT(0XFA) [2,2]MSG_LFT(0XFC) [1,2]MSG_RGT(0XB1,72,127) [2,2]MSG_RGT(0XB1,72,0)"
"<RS0_RLS>"
"<RS0_RLL>"
"<RS1_LBL>"
"<RS1_INF>"
"<RS1_PRS>"
"<RS1_RLS>"
"<RS1_RLL>"
"<RS2_LBL>"
"<RS2_INF>"
"<RS2_PRS>"
"<RS2_RLS>"
"<RS2_RLL>"

"<PRESET>"
"<PST_LBL>KAOSS V2"
"<PST_DSC>Set BPM using the left\nfoot switch. The right\nfoot switch alternately\nstarts or stops the\ntransport. The default\nBPM is set to 110 when\nentering the preset.\nThe MIDI clock re-syncs every 16 beats: on the left DIN-5 MIDI realtime start/stop commands are sent; on the right DIN-5 MIDI touchpad off/on messages are sent."
"<PST_CLK>[1,1]MSG_USB(0xF8) [1,1]MSG_LFT(0xF8) [1,1]MSG_RGT(0xF8) [@0,96]MSG_LFT(0XFC) [@0,96]MSG_LFT(0XFA) [@0,96]MSG_RGT(0XB1,72,0) [@0,96]MSG_RGT(0XB1,72,127)"
"<PST_ENT>[1,1]SET_BPM(1) [1,1]SET_PV1(0)"
"<PST_EXT>"
"<LS0_LBL>TAP BPM"
"<LS0_INF>Tap the left foot switch\nto set the BPM."
"<LS0_PRS>[1,1]TAP_BPM(1)"
"<LS0_RLS>"
"<LS0_RLL>"
"<LS1_LBL>"
"<LS1_INF>"
"<LS1_PRS>"
"<LS1_RLS>"
"<LS1_RLL>"
"<LS2_LBL>"
"<LS2_INF>"
"<LS2_PRS>"
"<LS2_RLS>"
"<LS2_RLL>"
"<RS0_LBL>TRNSPRT"
"<RS0_INF>Tap the right foot switch\nto start or stop the\ntransport. Each alternate\npress either sets or\nresets a variable that \ncontrols whether sync\nMIDI messages are sent."
"<RS0_PRS>[1,2]SET_VAR(0,1) [1,2]SET_BPM(0) [2,2]SET_VAR(0,0) [2,2]MSG_LFT(0XFC) [2,2]MSG_RGT(0XB1,72,0)"
"<RS0_RLS>"
"<RS0_RLL>"
"<RS1_LBL>"
"<RS1_INF>"
"<RS1_PRS>"
"<RS1_RLS>"
"<RS1_RLL>"
"<RS2_LBL>"
"<RS2_INF>"
"<RS2_PRS>"
"<RS2_RLS>"
"<RS2_RLL>"

"<PRESET>"
"<PST_LBL>VERIFY3"
"<PST_DSC>Verify that all switches and MIDI ports work, by sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages to all MIDI ports for each switch that is PRESSED, RELEASED or TIMED OUT. Also sends a MIDI system real time clock message at 120 BPM."
"<PST_CLK>[1,1]MSG_USB(0xF8)"
"<PST_ENT>[1,1]SET_BPM(120)"
"<PST_EXT>[1,1]SET_BPM(120)"
"<LS0_LBL>LFT SW0"
"<LS0_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS0_PRS>[1,1]MSG_USB(0X90,1,127) [1,1]MSG_LFT(0X90,4,127) [1,1]MSG_RGT(0X90,7,127)"
"<LS0_RLS>[1,1]MSG_USB(0X80,1,127) [1,1]MSG_LFT(0X80,4,127) [1,1]MSG_RGT(0X80,7,127)"
"<LS0_RLL>[1,1]MSG_USB(0XA0,1,127) [1,1]MSG_LFT(0XA0,4,127) [1,1]MSG_RGT(0XA0,7,127)"
"<LS1_LBL>LFT SW1"
"<LS1_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS1_PRS>[1,1]MSG_USB(0X90,2,127) [1,1]MSG_LFT(0X90,5,127) [1,1]MSG_RGT(0X90,8,127)"
"<LS1_RLS>[1,1]MSG_USB(0X80,2,127) [1,1]MSG_LFT(0X80,5,127) [1,1]MSG_RGT(0X80,8,127)"
"<LS1_RLL>[1,1]MSG_USB(0XA0,2,127) [1,1]MSG_LFT(0XA0,5,127) [1,1]MSG_RGT(0XA0,8,127)"
"<LS2_LBL>LFT SW2"
"<LS2_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS2_PRS>[1,1]MSG_USB(0X90,3,127) [1,1]MSG_LFT(0X90,6,127) [1,1]MSG_RGT(0X90,9,127)"
"<LS2_RLS>[1,1]MSG_USB(0X80,3,127) [1,1]MSG_LFT(0X80,6,127) [1,1]MSG_RGT(0X80,9,127)"
"<LS2_RLL>[1,1]MSG_USB(0XA0,3,127) [1,1]MSG_LFT(0XA0,6,127) [1,1]MSG_RGT(0XA0,9,127)"
"<RS0_LBL>RGT SW0"
"<RS0_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS0_PRS>[1,1]MSG_USB(0X90,11,127) [1,1]MSG_LFT(0X90,14,127) [1,1]MSG_RGT(0X90,17,127)"
"<RS0_RLS>[1,1]MSG_USB(0X80,11,127) [1,1]MSG_LFT(0X80,14,127) [1,1]MSG_RGT(0X80,17,127)"
"<RS0_RLL>[1,1]MSG_USB(0XA0,11,127) [1,1]MSG_LFT(0XA0,14,127) [1,1]MSG_RGT(0XA0,17,127)"
"<RS1_LBL>RGT SW1"
"<RS1_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS1_PRS>[1,1]MSG_USB(0X90,12,127) [1,1]MSG_LFT(0X90,15,127) [1,1]MSG_RGT(0X90,18,127)"
"<RS1_RLS>[1,1]MSG_USB(0X80,12,127) [1,1]MSG_LFT(0X80,15,127) [1,1]MSG_RGT(0X80,18,127)"
"<RS1_RLL>[1,1]MSG_USB(0XA0,12,127) [1,1]MSG_LFT(0XA0,15,127) [1,1]MSG_RGT(0XA0,18,127)"
"<RS2_LBL>RGT SW2"
"<RS2_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS2_PRS>[1,1]MSG_USB(0X90,13,127) [1,1]MSG_LFT(0X90,16,127) [1,1]MSG_RGT(0X90,19,127)"
"<RS2_RLS>[1,1]MSG_USB(0X80,13,127) [1,1]MSG_LFT(0X80,16,127) [1,1]MSG_RGT(0X80,19,127)"
"<RS2_RLL>[1,1]MSG_USB(0XA0,13,127) [1,1]MSG_LFT(0XA0,16,127) [1,1]MSG_RGT(0XA0,19,127)"

"<PRESET>"
"<PST_LBL>VERIFY4"
"<PST_DSC>Verify that all switches and MIDI ports work, by sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages to all MIDI ports for each switch that is PRESSED, RELEASED or TIMED OUT. Also sends a MIDI system real time clock message at 120 BPM."
"<PST_CLK>[1,1]MSG_USB(0xF8)"
"<PST_ENT>[1,1]SET_BPM(120)"
"<PST_EXT>[1,1]SET_BPM(120)"
"<LS0_LBL>LFT SW0"
"<LS0_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS0_PRS>[1,1]MSG_USB(0X90,1,127) [1,1]MSG_LFT(0X90,4,127) [1,1]MSG_RGT(0X90,7,127)"
"<LS0_RLS>[1,1]MSG_USB(0X80,1,127) [1,1]MSG_LFT(0X80,4,127) [1,1]MSG_RGT(0X80,7,127)"
"<LS0_RLL>[1,1]MSG_USB(0XA0,1,127) [1,1]MSG_LFT(0XA0,4,127) [1,1]MSG_RGT(0XA0,7,127)"
"<LS1_LBL>LFT SW1"
"<LS1_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS1_PRS>[1,1]MSG_USB(0X90,2,127) [1,1]MSG_LFT(0X90,5,127) [1,1]MSG_RGT(0X90,8,127)"
"<LS1_RLS>[1,1]MSG_USB(0X80,2,127) [1,1]MSG_LFT(0X80,5,127) [1,1]MSG_RGT(0X80,8,127)"
"<LS1_RLL>[1,1]MSG_USB(0XA0,2,127) [1,1]MSG_LFT(0XA0,5,127) [1,1]MSG_RGT(0XA0,8,127)"
"<LS2_LBL>LFT SW2"
"<LS2_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS2_PRS>[1,1]MSG_USB(0X90,3,127) [1,1]MSG_LFT(0X90,6,127) [1,1]MSG_RGT(0X90,9,127)"
"<LS2_RLS>[1,1]MSG_USB(0X80,3,127) [1,1]MSG_LFT(0X80,6,127) [1,1]MSG_RGT(0X80,9,127)"
"<LS2_RLL>[1,1]MSG_USB(0XA0,3,127) [1,1]MSG_LFT(0XA0,6,127) [1,1]MSG_RGT(0XA0,9,127)"
"<RS0_LBL>RGT SW0"
"<RS0_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS0_PRS>[1,1]MSG_USB(0X90,11,127) [1,1]MSG_LFT(0X90,14,127) [1,1]MSG_RGT(0X90,17,127)"
"<RS0_RLS>[1,1]MSG_USB(0X80,11,127) [1,1]MSG_LFT(0X80,14,127) [1,1]MSG_RGT(0X80,17,127)"
"<RS0_RLL>[1,1]MSG_USB(0XA0,11,127) [1,1]MSG_LFT(0XA0,14,127) [1,1]MSG_RGT(0XA0,17,127)"
"<RS1_LBL>RGT SW1"
"<RS1_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS1_PRS>[1,1]MSG_USB(0X90,12,127) [1,1]MSG_LFT(0X90,15,127) [1,1]MSG_RGT(0X90,18,127)"
"<RS1_RLS>[1,1]MSG_USB(0X80,12,127) [1,1]MSG_LFT(0X80,15,127) [1,1]MSG_RGT(0X80,18,127)"
"<RS1_RLL>[1,1]MSG_USB(0XA0,12,127) [1,1]MSG_LFT(0XA0,15,127) [1,1]MSG_RGT(0XA0,18,127)"
"<RS2_LBL>RGT SW2"
"<RS2_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS2_PRS>[1,1]MSG_USB(0X90,13,127) [1,1]MSG_LFT(0X90,16,127) [1,1]MSG_RGT(0X90,19,127)"
"<RS2_RLS>[1,1]MSG_USB(0X80,13,127) [1,1]MSG_LFT(0X80,16,127) [1,1]MSG_RGT(0X80,19,127)"
"<RS2_RLL>[1,1]MSG_USB(0XA0,13,127) [1,1]MSG_LFT(0XA0,16,127) [1,1]MSG_RGT(0XA0,19,127)"

"<PRESET>"
"<PST_LBL>VERIFY5"
"<PST_DSC>Verify that all switches and MIDI ports work, by sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages to all MIDI ports for each switch that is PRESSED, RELEASED or TIMED OUT. Also sends a MIDI system real time clock message at 120 BPM."
"<PST_CLK>[1,1]MSG_USB(0xF8)"
"<PST_ENT>[1,1]SET_BPM(120)"
"<PST_EXT>[1,1]SET_BPM(120)"
"<LS0_LBL>LFT SW0"
"<LS0_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS0_PRS>[1,1]MSG_USB(0X90,1,127) [1,1]MSG_LFT(0X90,4,127) [1,1]MSG_RGT(0X90,7,127)"
"<LS0_RLS>[1,1]MSG_USB(0X80,1,127) [1,1]MSG_LFT(0X80,4,127) [1,1]MSG_RGT(0X80,7,127)"
"<LS0_RLL>[1,1]MSG_USB(0XA0,1,127) [1,1]MSG_LFT(0XA0,4,127) [1,1]MSG_RGT(0XA0,7,127)"
"<LS1_LBL>LFT SW1"
"<LS1_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS1_PRS>[1,1]MSG_USB(0X90,2,127) [1,1]MSG_LFT(0X90,5,127) [1,1]MSG_RGT(0X90,8,127)"
"<LS1_RLS>[1,1]MSG_USB(0X80,2,127) [1,1]MSG_LFT(0X80,5,127) [1,1]MSG_RGT(0X80,8,127)"
"<LS1_RLL>[1,1]MSG_USB(0XA0,2,127) [1,1]MSG_LFT(0XA0,5,127) [1,1]MSG_RGT(0XA0,8,127)"
"<LS2_LBL>LFT SW2"
"<LS2_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS2_PRS>[1,1]MSG_USB(0X90,3,127) [1,1]MSG_LFT(0X90,6,127) [1,1]MSG_RGT(0X90,9,127)"
"<LS2_RLS>[1,1]MSG_USB(0X80,3,127) [1,1]MSG_LFT(0X80,6,127) [1,1]MSG_RGT(0X80,9,127)"
"<LS2_RLL>[1,1]MSG_USB(0XA0,3,127) [1,1]MSG_LFT(0XA0,6,127) [1,1]MSG_RGT(0XA0,9,127)"
"<RS0_LBL>RGT SW0"
"<RS0_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS0_PRS>[1,1]MSG_USB(0X90,11,127) [1,1]MSG_LFT(0X90,14,127) [1,1]MSG_RGT(0X90,17,127)"
"<RS0_RLS>[1,1]MSG_USB(0X80,11,127) [1,1]MSG_LFT(0X80,14,127) [1,1]MSG_RGT(0X80,17,127)"
"<RS0_RLL>[1,1]MSG_USB(0XA0,11,127) [1,1]MSG_LFT(0XA0,14,127) [1,1]MSG_RGT(0XA0,17,127)"
"<RS1_LBL>RGT SW1"
"<RS1_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS1_PRS>[1,1]MSG_USB(0X90,12,127) [1,1]MSG_LFT(0X90,15,127) [1,1]MSG_RGT(0X90,18,127)"
"<RS1_RLS>[1,1]MSG_USB(0X80,12,127) [1,1]MSG_LFT(0X80,15,127) [1,1]MSG_RGT(0X80,18,127)"
"<RS1_RLL>[1,1]MSG_USB(0XA0,12,127) [1,1]MSG_LFT(0XA0,15,127) [1,1]MSG_RGT(0XA0,18,127)"
"<RS2_LBL>RGT SW2"
"<RS2_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS2_PRS>[1,1]MSG_USB(0X90,13,127) [1,1]MSG_LFT(0X90,16,127) [1,1]MSG_RGT(0X90,19,127)"
"<RS2_RLS>[1,1]MSG_USB(0X80,13,127) [1,1]MSG_LFT(0X80,16,127) [1,1]MSG_RGT(0X80,19,127)"
"<RS2_RLL>[1,1]MSG_USB(0XA0,13,127) [1,1]MSG_LFT(0XA0,16,127) [1,1]MSG_RGT(0XA0,19,127)"

"<PRESET>"
"<PST_LBL>VERIFY6"
"<PST_DSC>Verify that all switches and MIDI ports work, by sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages to all MIDI ports for each switch that is PRESSED, RELEASED or TIMED OUT. Also sends a MIDI system real time clock message at 120 BPM."
"<PST_CLK>[1,1]MSG_USB(0xF8)"
"<PST_ENT>[1,1]SET_BPM(120)"
"<PST_EXT>[1,1]SET_BPM(120)"
"<LS0_LBL>LFT SW0"
"<LS0_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS0_PRS>[1,1]MSG_USB(0X90,1,127) [1,1]MSG_LFT(0X90,4,127) [1,1]MSG_RGT(0X90,7,127)"
"<LS0_RLS>[1,1]MSG_USB(0X80,1,127) [1,1]MSG_LFT(0X80,4,127) [1,1]MSG_RGT(0X80,7,127)"
"<LS0_RLL>[1,1]MSG_USB(0XA0,1,127) [1,1]MSG_LFT(0XA0,4,127) [1,1]MSG_RGT(0XA0,7,127)"
"<LS1_LBL>LFT SW1"
"<LS1_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS1_PRS>[1,1]MSG_USB(0X90,2,127) [1,1]MSG_LFT(0X90,5,127) [1,1]MSG_RGT(0X90,8,127)"
"<LS1_RLS>[1,1]MSG_USB(0X80,2,127) [1,1]MSG_LFT(0X80,5,127) [1,1]MSG_RGT(0X80,8,127)"
"<LS1_RLL>[1,1]MSG_USB(0XA0,2,127) [1,1]MSG_LFT(0XA0,5,127) [1,1]MSG_RGT(0XA0,8,127)"
"<LS2_LBL>LFT SW2"
"<LS2_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS2_PRS>[1,1]MSG_USB(0X90,3,127) [1,1]MSG_LFT(0X90,6,127) [1,1]MSG_RGT(0X90,9,127)"
"<LS2_RLS>[1,1]MSG_USB(0X80,3,127) [1,1]MSG_LFT(0X80,6,127) [1,1]MSG_RGT(0X80,9,127)"
"<LS2_RLL>[1,1]MSG_USB(0XA0,3,127) [1,1]MSG_LFT(0XA0,6,127) [1,1]MSG_RGT(0XA0,9,127)"
"<RS0_LBL>RGT SW0"
"<RS0_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS0_PRS>[1,1]MSG_USB(0X90,11,127) [1,1]MSG_LFT(0X90,14,127) [1,1]MSG_RGT(0X90,17,127)"
"<RS0_RLS>[1,1]MSG_USB(0X80,11,127) [1,1]MSG_LFT(0X80,14,127) [1,1]MSG_RGT(0X80,17,127)"
"<RS0_RLL>[1,1]MSG_USB(0XA0,11,127) [1,1]MSG_LFT(0XA0,14,127) [1,1]MSG_RGT(0XA0,17,127)"
"<RS1_LBL>RGT SW1"
"<RS1_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS1_PRS>[1,1]MSG_USB(0X90,12,127) [1,1]MSG_LFT(0X90,15,127) [1,1]MSG_RGT(0X90,18,127)"
"<RS1_RLS>[1,1]MSG_USB(0X80,12,127) [1,1]MSG_LFT(0X80,15,127) [1,1]MSG_RGT(0X80,18,127)"
"<RS1_RLL>[1,1]MSG_USB(0XA0,12,127) [1,1]MSG_LFT(0XA0,15,127) [1,1]MSG_RGT(0XA0,18,127)"
"<RS2_LBL>RGT SW2"
"<RS2_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS2_PRS>[1,1]MSG_USB(0X90,13,127) [1,1]MSG_LFT(0X90,16,127) [1,1]MSG_RGT(0X90,19,127)"
"<RS2_RLS>[1,1]MSG_USB(0X80,13,127) [1,1]MSG_LFT(0X80,16,127) [1,1]MSG_RGT(0X80,19,127)"
"<RS2_RLL>[1,1]MSG_USB(0XA0,13,127) [1,1]MSG_LFT(0XA0,16,127) [1,1]MSG_RGT(0XA0,19,127)"

"<PRESET>"
"<PST_LBL>VERIFY7"
"<PST_DSC>Verify that all switches and MIDI ports work, by sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages to all MIDI ports for each switch that is PRESSED, RELEASED or TIMED OUT. Also sends a MIDI system real time clock message at 120 BPM."
"<PST_CLK>[1,1]MSG_USB(0xF8)"
"<PST_ENT>[1,1]SET_BPM(120)"
"<PST_EXT>[1,1]SET_BPM(120)"
"<LS0_LBL>LFT SW0"
"<LS0_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS0_PRS>[1,1]MSG_USB(0X90,1,127) [1,1]MSG_LFT(0X90,4,127) [1,1]MSG_RGT(0X90,7,127)"
"<LS0_RLS>[1,1]MSG_USB(0X80,1,127) [1,1]MSG_LFT(0X80,4,127) [1,1]MSG_RGT(0X80,7,127)"
"<LS0_RLL>[1,1]MSG_USB(0XA0,1,127) [1,1]MSG_LFT(0XA0,4,127) [1,1]MSG_RGT(0XA0,7,127)"
"<LS1_LBL>LFT SW1"
"<LS1_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS1_PRS>[1,1]MSG_USB(0X90,2,127) [1,1]MSG_LFT(0X90,5,127) [1,1]MSG_RGT(0X90,8,127)"
"<LS1_RLS>[1,1]MSG_USB(0X80,2,127) [1,1]MSG_LFT(0X80,5,127) [1,1]MSG_RGT(0X80,8,127)"
"<LS1_RLL>[1,1]MSG_USB(0XA0,2,127) [1,1]MSG_LFT(0XA0,5,127) [1,1]MSG_RGT(0XA0,8,127)"
"<LS2_LBL>LFT SW2"
"<LS2_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS2_PRS>[1,1]MSG_USB(0X90,3,127) [1,1]MSG_LFT(0X90,6,127) [1,1]MSG_RGT(0X90,9,127)"
"<LS2_RLS>[1,1]MSG_USB(0X80,3,127) [1,1]MSG_LFT(0X80,6,127) [1,1]MSG_RGT(0X80,9,127)"
"<LS2_RLL>[1,1]MSG_USB(0XA0,3,127) [1,1]MSG_LFT(0XA0,6,127) [1,1]MSG_RGT(0XA0,9,127)"
"<RS0_LBL>RGT SW0"
"<RS0_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS0_PRS>[1,1]MSG_USB(0X90,11,127) [1,1]MSG_LFT(0X90,14,127) [1,1]MSG_RGT(0X90,17,127)"
"<RS0_RLS>[1,1]MSG_USB(0X80,11,127) [1,1]MSG_LFT(0X80,14,127) [1,1]MSG_RGT(0X80,17,127)"
"<RS0_RLL>[1,1]MSG_USB(0XA0,11,127) [1,1]MSG_LFT(0XA0,14,127) [1,1]MSG_RGT(0XA0,17,127)"
"<RS1_LBL>RGT SW1"
"<RS1_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS1_PRS>[1,1]MSG_USB(0X90,12,127) [1,1]MSG_LFT(0X90,15,127) [1,1]MSG_RGT(0X90,18,127)"
"<RS1_RLS>[1,1]MSG_USB(0X80,12,127) [1,1]MSG_LFT(0X80,15,127) [1,1]MSG_RGT(0X80,18,127)"
"<RS1_RLL>[1,1]MSG_USB(0XA0,12,127) [1,1]MSG_LFT(0XA0,15,127) [1,1]MSG_RGT(0XA0,18,127)"
"<RS2_LBL>RGT SW2"
"<RS2_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS2_PRS>[1,1]MSG_USB(0X90,13,127) [1,1]MSG_LFT(0X90,16,127) [1,1]MSG_RGT(0X90,19,127)"
"<RS2_RLS>[1,1]MSG_USB(0X80,13,127) [1,1]MSG_LFT(0X80,16,127) [1,1]MSG_RGT(0X80,19,127)"
"<RS2_RLL>[1,1]MSG_USB(0XA0,13,127) [1,1]MSG_LFT(0XA0,16,127) [1,1]MSG_RGT(0XA0,19,127)"

"<PRESET>"
"<PST_LBL>VERIFY8"
"<PST_DSC>Verify that all switches and MIDI ports work, by sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages to all MIDI ports for each switch that is PRESSED, RELEASED or TIMED OUT. Also sends a MIDI system real time clock message at 120 BPM."
"<PST_CLK>[1,1]MSG_USB(0xF8)"
"<PST_ENT>[1,1]SET_BPM(120)"
"<PST_EXT>[1,1]SET_BPM(120)"
"<LS0_LBL>LFT SW0"
"<LS0_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS0_PRS>[1,1]MSG_USB(0X90,1,127) [1,1]MSG_LFT(0X90,4,127) [1,1]MSG_RGT(0X90,7,127)"
"<LS0_RLS>[1,1]MSG_USB(0X80,1,127) [1,1]MSG_LFT(0X80,4,127) [1,1]MSG_RGT(0X80,7,127)"
"<LS0_RLL>[1,1]MSG_USB(0XA0,1,127) [1,1]MSG_LFT(0XA0,4,127) [1,1]MSG_RGT(0XA0,7,127)"
"<LS1_LBL>LFT SW1"
"<LS1_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS1_PRS>[1,1]MSG_USB(0X90,2,127) [1,1]MSG_LFT(0X90,5,127) [1,1]MSG_RGT(0X90,8,127)"
"<LS1_RLS>[1,1]MSG_USB(0X80,2,127) [1,1]MSG_LFT(0X80,5,127) [1,1]MSG_RGT(0X80,8,127)"
"<LS1_RLL>[1,1]MSG_USB(0XA0,2,127) [1,1]MSG_LFT(0XA0,5,127) [1,1]MSG_RGT(0XA0,8,127)"
"<LS2_LBL>LFT SW2"
"<LS2_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS2_PRS>[1,1]MSG_USB(0X90,3,127) [1,1]MSG_LFT(0X90,6,127) [1,1]MSG_RGT(0X90,9,127)"
"<LS2_RLS>[1,1]MSG_USB(0X80,3,127) [1,1]MSG_LFT(0X80,6,127) [1,1]MSG_RGT(0X80,9,127)"
"<LS2_RLL>[1,1]MSG_USB(0XA0,3,127) [1,1]MSG_LFT(0XA0,6,127) [1,1]MSG_RGT(0XA0,9,127)"
"<RS0_LBL>RGT SW0"
"<RS0_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS0_PRS>[1,1]MSG_USB(0X90,11,127) [1,1]MSG_LFT(0X90,14,127) [1,1]MSG_RGT(0X90,17,127)"
"<RS0_RLS>[1,1]MSG_USB(0X80,11,127) [1,1]MSG_LFT(0X80,14,127) [1,1]MSG_RGT(0X80,17,127)"
"<RS0_RLL>[1,1]MSG_USB(0XA0,11,127) [1,1]MSG_LFT(0XA0,14,127) [1,1]MSG_RGT(0XA0,17,127)"
"<RS1_LBL>RGT SW1"
"<RS1_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS1_PRS>[1,1]MSG_USB(0X90,12,127) [1,1]MSG_LFT(0X90,15,127) [1,1]MSG_RGT(0X90,18,127)"
"<RS1_RLS>[1,1]MSG_USB(0X80,12,127) [1,1]MSG_LFT(0X80,15,127) [1,1]MSG_RGT(0X80,18,127)"
"<RS1_RLL>[1,1]MSG_USB(0XA0,12,127) [1,1]MSG_LFT(0XA0,15,127) [1,1]MSG_RGT(0XA0,18,127)"
"<RS2_LBL>RGT SW2"
"<RS2_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS2_PRS>[1,1]MSG_USB(0X90,13,127) [1,1]MSG_LFT(0X90,16,127) [1,1]MSG_RGT(0X90,19,127)"
"<RS2_RLS>[1,1]MSG_USB(0X80,13,127) [1,1]MSG_LFT(0X80,16,127) [1,1]MSG_RGT(0X80,19,127)"
"<RS2_RLL>[1,1]MSG_USB(0XA0,13,127) [1,1]MSG_LFT(0XA0,16,127) [1,1]MSG_RGT(0XA0,19,127)"


"<PRESET>"
"<PST_LBL>VERIFY9"
"<PST_DSC>Verify that all switches and MIDI ports work, by sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages to all MIDI ports for each switch that is PRESSED, RELEASED or TIMED OUT. Also sends a MIDI system real time clock message at 120 BPM."
"<PST_CLK>[1,1]MSG_USB(0xF8)"
"<PST_ENT>[1,1]SET_BPM(120)"
"<PST_EXT>[1,1]SET_BPM(120)"
"<LS0_LBL>LFT SW0"
"<LS0_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS0_PRS>[1,1]MSG_USB(0X90,1,127) [1,1]MSG_LFT(0X90,4,127) [1,1]MSG_RGT(0X90,7,127)"
"<LS0_RLS>[1,1]MSG_USB(0X80,1,127) [1,1]MSG_LFT(0X80,4,127) [1,1]MSG_RGT(0X80,7,127)"
"<LS0_RLL>[1,1]MSG_USB(0XA0,1,127) [1,1]MSG_LFT(0XA0,4,127) [1,1]MSG_RGT(0XA0,7,127)"
"<LS1_LBL>LFT SW1"
"<LS1_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS1_PRS>[1,1]MSG_USB(0X90,2,127) [1,1]MSG_LFT(0X90,5,127) [1,1]MSG_RGT(0X90,8,127)"
"<LS1_RLS>[1,1]MSG_USB(0X80,2,127) [1,1]MSG_LFT(0X80,5,127) [1,1]MSG_RGT(0X80,8,127)"
"<LS1_RLL>[1,1]MSG_USB(0XA0,2,127) [1,1]MSG_LFT(0XA0,5,127) [1,1]MSG_RGT(0XA0,8,127)"
"<LS2_LBL>LFT SW2"
"<LS2_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS2_PRS>[1,1]MSG_USB(0X90,3,127) [1,1]MSG_LFT(0X90,6,127) [1,1]MSG_RGT(0X90,9,127)"
"<LS2_RLS>[1,1]MSG_USB(0X80,3,127) [1,1]MSG_LFT(0X80,6,127) [1,1]MSG_RGT(0X80,9,127)"
"<LS2_RLL>[1,1]MSG_USB(0XA0,3,127) [1,1]MSG_LFT(0XA0,6,127) [1,1]MSG_RGT(0XA0,9,127)"
"<RS0_LBL>RGT SW0"
"<RS0_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS0_PRS>[1,1]MSG_USB(0X90,11,127) [1,1]MSG_LFT(0X90,14,127) [1,1]MSG_RGT(0X90,17,127)"
"<RS0_RLS>[1,1]MSG_USB(0X80,11,127) [1,1]MSG_LFT(0X80,14,127) [1,1]MSG_RGT(0X80,17,127)"
"<RS0_RLL>[1,1]MSG_USB(0XA0,11,127) [1,1]MSG_LFT(0XA0,14,127) [1,1]MSG_RGT(0XA0,17,127)"
"<RS1_LBL>RGT SW1"
"<RS1_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS1_PRS>[1,1]MSG_USB(0X90,12,127) [1,1]MSG_LFT(0X90,15,127) [1,1]MSG_RGT(0X90,18,127)"
"<RS1_RLS>[1,1]MSG_USB(0X80,12,127) [1,1]MSG_LFT(0X80,15,127) [1,1]MSG_RGT(0X80,18,127)"
"<RS1_RLL>[1,1]MSG_USB(0XA0,12,127) [1,1]MSG_LFT(0XA0,15,127) [1,1]MSG_RGT(0XA0,18,127)"
"<RS2_LBL>RGT SW2"
"<RS2_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS2_PRS>[1,1]MSG_USB(0X90,13,127) [1,1]MSG_LFT(0X90,16,127) [1,1]MSG_RGT(0X90,19,127)"
"<RS2_RLS>[1,1]MSG_USB(0X80,13,127) [1,1]MSG_LFT(0X80,16,127) [1,1]MSG_RGT(0X80,19,127)"
"<RS2_RLL>[1,1]MSG_USB(0XA0,13,127) [1,1]MSG_LFT(0XA0,16,127) [1,1]MSG_RGT(0XA0,19,127)"


"<PRESET>"
"<PST_LBL>VERIFY10"
"<PST_DSC>Verify that all switches and MIDI ports work, by sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages to all MIDI ports for each switch that is PRESSED, RELEASED or TIMED OUT. Also sends a MIDI system real time clock message at 120 BPM."
"<PST_CLK>[1,1]MSG_USB(0xF8)"
"<PST_ENT>[1,1]SET_BPM(120)"
"<PST_EXT>[1,1]SET_BPM(120)"
"<LS0_LBL>LFT SW0"
"<LS0_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS0_PRS>[1,1]MSG_USB(0X90,1,127) [1,1]MSG_LFT(0X90,4,127) [1,1]MSG_RGT(0X90,7,127)"
"<LS0_RLS>[1,1]MSG_USB(0X80,1,127) [1,1]MSG_LFT(0X80,4,127) [1,1]MSG_RGT(0X80,7,127)"
"<LS0_RLL>[1,1]MSG_USB(0XA0,1,127) [1,1]MSG_LFT(0XA0,4,127) [1,1]MSG_RGT(0XA0,7,127)"
"<LS1_LBL>LFT SW1"
"<LS1_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS1_PRS>[1,1]MSG_USB(0X90,2,127) [1,1]MSG_LFT(0X90,5,127) [1,1]MSG_RGT(0X90,8,127)"
"<LS1_RLS>[1,1]MSG_USB(0X80,2,127) [1,1]MSG_LFT(0X80,5,127) [1,1]MSG_RGT(0X80,8,127)"
"<LS1_RLL>[1,1]MSG_USB(0XA0,2,127) [1,1]MSG_LFT(0XA0,5,127) [1,1]MSG_RGT(0XA0,8,127)"
"<LS2_LBL>LFT SW2"
"<LS2_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS2_PRS>[1,1]MSG_USB(0X90,3,127) [1,1]MSG_LFT(0X90,6,127) [1,1]MSG_RGT(0X90,9,127)"
"<LS2_RLS>[1,1]MSG_USB(0X80,3,127) [1,1]MSG_LFT(0X80,6,127) [1,1]MSG_RGT(0X80,9,127)"
"<LS2_RLL>[1,1]MSG_USB(0XA0,3,127) [1,1]MSG_LFT(0XA0,6,127) [1,1]MSG_RGT(0XA0,9,127)"
"<RS0_LBL>RGT SW0"
"<RS0_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS0_PRS>[1,1]MSG_USB(0X90,11,127) [1,1]MSG_LFT(0X90,14,127) [1,1]MSG_RGT(0X90,17,127)"
"<RS0_RLS>[1,1]MSG_USB(0X80,11,127) [1,1]MSG_LFT(0X80,14,127) [1,1]MSG_RGT(0X80,17,127)"
"<RS0_RLL>[1,1]MSG_USB(0XA0,11,127) [1,1]MSG_LFT(0XA0,14,127) [1,1]MSG_RGT(0XA0,17,127)"
"<RS1_LBL>RGT SW1"
"<RS1_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS1_PRS>[1,1]MSG_USB(0X90,12,127) [1,1]MSG_LFT(0X90,15,127) [1,1]MSG_RGT(0X90,18,127)"
"<RS1_RLS>[1,1]MSG_USB(0X80,12,127) [1,1]MSG_LFT(0X80,15,127) [1,1]MSG_RGT(0X80,18,127)"
"<RS1_RLL>[1,1]MSG_USB(0XA0,12,127) [1,1]MSG_LFT(0XA0,15,127) [1,1]MSG_RGT(0XA0,18,127)"
"<RS2_LBL>RGT SW2"
"<RS2_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS2_PRS>[1,1]MSG_USB(0X90,13,127) [1,1]MSG_LFT(0X90,16,127) [1,1]MSG_RGT(0X90,19,127)"
"<RS2_RLS>[1,1]MSG_USB(0X80,13,127) [1,1]MSG_LFT(0X80,16,127) [1,1]MSG_RGT(0X80,19,127)"
"<RS2_RLL>[1,1]MSG_USB(0XA0,13,127) [1,1]MSG_LFT(0XA0,16,127) [1,1]MSG_RGT(0XA0,19,127)"


"<PRESET>"
"<PST_LBL>VERIFY11"
"<PST_DSC>Verify that all switches and MIDI ports work, by sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages to all MIDI ports for each switch that is PRESSED, RELEASED or TIMED OUT. Also sends a MIDI system real time clock message at 120 BPM."
"<PST_CLK>[1,1]MSG_USB(0xF8)"
"<PST_ENT>[1,1]SET_BPM(120)"
"<PST_EXT>[1,1]SET_BPM(120)"
"<LS0_LBL>LFT SW0"
"<LS0_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS0_PRS>[1,1]MSG_USB(0X90,1,127) [1,1]MSG_LFT(0X90,4,127) [1,1]MSG_RGT(0X90,7,127)"
"<LS0_RLS>[1,1]MSG_USB(0X80,1,127) [1,1]MSG_LFT(0X80,4,127) [1,1]MSG_RGT(0X80,7,127)"
"<LS0_RLL>[1,1]MSG_USB(0XA0,1,127) [1,1]MSG_LFT(0XA0,4,127) [1,1]MSG_RGT(0XA0,7,127)"
"<LS1_LBL>LFT SW1"
"<LS1_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS1_PRS>[1,1]MSG_USB(0X90,2,127) [1,1]MSG_LFT(0X90,5,127) [1,1]MSG_RGT(0X90,8,127)"
"<LS1_RLS>[1,1]MSG_USB(0X80,2,127) [1,1]MSG_LFT(0X80,5,127) [1,1]MSG_RGT(0X80,8,127)"
"<LS1_RLL>[1,1]MSG_USB(0XA0,2,127) [1,1]MSG_LFT(0XA0,5,127) [1,1]MSG_RGT(0XA0,8,127)"
"<LS2_LBL>LFT SW2"
"<LS2_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS2_PRS>[1,1]MSG_USB(0X90,3,127) [1,1]MSG_LFT(0X90,6,127) [1,1]MSG_RGT(0X90,9,127)"
"<LS2_RLS>[1,1]MSG_USB(0X80,3,127) [1,1]MSG_LFT(0X80,6,127) [1,1]MSG_RGT(0X80,9,127)"
"<LS2_RLL>[1,1]MSG_USB(0XA0,3,127) [1,1]MSG_LFT(0XA0,6,127) [1,1]MSG_RGT(0XA0,9,127)"
"<RS0_LBL>RGT SW0"
"<RS0_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS0_PRS>[1,1]MSG_USB(0X90,11,127) [1,1]MSG_LFT(0X90,14,127) [1,1]MSG_RGT(0X90,17,127)"
"<RS0_RLS>[1,1]MSG_USB(0X80,11,127) [1,1]MSG_LFT(0X80,14,127) [1,1]MSG_RGT(0X80,17,127)"
"<RS0_RLL>[1,1]MSG_USB(0XA0,11,127) [1,1]MSG_LFT(0XA0,14,127) [1,1]MSG_RGT(0XA0,17,127)"
"<RS1_LBL>RGT SW1"
"<RS1_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS1_PRS>[1,1]MSG_USB(0X90,12,127) [1,1]MSG_LFT(0X90,15,127) [1,1]MSG_RGT(0X90,18,127)"
"<RS1_RLS>[1,1]MSG_USB(0X80,12,127) [1,1]MSG_LFT(0X80,15,127) [1,1]MSG_RGT(0X80,18,127)"
"<RS1_RLL>[1,1]MSG_USB(0XA0,12,127) [1,1]MSG_LFT(0XA0,15,127) [1,1]MSG_RGT(0XA0,18,127)"
"<RS2_LBL>RGT SW2"
"<RS2_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS2_PRS>[1,1]MSG_USB(0X90,13,127) [1,1]MSG_LFT(0X90,16,127) [1,1]MSG_RGT(0X90,19,127)"
"<RS2_RLS>[1,1]MSG_USB(0X80,13,127) [1,1]MSG_LFT(0X80,16,127) [1,1]MSG_RGT(0X80,19,127)"
"<RS2_RLL>[1,1]MSG_USB(0XA0,13,127) [1,1]MSG_LFT(0XA0,16,127) [1,1]MSG_RGT(0XA0,19,127)"


"<PRESET>"
"<PST_LBL>VERIFY12"
"<PST_DSC>Verify that all switches and MIDI ports work, by sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages to all MIDI ports for each switch that is PRESSED, RELEASED or TIMED OUT. Also sends a MIDI system real time clock message at 120 BPM."
"<PST_CLK>[1,1]MSG_USB(0xF8)"
"<PST_ENT>[1,1]SET_BPM(120)"
"<PST_EXT>[1,1]SET_BPM(120)"
"<LS0_LBL>LFT SW0"
"<LS0_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS0_PRS>[1,1]MSG_USB(0X90,1,127) [1,1]MSG_LFT(0X90,4,127) [1,1]MSG_RGT(0X90,7,127)"
"<LS0_RLS>[1,1]MSG_USB(0X80,1,127) [1,1]MSG_LFT(0X80,4,127) [1,1]MSG_RGT(0X80,7,127)"
"<LS0_RLL>[1,1]MSG_USB(0XA0,1,127) [1,1]MSG_LFT(0XA0,4,127) [1,1]MSG_RGT(0XA0,7,127)"
"<LS1_LBL>LFT SW1"
"<LS1_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS1_PRS>[1,1]MSG_USB(0X90,2,127) [1,1]MSG_LFT(0X90,5,127) [1,1]MSG_RGT(0X90,8,127)"
"<LS1_RLS>[1,1]MSG_USB(0X80,2,127) [1,1]MSG_LFT(0X80,5,127) [1,1]MSG_RGT(0X80,8,127)"
"<LS1_RLL>[1,1]MSG_USB(0XA0,2,127) [1,1]MSG_LFT(0XA0,5,127) [1,1]MSG_RGT(0XA0,8,127)"
"<LS2_LBL>LFT SW2"
"<LS2_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS2_PRS>[1,1]MSG_USB(0X90,3,127) [1,1]MSG_LFT(0X90,6,127) [1,1]MSG_RGT(0X90,9,127)"
"<LS2_RLS>[1,1]MSG_USB(0X80,3,127) [1,1]MSG_LFT(0X80,6,127) [1,1]MSG_RGT(0X80,9,127)"
"<LS2_RLL>[1,1]MSG_USB(0XA0,3,127) [1,1]MSG_LFT(0XA0,6,127) [1,1]MSG_RGT(0XA0,9,127)"
"<RS0_LBL>RGT SW0"
"<RS0_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS0_PRS>[1,1]MSG_USB(0X90,11,127) [1,1]MSG_LFT(0X90,14,127) [1,1]MSG_RGT(0X90,17,127)"
"<RS0_RLS>[1,1]MSG_USB(0X80,11,127) [1,1]MSG_LFT(0X80,14,127) [1,1]MSG_RGT(0X80,17,127)"
"<RS0_RLL>[1,1]MSG_USB(0XA0,11,127) [1,1]MSG_LFT(0XA0,14,127) [1,1]MSG_RGT(0XA0,17,127)"
"<RS1_LBL>RGT SW1"
"<RS1_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS1_PRS>[1,1]MSG_USB(0X90,12,127) [1,1]MSG_LFT(0X90,15,127) [1,1]MSG_RGT(0X90,18,127)"
"<RS1_RLS>[1,1]MSG_USB(0X80,12,127) [1,1]MSG_LFT(0X80,15,127) [1,1]MSG_RGT(0X80,18,127)"
"<RS1_RLL>[1,1]MSG_USB(0XA0,12,127) [1,1]MSG_LFT(0XA0,15,127) [1,1]MSG_RGT(0XA0,18,127)"
"<RS2_LBL>RGT SW2"
"<RS2_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS2_PRS>[1,1]MSG_USB(0X90,13,127) [1,1]MSG_LFT(0X90,16,127) [1,1]MSG_RGT(0X90,19,127)"
"<RS2_RLS>[1,1]MSG_USB(0X80,13,127) [1,1]MSG_LFT(0X80,16,127) [1,1]MSG_RGT(0X80,19,127)"
"<RS2_RLL>[1,1]MSG_USB(0XA0,13,127) [1,1]MSG_LFT(0XA0,16,127) [1,1]MSG_RGT(0XA0,19,127)"


"<PRESET>"
"<PST_LBL>VERIFY13"
"<PST_DSC>Verify that all switches and MIDI ports work, by sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages to all MIDI ports for each switch that is PRESSED, RELEASED or TIMED OUT. Also sends a MIDI system real time clock message at 120 BPM."
"<PST_CLK>[1,1]MSG_USB(0xF8)"
"<PST_ENT>[1,1]SET_BPM(120)"
"<PST_EXT>[1,1]SET_BPM(120)"
"<LS0_LBL>LFT SW0"
"<LS0_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS0_PRS>[1,1]MSG_USB(0X90,1,127) [1,1]MSG_LFT(0X90,4,127) [1,1]MSG_RGT(0X90,7,127)"
"<LS0_RLS>[1,1]MSG_USB(0X80,1,127) [1,1]MSG_LFT(0X80,4,127) [1,1]MSG_RGT(0X80,7,127)"
"<LS0_RLL>[1,1]MSG_USB(0XA0,1,127) [1,1]MSG_LFT(0XA0,4,127) [1,1]MSG_RGT(0XA0,7,127)"
"<LS1_LBL>LFT SW1"
"<LS1_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS1_PRS>[1,1]MSG_USB(0X90,2,127) [1,1]MSG_LFT(0X90,5,127) [1,1]MSG_RGT(0X90,8,127)"
"<LS1_RLS>[1,1]MSG_USB(0X80,2,127) [1,1]MSG_LFT(0X80,5,127) [1,1]MSG_RGT(0X80,8,127)"
"<LS1_RLL>[1,1]MSG_USB(0XA0,2,127) [1,1]MSG_LFT(0XA0,5,127) [1,1]MSG_RGT(0XA0,8,127)"
"<LS2_LBL>LFT SW2"
"<LS2_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS2_PRS>[1,1]MSG_USB(0X90,3,127) [1,1]MSG_LFT(0X90,6,127) [1,1]MSG_RGT(0X90,9,127)"
"<LS2_RLS>[1,1]MSG_USB(0X80,3,127) [1,1]MSG_LFT(0X80,6,127) [1,1]MSG_RGT(0X80,9,127)"
"<LS2_RLL>[1,1]MSG_USB(0XA0,3,127) [1,1]MSG_LFT(0XA0,6,127) [1,1]MSG_RGT(0XA0,9,127)"
"<RS0_LBL>RGT SW0"
"<RS0_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS0_PRS>[1,1]MSG_USB(0X90,11,127) [1,1]MSG_LFT(0X90,14,127) [1,1]MSG_RGT(0X90,17,127)"
"<RS0_RLS>[1,1]MSG_USB(0X80,11,127) [1,1]MSG_LFT(0X80,14,127) [1,1]MSG_RGT(0X80,17,127)"
"<RS0_RLL>[1,1]MSG_USB(0XA0,11,127) [1,1]MSG_LFT(0XA0,14,127) [1,1]MSG_RGT(0XA0,17,127)"
"<RS1_LBL>RGT SW1"
"<RS1_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS1_PRS>[1,1]MSG_USB(0X90,12,127) [1,1]MSG_LFT(0X90,15,127) [1,1]MSG_RGT(0X90,18,127)"
"<RS1_RLS>[1,1]MSG_USB(0X80,12,127) [1,1]MSG_LFT(0X80,15,127) [1,1]MSG_RGT(0X80,18,127)"
"<RS1_RLL>[1,1]MSG_USB(0XA0,12,127) [1,1]MSG_LFT(0XA0,15,127) [1,1]MSG_RGT(0XA0,18,127)"
"<RS2_LBL>RGT SW2"
"<RS2_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS2_PRS>[1,1]MSG_USB(0X90,13,127) [1,1]MSG_LFT(0X90,16,127) [1,1]MSG_RGT(0X90,19,127)"
"<RS2_RLS>[1,1]MSG_USB(0X80,13,127) [1,1]MSG_LFT(0X80,16,127) [1,1]MSG_RGT(0X80,19,127)"
"<RS2_RLL>[1,1]MSG_USB(0XA0,13,127) [1,1]MSG_LFT(0XA0,16,127) [1,1]MSG_RGT(0XA0,19,127)"


"<PRESET>"
"<PST_LBL>VERIFY14"
"<PST_DSC>Verify that all switches and MIDI ports work, by sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages to all MIDI ports for each switch that is PRESSED, RELEASED or TIMED OUT. Also sends a MIDI system real time clock message at 120 BPM."
"<PST_CLK>[1,1]MSG_USB(0xF8)"
"<PST_ENT>[1,1]SET_BPM(120)"
"<PST_EXT>[1,1]SET_BPM(120)"
"<LS0_LBL>LFT SW0"
"<LS0_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS0_PRS>[1,1]MSG_USB(0X90,1,127) [1,1]MSG_LFT(0X90,4,127) [1,1]MSG_RGT(0X90,7,127)"
"<LS0_RLS>[1,1]MSG_USB(0X80,1,127) [1,1]MSG_LFT(0X80,4,127) [1,1]MSG_RGT(0X80,7,127)"
"<LS0_RLL>[1,1]MSG_USB(0XA0,1,127) [1,1]MSG_LFT(0XA0,4,127) [1,1]MSG_RGT(0XA0,7,127)"
"<LS1_LBL>LFT SW1"
"<LS1_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS1_PRS>[1,1]MSG_USB(0X90,2,127) [1,1]MSG_LFT(0X90,5,127) [1,1]MSG_RGT(0X90,8,127)"
"<LS1_RLS>[1,1]MSG_USB(0X80,2,127) [1,1]MSG_LFT(0X80,5,127) [1,1]MSG_RGT(0X80,8,127)"
"<LS1_RLL>[1,1]MSG_USB(0XA0,2,127) [1,1]MSG_LFT(0XA0,5,127) [1,1]MSG_RGT(0XA0,8,127)"
"<LS2_LBL>LFT SW2"
"<LS2_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS2_PRS>[1,1]MSG_USB(0X90,3,127) [1,1]MSG_LFT(0X90,6,127) [1,1]MSG_RGT(0X90,9,127)"
"<LS2_RLS>[1,1]MSG_USB(0X80,3,127) [1,1]MSG_LFT(0X80,6,127) [1,1]MSG_RGT(0X80,9,127)"
"<LS2_RLL>[1,1]MSG_USB(0XA0,3,127) [1,1]MSG_LFT(0XA0,6,127) [1,1]MSG_RGT(0XA0,9,127)"
"<RS0_LBL>RGT SW0"
"<RS0_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS0_PRS>[1,1]MSG_USB(0X90,11,127) [1,1]MSG_LFT(0X90,14,127) [1,1]MSG_RGT(0X90,17,127)"
"<RS0_RLS>[1,1]MSG_USB(0X80,11,127) [1,1]MSG_LFT(0X80,14,127) [1,1]MSG_RGT(0X80,17,127)"
"<RS0_RLL>[1,1]MSG_USB(0XA0,11,127) [1,1]MSG_LFT(0XA0,14,127) [1,1]MSG_RGT(0XA0,17,127)"
"<RS1_LBL>RGT SW1"
"<RS1_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS1_PRS>[1,1]MSG_USB(0X90,12,127) [1,1]MSG_LFT(0X90,15,127) [1,1]MSG_RGT(0X90,18,127)"
"<RS1_RLS>[1,1]MSG_USB(0X80,12,127) [1,1]MSG_LFT(0X80,15,127) [1,1]MSG_RGT(0X80,18,127)"
"<RS1_RLL>[1,1]MSG_USB(0XA0,12,127) [1,1]MSG_LFT(0XA0,15,127) [1,1]MSG_RGT(0XA0,18,127)"
"<RS2_LBL>RGT SW2"
"<RS2_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS2_PRS>[1,1]MSG_USB(0X90,13,127) [1,1]MSG_LFT(0X90,16,127) [1,1]MSG_RGT(0X90,19,127)"
"<RS2_RLS>[1,1]MSG_USB(0X80,13,127) [1,1]MSG_LFT(0X80,16,127) [1,1]MSG_RGT(0X80,19,127)"
"<RS2_RLL>[1,1]MSG_USB(0XA0,13,127) [1,1]MSG_LFT(0XA0,16,127) [1,1]MSG_RGT(0XA0,19,127)"


"<PRESET>"
"<PST_LBL>VERIFY15"
"<PST_DSC>Verify that all switches and MIDI ports work, by sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages to all MIDI ports for each switch that is PRESSED, RELEASED or TIMED OUT. Also sends a MIDI system real time clock message at 120 BPM."
"<PST_CLK>[1,1]MSG_USB(0xF8)"
"<PST_ENT>[1,1]SET_BPM(120)"
"<PST_EXT>[1,1]SET_BPM(120)"
"<LS0_LBL>LFT SW0"
"<LS0_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS0_PRS>[1,1]MSG_USB(0X90,1,127) [1,1]MSG_LFT(0X90,4,127) [1,1]MSG_RGT(0X90,7,127)"
"<LS0_RLS>[1,1]MSG_USB(0X80,1,127) [1,1]MSG_LFT(0X80,4,127) [1,1]MSG_RGT(0X80,7,127)"
"<LS0_RLL>[1,1]MSG_USB(0XA0,1,127) [1,1]MSG_LFT(0XA0,4,127) [1,1]MSG_RGT(0XA0,7,127)"
"<LS1_LBL>LFT SW1"
"<LS1_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS1_PRS>[1,1]MSG_USB(0X90,2,127) [1,1]MSG_LFT(0X90,5,127) [1,1]MSG_RGT(0X90,8,127)"
"<LS1_RLS>[1,1]MSG_USB(0X80,2,127) [1,1]MSG_LFT(0X80,5,127) [1,1]MSG_RGT(0X80,8,127)"
"<LS1_RLL>[1,1]MSG_USB(0XA0,2,127) [1,1]MSG_LFT(0XA0,5,127) [1,1]MSG_RGT(0XA0,8,127)"
"<LS2_LBL>LFT SW2"
"<LS2_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS2_PRS>[1,1]MSG_USB(0X90,3,127) [1,1]MSG_LFT(0X90,6,127) [1,1]MSG_RGT(0X90,9,127)"
"<LS2_RLS>[1,1]MSG_USB(0X80,3,127) [1,1]MSG_LFT(0X80,6,127) [1,1]MSG_RGT(0X80,9,127)"
"<LS2_RLL>[1,1]MSG_USB(0XA0,3,127) [1,1]MSG_LFT(0XA0,6,127) [1,1]MSG_RGT(0XA0,9,127)"
"<RS0_LBL>RGT SW0"
"<RS0_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS0_PRS>[1,1]MSG_USB(0X90,11,127) [1,1]MSG_LFT(0X90,14,127) [1,1]MSG_RGT(0X90,17,127)"
"<RS0_RLS>[1,1]MSG_USB(0X80,11,127) [1,1]MSG_LFT(0X80,14,127) [1,1]MSG_RGT(0X80,17,127)"
"<RS0_RLL>[1,1]MSG_USB(0XA0,11,127) [1,1]MSG_LFT(0XA0,14,127) [1,1]MSG_RGT(0XA0,17,127)"
"<RS1_LBL>RGT SW1"
"<RS1_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS1_PRS>[1,1]MSG_USB(0X90,12,127) [1,1]MSG_LFT(0X90,15,127) [1,1]MSG_RGT(0X90,18,127)"
"<RS1_RLS>[1,1]MSG_USB(0X80,12,127) [1,1]MSG_LFT(0X80,15,127) [1,1]MSG_RGT(0X80,18,127)"
"<RS1_RLL>[1,1]MSG_USB(0XA0,12,127) [1,1]MSG_LFT(0XA0,15,127) [1,1]MSG_RGT(0XA0,18,127)"
"<RS2_LBL>RGT SW2"
"<RS2_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS2_PRS>[1,1]MSG_USB(0X90,13,127) [1,1]MSG_LFT(0X90,16,127) [1,1]MSG_RGT(0X90,19,127)"
"<RS2_RLS>[1,1]MSG_USB(0X80,13,127) [1,1]MSG_LFT(0X80,16,127) [1,1]MSG_RGT(0X80,19,127)"
"<RS2_RLL>[1,1]MSG_USB(0XA0,13,127) [1,1]MSG_LFT(0XA0,16,127) [1,1]MSG_RGT(0XA0,19,127)"


"<PRESET>"
"<PST_LBL>VERIFY16"
"<PST_DSC>Verify that all switches and MIDI ports work, by sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages to all MIDI ports for each switch that is PRESSED, RELEASED or TIMED OUT. Also sends a MIDI system real time clock message at 120 BPM."
"<PST_CLK>[1,1]MSG_USB(0xF8)"
"<PST_ENT>[1,1]SET_BPM(120)"
"<PST_EXT>[1,1]SET_BPM(120)"
"<LS0_LBL>LFT SW0"
"<LS0_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS0_PRS>[1,1]MSG_USB(0X90,1,127) [1,1]MSG_LFT(0X90,4,127) [1,1]MSG_RGT(0X90,7,127)"
"<LS0_RLS>[1,1]MSG_USB(0X80,1,127) [1,1]MSG_LFT(0X80,4,127) [1,1]MSG_RGT(0X80,7,127)"
"<LS0_RLL>[1,1]MSG_USB(0XA0,1,127) [1,1]MSG_LFT(0XA0,4,127) [1,1]MSG_RGT(0XA0,7,127)"
"<LS1_LBL>LFT SW1"
"<LS1_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS1_PRS>[1,1]MSG_USB(0X90,2,127) [1,1]MSG_LFT(0X90,5,127) [1,1]MSG_RGT(0X90,8,127)"
"<LS1_RLS>[1,1]MSG_USB(0X80,2,127) [1,1]MSG_LFT(0X80,5,127) [1,1]MSG_RGT(0X80,8,127)"
"<LS1_RLL>[1,1]MSG_USB(0XA0,2,127) [1,1]MSG_LFT(0XA0,5,127) [1,1]MSG_RGT(0XA0,8,127)"
"<LS2_LBL>LFT SW2"
"<LS2_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<LS2_PRS>[1,1]MSG_USB(0X90,3,127) [1,1]MSG_LFT(0X90,6,127) [1,1]MSG_RGT(0X90,9,127)"
"<LS2_RLS>[1,1]MSG_USB(0X80,3,127) [1,1]MSG_LFT(0X80,6,127) [1,1]MSG_RGT(0X80,9,127)"
"<LS2_RLL>[1,1]MSG_USB(0XA0,3,127) [1,1]MSG_LFT(0XA0,6,127) [1,1]MSG_RGT(0XA0,9,127)"
"<RS0_LBL>RGT SW0"
"<RS0_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS0_PRS>[1,1]MSG_USB(0X90,11,127) [1,1]MSG_LFT(0X90,14,127) [1,1]MSG_RGT(0X90,17,127)"
"<RS0_RLS>[1,1]MSG_USB(0X80,11,127) [1,1]MSG_LFT(0X80,14,127) [1,1]MSG_RGT(0X80,17,127)"
"<RS0_RLL>[1,1]MSG_USB(0XA0,11,127) [1,1]MSG_LFT(0XA0,14,127) [1,1]MSG_RGT(0XA0,17,127)"
"<RS1_LBL>RGT SW1"
"<RS1_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS1_PRS>[1,1]MSG_USB(0X90,12,127) [1,1]MSG_LFT(0X90,15,127) [1,1]MSG_RGT(0X90,18,127)"
"<RS1_RLS>[1,1]MSG_USB(0X80,12,127) [1,1]MSG_LFT(0X80,15,127) [1,1]MSG_RGT(0X80,18,127)"
"<RS1_RLL>[1,1]MSG_USB(0XA0,12,127) [1,1]MSG_LFT(0XA0,15,127) [1,1]MSG_RGT(0XA0,18,127)"
"<RS2_LBL>RGT SW2"
"<RS2_INF>Verify the PRESSED, RELEASED and TIMED OUT actions are sending NOTE_ON, NOTE_OFF & AFTERTOUCH messages messages to all MIDI ports."
"<RS2_PRS>[1,1]MSG_USB(0X90,13,127) [1,1]MSG_LFT(0X90,16,127) [1,1]MSG_RGT(0X90,19,127)"
"<RS2_RLS>[1,1]MSG_USB(0X80,13,127) [1,1]MSG_LFT(0X80,16,127) [1,1]MSG_RGT(0X80,19,127)"
"<RS2_RLL>[1,1]MSG_USB(0XA0,13,127) [1,1]MSG_LFT(0XA0,16,127) [1,1]MSG_RGT(0XA0,19,127)"

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
"<LS1_LBL>"
"<LS1_INF>"
"<LS1_PRS>"
"<LS1_RLS>"
"<LS1_RLL>"
"<LS2_LBL>"
"<LS2_INF>"
"<LS2_PRS>"
"<LS2_RLS>"
"<LS2_RLL>"
"<RS0_LBL>"
"<RS0_INF>"
"<RS0_PRS>"
"<RS0_RLS>"
"<RS0_RLL>"
"<RS1_LBL>"
"<RS1_INF>"
"<RS1_PRS>"
"<RS1_RLS>"
"<RS1_RLL>"
"<RS2_LBL>"
"<RS2_INF>"
"<RS2_PRS>"
"<RS2_RLS>"
"<RS2_RLL>"

"\0";  //always end string with 0x00
