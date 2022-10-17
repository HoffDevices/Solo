/*
* screen.c
* Contains screen definitions
*/

/*
* Menu to select presets
*/
void S_MENU() {
    void setupMain() {  //run once
    }
    void updateMain() {  //run continuously
    }
    char s[10];
    sprintf(s, "MENU %d", (CURR_VIEW.page + 1));
    B_LABEL_STATIC.name = s;
    BUTTON mainButtons[] = {B_BCK_PRESET, B_LABEL_STATIC, B_FWD_PRESET, B_SELECT_1, B_SELECT_2, B_SELECT_3, B_SELECT_4, B_SELECT_5, B_SELECT_6, B_SELECT_7, B_SELECT_8};
    int mainLength = sizeof(mainButtons)/sizeof(BUTTON);
    selectButtons(mainButtons, 3, false, true);
    handleScreen(mainButtons, mainLength, setupMain, updateMain);
}


/*
* System "preset"
*/
void S_SYSTEM() {
    void setupMain() {  //run once
    }
    void updateMain() {  //run continuously
    }
    B_SYSTEM_FIRMWARE_SHOW.name = SYSTEM_VERSION;
    BUTTON mainButtons[] = {B_BCK_PRESET, B_LABEL_STATIC, B_SYSTEM_SYSEX_SEND, B_SYSTEM_SYSEX_RECV, B_SYSTEM_FACTORY_RESET, B_SYSTEM_FIRMWARE_SHOW};
    int mainLength = sizeof(mainButtons)/sizeof(BUTTON);
    if (sendInterCoreFlag(CORE_REQ_DISABLE_PRESET, CORE_ACK_DISABLE_PRESET, 10000) == false) showInfo("ERROR\nDISABLING PRESET",1000);  //disable clock timer - timeout of 10ms because we might have a lot of functions running in the midi clock
    handleScreen(mainButtons, mainLength, setupMain, updateMain);
}


/*
* Generic Preset screen
*/
void S_PRESET() {
    void setupMain() {  //run once
    }
    void updateMain() {  //run continuously
        //show BPM
        int bpm = (int64_t) (-60000000 / 24 / MIDI_CLOCK_TIMER_US);  //24 midi clocks per BPM
        char bpmText[4];  //3 digits + 0x00
        sprintf(bpmText, "%d", bpm);
        B_BPM_HELP.name = bpmText;
        drawButton(B_BPM_HELP, 3);  //max 3 digits for bpm
    }

    strcpy(SYSTEM_DEFAULT, SCREENS[CURR_SCREEN_INDEX].name);  //save preset screen name as default
    B_BPM_HELP.name = "---";  //prevent old pointer from displaying garbage

    if (strlen(SCREENS[CURR_SCREEN_INDEX].LS0.LBL) > 0) {
        B_LS0_HELP.active = true;
        B_LS0_HELP.name = SCREENS[CURR_SCREEN_INDEX].LS0.LBL;
    }
    else {
        B_LS0_HELP.active = false;
        B_LS0_HELP.name = " ";
    }

    if (strlen(SCREENS[CURR_SCREEN_INDEX].LS1.LBL) > 0) {
        B_LS1_HELP.active = true;
        B_LS1_HELP.name = SCREENS[CURR_SCREEN_INDEX].LS1.LBL;
    }
    else {
        B_LS1_HELP.active = false;
        B_LS1_HELP.name = " ";
    }

    if (strlen(SCREENS[CURR_SCREEN_INDEX].LS2.LBL) > 0) {
        B_LS2_HELP.active = true;
        B_LS2_HELP.name = SCREENS[CURR_SCREEN_INDEX].LS2.LBL;
    }
    else {
        B_LS2_HELP.active = false;
        B_LS2_HELP.name = " ";
    }

    if (strlen(SCREENS[CURR_SCREEN_INDEX].RS0.LBL) > 0) {
        B_RS0_HELP.active = true;
        B_RS0_HELP.name = SCREENS[CURR_SCREEN_INDEX].RS0.LBL;
    }
    else {
        B_RS0_HELP.active = false;
        B_RS0_HELP.name = " ";
    }

    if (strlen(SCREENS[CURR_SCREEN_INDEX].RS1.LBL) > 0) {
        B_RS1_HELP.active = true;
        B_RS1_HELP.name = SCREENS[CURR_SCREEN_INDEX].RS1.LBL;
    }
    else {
        B_RS1_HELP.active = false;
        B_RS1_HELP.name = " ";
    }

    if (strlen(SCREENS[CURR_SCREEN_INDEX].RS2.LBL) > 0) {
        B_RS2_HELP.active = true;
        B_RS2_HELP.name = SCREENS[CURR_SCREEN_INDEX].RS2.LBL;
    }
    else {
        B_RS2_HELP.active = false;
        B_RS2_HELP.name = " ";
    }

    //reset switch interval counters
    SWITCH_LS0.counter = 0;
    SWITCH_LS1.counter = 0;
    SWITCH_LS2.counter = 0;
    SWITCH_RS0.counter = 0;
    SWITCH_RS1.counter = 0;
    SWITCH_RS2.counter = 0;

    BUTTON mainButtons[] = {B_LABEL_MENU, B_BPM_MINUSTEN, B_BPM_MINUSONE, B_BPM_HELP, B_BPM_PLUSONE, B_BPM_PLUSTEN, B_LS2_HELP, B_RS2_HELP, B_LS1_HELP, B_RS1_HELP, B_LS0_HELP, B_RS0_HELP};  //main view buttons
    int mainLength = sizeof(mainButtons)/sizeof(BUTTON);
    if (sendInterCoreFlag(CORE_REQ_ENABLE_PRESET, CORE_ACK_ENABLE_PRESET, 10000) == false) showInfo("ERROR\nENABLING PRESET",1000);  //enable clock timer - timeout of 10ms because we might have a lot of functions in the preset init property
    handleScreen(mainButtons, mainLength, setupMain, updateMain);
    //if (sendInterCoreFlag(CORE_REQ_DISABLE_PRESET, CORE_ACK_DISABLE_PRESET, 10000) == false) showInfo("ERROR\nDISABLING PRESET",1000);  //disable clock timer - timeout of 10ms because we might have a lot of functions running in the midi clock
}

/*
* Initialise screens - fill empties with dashes
* Load system presets
* Help width: 30 char
*/
void loadSystemScreens() {
    uint8_t lenScreens = sizeof(SCREENS)/sizeof(SCREEN);
    for (uint8_t i=0; i<lenScreens; i++) {
        SCREENS[i].system = false;
        SCREENS[i].preset = false;
        SCREENS[i].name = "--------";
        SCREENS[i].show = NULL;
    }
    //default screens
    SCREENS[0] = (SCREEN) {
        .system = false,
        .preset = false,
        .name = "MENU",
        .show = S_MENU
    };
    SCREENS[1] = (SCREEN) {
        .system = false,
        .preset = true,
        .name = "SYSTEM",
        .show = S_SYSTEM
    };
}

void loadPresetScreens() {
    //find first empty screen slot
    uint8_t p = 0;
    while (strcmp(SCREENS[p].name, "--------") != 0) p++;
    p--;  //we pre-increment below with each new preset
    //parsing process
    bool readProperty = false;
    char textProperty[8];  //max is probably 8 (???_??? + 0x00)
    char parentProperty[8];  //max is probably 8 (???_??? + 0x00)
    uint32_t i=0;  //CONFIG_TEXT index
    uint8_t k=0;  //property/value string index
    while (CONFIG_TEXT[i] != 0x00) {

        if (CONFIG_TEXT[i] == '<') {
            readProperty = true;
            CONFIG_TEXT[i] = 0x00;  //start of new property implies termination of previous property string (including comment)
            k=0;
        }
        if (CONFIG_TEXT[i] == '>' && readProperty == true) {  //update property
            readProperty = false;
            textProperty[k] = 0x00;  //terminate string
            k=0;
            if      (strcmp(textProperty, "SYSTEM") == 0) strcpy(parentProperty, textProperty);  //read system properties - use strcpy because the pointer [i+1] would be at the end of the textProperty
            else if (strcmp(textProperty, "PRESET") == 0) {  //start new preset screen
                strcpy(parentProperty, textProperty);
                p++;
                MAX_SCREEN_INDEX = p;
                SCREENS[p].system = false;
                SCREENS[p].preset = true;
                SCREENS[p].show = S_PRESET;
            }
            else if (strcmp(parentProperty, "SYSTEM") == 0) {  //system properties
                if      (strcmp(textProperty, "VERSION") == 0) SYSTEM_VERSION = &CONFIG_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "DEFAULT") == 0) SYSTEM_DEFAULT = &CONFIG_TEXT[i+1];  //pointer to start of property
            }
            else if (strcmp(parentProperty, "PRESET") == 0) {  //preset properties
                if      (strcmp(textProperty, "PST_LBL") == 0) {
                    SCREENS[p].PST.LBL = &CONFIG_TEXT[i+1];  //pointer to start of property, which is the next char
                    SCREENS[p].name = &CONFIG_TEXT[i+1];  //pointer to start of property, which is the next char
                }
                else if (strcmp(textProperty, "PST_DSC") == 0) SCREENS[p].PST.DSC = &CONFIG_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "PST_CLK") == 0) SCREENS[p].PST.CLK = &CONFIG_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "PST_ENT") == 0) SCREENS[p].PST.ENT = &CONFIG_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "PST_EXT") == 0) SCREENS[p].PST.EXT = &CONFIG_TEXT[i+1];  //pointer to start of property

                else if (strcmp(textProperty, "LS0_LBL") == 0) SCREENS[p].LS0.LBL = &CONFIG_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "LS0_INF") == 0) SCREENS[p].LS0.INF = &CONFIG_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "LS0_PRS") == 0) SCREENS[p].LS0.PRS = &CONFIG_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "LS0_RLS") == 0) SCREENS[p].LS0.RLS = &CONFIG_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "LS0_RLL") == 0) SCREENS[p].LS0.RLL = &CONFIG_TEXT[i+1];  //pointer to start of property

                else if (strcmp(textProperty, "LS1_LBL") == 0) SCREENS[p].LS1.LBL = &CONFIG_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "LS1_INF") == 0) SCREENS[p].LS1.INF = &CONFIG_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "LS1_PRS") == 0) SCREENS[p].LS1.PRS = &CONFIG_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "LS1_RLS") == 0) SCREENS[p].LS1.RLS = &CONFIG_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "LS1_RLL") == 0) SCREENS[p].LS1.RLL = &CONFIG_TEXT[i+1];  //pointer to start of property

                else if (strcmp(textProperty, "LS2_LBL") == 0) SCREENS[p].LS2.LBL = &CONFIG_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "LS2_INF") == 0) SCREENS[p].LS2.INF = &CONFIG_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "LS2_PRS") == 0) SCREENS[p].LS2.PRS = &CONFIG_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "LS2_RLS") == 0) SCREENS[p].LS2.RLS = &CONFIG_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "LS2_RLL") == 0) SCREENS[p].LS2.RLL = &CONFIG_TEXT[i+1];  //pointer to start of property

                else if (strcmp(textProperty, "RS0_LBL") == 0) SCREENS[p].RS0.LBL = &CONFIG_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "RS0_INF") == 0) SCREENS[p].RS0.INF = &CONFIG_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "RS0_PRS") == 0) SCREENS[p].RS0.PRS = &CONFIG_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "RS0_RLS") == 0) SCREENS[p].RS0.RLS = &CONFIG_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "RS0_RLL") == 0) SCREENS[p].RS0.RLL = &CONFIG_TEXT[i+1];  //pointer to start of property

                else if (strcmp(textProperty, "RS1_LBL") == 0) SCREENS[p].RS1.LBL = &CONFIG_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "RS1_INF") == 0) SCREENS[p].RS1.INF = &CONFIG_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "RS1_PRS") == 0) SCREENS[p].RS1.PRS = &CONFIG_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "RS1_RLS") == 0) SCREENS[p].RS1.RLS = &CONFIG_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "RS1_RLL") == 0) SCREENS[p].RS1.RLL = &CONFIG_TEXT[i+1];  //pointer to start of property

                else if (strcmp(textProperty, "RS2_LBL") == 0) SCREENS[p].RS2.LBL = &CONFIG_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "RS2_INF") == 0) SCREENS[p].RS2.INF = &CONFIG_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "RS2_PRS") == 0) SCREENS[p].RS2.PRS = &CONFIG_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "RS2_RLS") == 0) SCREENS[p].RS2.RLS = &CONFIG_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "RS2_RLL") == 0) SCREENS[p].RS2.RLL = &CONFIG_TEXT[i+1];  //pointer to start of property
            }
        }
        if (CONFIG_TEXT[i]!='<' && CONFIG_TEXT[i]!='>' && CONFIG_TEXT[i]!=0x10 && CONFIG_TEXT[i]!=0x13 && CONFIG_TEXT[i]!=0x00) {  //ignore property markers and cr/lf and 0x00
            if (readProperty == true) {
                textProperty[k] = CONFIG_TEXT[i];
                k++;
            }
        }
        i++;
    }
}
