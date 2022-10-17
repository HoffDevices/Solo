/*
* button.c
* Contains touch screen button stuff
*/


/*
* Confirm system options
*/
void confirmButton(BUTTON B) {
    drawButton(B, 0);
    absolute_time_t waitTime = get_absolute_time();
    while (absolute_time_diff_us(waitTime, get_absolute_time()) < 3000000) {  //3 seconds
        getTouch();  //get touch status into global, only after screen draw done
        touchButton(B);
    }
    drawFilledRectangle(B.left, B.top, B.width+1, B.height+1, COLOR_BLACK);
}


/*
* Show forward button, top right
* Cycle forward through events : PRS -> SRT -> LNG -> MAIN
*/
void A_FWD_HELP() {
    if (CURR_VIEW.event == EVENT_INF) {
        CURR_VIEW.event = EVENT_PRS;
        CURR_VIEW.type = VIEW_HELP;
    }
    else if (CURR_VIEW.event == EVENT_PRS) {
        CURR_VIEW.event = EVENT_RLS;
        CURR_VIEW.type = VIEW_HELP;
    }
    else if (CURR_VIEW.event == EVENT_RLS) {
        CURR_VIEW.event = EVENT_RLL;
        CURR_VIEW.type = VIEW_HELP;
    }
    else if (CURR_VIEW.event == EVENT_DSC) {
        CURR_VIEW.event = EVENT_CLK;
        CURR_VIEW.type = VIEW_HELP;
    }
    else if (CURR_VIEW.event == EVENT_CLK) {
        CURR_VIEW.event = EVENT_ENT;
        CURR_VIEW.type = VIEW_HELP;
    }
    else if (CURR_VIEW.event == EVENT_ENT) {
        CURR_VIEW.event = EVENT_EXT;
        CURR_VIEW.type = VIEW_HELP;
    }
    else {
        CURR_VIEW.control = NULL;
        CURR_VIEW.event = EVENT_NONE;
        CURR_VIEW.type = VIEW_MAIN;
    }
    CURR_VIEW.draw = true;
    CURR_VIEW.startTime = get_absolute_time();
}
static BUTTON B_FWD_HELP = (BUTTON) {
    .left = LCD_WIDTH-1-38,
    .top = 0, 
    .width = 38, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_CYAN,
    .active = true,
    .frame = true,
    .font = &Font30x17,
    .name = ">",
    .action = A_FWD_HELP
};

/*
* Show back button, top left
* Cycle backward through events : PRS <- SRT <- LNG <- MAIN
*/
void A_BCK_HELP() {
    if (CURR_VIEW.event == EVENT_RLL) {
        CURR_VIEW.event = EVENT_RLS;
        CURR_VIEW.type = VIEW_HELP;
    }
    else if (CURR_VIEW.event == EVENT_RLS) {
        CURR_VIEW.event = EVENT_PRS;
        CURR_VIEW.type = VIEW_HELP;
    }
    else if (CURR_VIEW.event == EVENT_PRS) {
        CURR_VIEW.event = EVENT_INF;
        CURR_VIEW.type = VIEW_HELP;
    }
    else if (CURR_VIEW.event == EVENT_EXT) {
        CURR_VIEW.event = EVENT_ENT;
        CURR_VIEW.type = VIEW_HELP;
    }
    else if (CURR_VIEW.event == EVENT_ENT) {
        CURR_VIEW.event = EVENT_CLK;
        CURR_VIEW.type = VIEW_HELP;
    }
    else if (CURR_VIEW.event == EVENT_CLK) {
        CURR_VIEW.event = EVENT_DSC;
        CURR_VIEW.type = VIEW_HELP;
    }
    else {
        CURR_VIEW.control = NULL;
        CURR_VIEW.event = EVENT_NONE;
        CURR_VIEW.type = VIEW_MAIN;
    }
    CURR_VIEW.draw = true;
    CURR_VIEW.startTime = get_absolute_time();
}
static BUTTON B_BCK_HELP = (BUTTON) {
    .left = 0,
    .top = 0, 
    .width = 38, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_CYAN,
    .active = true,
    .frame = true,
    .font = &Font30x17,
    .name = "<",
    .action = A_BCK_HELP
};

/*
* Show forward button, top right
* Go to next menu page
*/
void A_FWD_PRESET() {
    if (CURR_VIEW.page < (MAX_SCREEN_INDEX / 8)) {  //need to adjust this for system screens
        CURR_VIEW.page++;
        setCurrScreenIndex("MENU");
    }
    else {
        setCurrScreenIndex(SYSTEM_DEFAULT);
    }
}
static BUTTON B_FWD_PRESET = (BUTTON) {
    .left = LCD_WIDTH-1-38,
    .top = 0, 
    .width = 38, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_WHITE,
    .active = true,
    .frame = true,
    .font = &Font30x17,
    .name = ">",
    .action = A_FWD_PRESET
};


/*
* Displays back button, top left
* Sets screen to last active preset
*/
void A_BCK_PRESET() {
    if (CURR_VIEW.page > 0) {
        CURR_VIEW.page--;
        setCurrScreenIndex("MENU");
    }
    else {
        setCurrScreenIndex(SYSTEM_DEFAULT);
    }
}
static BUTTON B_BCK_PRESET = (BUTTON) {
    .left = 0, 
    .top = 0, 
    .width = 38, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_WHITE,
    .active = true,
    .frame = true,
    .font = &Font30x17,
    .name = "<",
    .action = A_BCK_PRESET
};


/*
* Displays current screen's name, top middle
* Does nothing
*/
void A_LABEL_STATIC() {

}
static BUTTON B_LABEL_STATIC = (BUTTON) {
    .left = LCD_WIDTH/2-1-228/2, 
    .top = 0, 
    .width = 228, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_WHITE,
    .active = true,
    .frame = false,
    .font = &Font30x17,
    .name = "",
    .action = A_LABEL_STATIC
};

/*
* Displays current screen's name, top middle
* Sets screen to preset page 1
*/
void A_LABEL_MENU() {
    CURR_VIEW.page = 0;
    setCurrScreenIndex("MENU");
}
static BUTTON B_LABEL_MENU = (BUTTON) {
    .left = 0, 
    .top = 0, 
    .width = 320, 
    .height = 50, 
    .radius = 10, 
    .color = COLOR_WHITE,
    .active = true,
    .frame = false,
    .font = &Font68x39,
    .name = "",//empty string gets populated by screen name
    .action = A_LABEL_MENU
};

/*
* Menu select button
* Sets screen to name of button, so action = NULL
*/
static BUTTON B_SELECT_1 = (BUTTON) {
    .left = 0,
    .top = LCD_HEIGHT-1-38-8-38-8-38-8-38, 
    .width = 156, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_CYAN,
    .active = true,
    .frame = true,
    .font = &Font30x17,
    .name = "--------",
    .action = NULL  //NULL action results in setCurrScreenIndex(B.name) being called
};

/*
* Menu select button
* Sets screen to name of button, so action = NULL
*/
static BUTTON B_SELECT_2 = (BUTTON) {
    .left = 0,
    .top = LCD_HEIGHT-1-38-8-38-8-38, 
    .width = 156, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_CYAN,
    .active = true,
    .frame = true,
    .font = &Font30x17,
    .name = "--------",
    .action = NULL  //NULL action results in setCurrScreenIndex(B.name) being called
};

/*
* Menu select button
* Sets screen to name of button, so action = NULL
*/
static BUTTON B_SELECT_3 = (BUTTON) {
    .left = 0,
    .top = LCD_HEIGHT-1-38-8-38, 
    .width = 156, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_CYAN,
    .active = true,
    .frame = true,
    .font = &Font30x17,
    .name = "--------",
    .action = NULL  //NULL action results in setCurrScreenIndex(B.name) being called
};

/*
* Menu select button
* Sets screen to name of button, so action = NULL
*/
static BUTTON B_SELECT_4 = (BUTTON) {
    .left = 0,
    .top = LCD_HEIGHT-1-38, 
    .width = 156, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_CYAN,
    .active = true,
    .frame = true,
    .font = &Font30x17,
    .name = "--------",
    .action = NULL  //NULL action results in setCurrScreenIndex(B.name) being called
};

/*
* Menu select button
* Sets screen to name of button, so action = NULL
*/
static BUTTON B_SELECT_5 = (BUTTON) {
    .left = LCD_WIDTH-1-156, 
    .top = LCD_HEIGHT-1-38-8-38-8-38-8-38, 
    .width = 156, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_CYAN,
    .active = true,
    .frame = true,
    .font = &Font30x17,
    .name = "--------",
    .action = NULL  //NULL action results in setCurrScreenIndex(B.name) being called
};

/*
* Menu select button
* Sets screen to name of button, so action = NULL
*/
static BUTTON B_SELECT_6 = (BUTTON) {
    .left = LCD_WIDTH-1-156, 
    .top = LCD_HEIGHT-1-38-8-38-8-38, 
    .width = 156, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_CYAN,
    .active = true,
    .frame = true,
    .font = &Font30x17,
    .name = "--------",
    .action = NULL  //NULL action results in setCurrScreenIndex(B.name) being called
};

/*
* Menu select button
* Sets screen to name of button, so action = NULL
*/
static BUTTON B_SELECT_7 = (BUTTON) {
    .left = LCD_WIDTH-1-156, 
    .top = LCD_HEIGHT-1-38-8-38, 
    .width = 156, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_CYAN,
    .active = true,
    .frame = true,
    .font = &Font30x17,
    .name = "--------",
    .action = NULL  //NULL action results in setCurrScreenIndex(B.name) being called
};

/*
* Menu select button
* Sets screen to name of button, so action = NULL
*/
static BUTTON B_SELECT_8 = (BUTTON) {
    .left = LCD_WIDTH-1-156, 
    .top = LCD_HEIGHT-1-38, 
    .width = 156, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_CYAN,
    .active = true,
    .frame = true,
    .font = &Font30x17,
    .name = "--------",
    .action = NULL  //NULL action results in setCurrScreenIndex(B.name) being called
};

/*
* Shows + next to BPM
* Increments BPM by 1
*/
void A_BPM_PLUSONE() {
    multicore_fifo_push_timeout_us(CORE_REQ_BPM_PLUSONE, 10000);  //send notification to core 1, no validation done
}
static BUTTON B_BPM_PLUSONE = (BUTTON) {
    .left = LCD_WIDTH-1-54-8-54, 
    .top = LCD_HEIGHT-1-38-8-38-8-38-8-38, 
    .width = 54, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_BLUE,
    .active = true,
    .frame = true,
    .font = &Font30x17,
    .name = "+",
    .action = A_BPM_PLUSONE
};

/*
* Shows ++ next to BPM
* Increments BPM by 10
*/
void A_BPM_PLUSTEN() {
    multicore_fifo_push_timeout_us(CORE_REQ_BPM_PLUSTEN, 10000);  //send notification to core 1, no validation done
}
static BUTTON B_BPM_PLUSTEN = (BUTTON) {
    .left = LCD_WIDTH-1-54, 
    .top = LCD_HEIGHT-1-38-8-38-8-38-8-38, 
    .width = 54, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_BLUE,
    .active = true,
    .frame = true,
    .font = &Font30x17,
    .name = "++",
    .action = A_BPM_PLUSTEN
};

/*
* Shows - next to BPM
* Decrements BPM by 1
*/
void A_BPM_MINUSONE() {
    multicore_fifo_push_timeout_us(CORE_REQ_BPM_MINUSONE, 10000);  //send notification to core 1, no validation done
}
static BUTTON B_BPM_MINUSONE = (BUTTON) {
    .left = 0+54+8, 
    .top = LCD_HEIGHT-1-38-8-38-8-38-8-38, 
    .width = 54, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_BLUE,
    .active = true,
    .frame = true,
    .font = &Font30x17,
    .name = "-",
    .action = A_BPM_MINUSONE
};

/*
* Shows -- next to BPM
* Decrements BPM by 10
*/
void A_BPM_MINUSTEN() {
    multicore_fifo_push_timeout_us(CORE_REQ_BPM_MINUSTEN, 10000);  //send notification to core 1, no validation done
}
static BUTTON B_BPM_MINUSTEN = (BUTTON) {
    .left = 0, 
    .top = LCD_HEIGHT-1-38-8-38-8-38-8-38, 
    .width = 54, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_BLUE,
    .active = true,
    .frame = true,
    .font = &Font30x17,
    .name = "--",
    .action = A_BPM_MINUSTEN
};

/*
* Displays current BPM
* Shows help, initialised and clock settings for PRESET
*/
void A_BPM_HELP() {
    CURR_VIEW.control = &SCREENS[CURR_SCREEN_INDEX].PST;
    CURR_VIEW.event = EVENT_DSC;
    CURR_VIEW.type = VIEW_HELP;
    CURR_VIEW.draw = true;
    CURR_VIEW.startTime = get_absolute_time();
}
static BUTTON B_BPM_HELP = (BUTTON) {
    .left = LCD_WIDTH/2-1-35, 
    .top = LCD_HEIGHT-1-38-8-38-8-38-8-38, 
    .width = 70, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_GREEN,
    .active = true,
    .frame = true,
    .font = &Font30x17,
    .name = "---",
    .action = A_BPM_HELP
};

/*
* PRESET Left Switch 0
* Show help
*/
void A_LS0_HELP() {
    CURR_VIEW.control = &SCREENS[CURR_SCREEN_INDEX].LS0;
    CURR_VIEW.event = EVENT_INF;
    CURR_VIEW.type = VIEW_HELP;
    CURR_VIEW.draw = true;
    CURR_VIEW.startTime = get_absolute_time();
}
static BUTTON B_LS0_HELP = (BUTTON) {
    .left = 0, 
    .top = LCD_HEIGHT-1-38, 
    .width = 156, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_CYAN,
    .active = false,
    .frame = true,
    .font = &Font30x17,
    .name = "--------",
    .action = A_LS0_HELP
};

/*
* PRESET Left Switch 1
* Show help
*/
void A_LS1_HELP() {
    CURR_VIEW.control = &SCREENS[CURR_SCREEN_INDEX].LS1;
    CURR_VIEW.event = EVENT_INF;
    CURR_VIEW.type = VIEW_HELP;
    CURR_VIEW.draw = true;
    CURR_VIEW.startTime = get_absolute_time();
}
static BUTTON B_LS1_HELP = (BUTTON) {
    .left = 0, 
    .top = LCD_HEIGHT-1-38-8-38, 
    .width = 156, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_CYAN,
    .active = false,
    .frame = true,
    .font = &Font30x17,
    .name = "--------",
    .action = A_LS1_HELP
};

/*
* PRESET Left Switch 2
* Show help
*/
void A_LS2_HELP() {
    CURR_VIEW.control = &SCREENS[CURR_SCREEN_INDEX].LS2;
    CURR_VIEW.event = EVENT_INF;
    CURR_VIEW.type = VIEW_HELP;
    CURR_VIEW.draw = true;
    CURR_VIEW.startTime = get_absolute_time();
}
static BUTTON B_LS2_HELP = (BUTTON) {
    .left = 0, 
    .top = LCD_HEIGHT-1-38-8-38-8-38, 
    .width = 156, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_CYAN,
    .active = false,
    .frame = true,
    .font = &Font30x17,
    .name = "--------",
    .action = A_LS2_HELP
};

/*
* PRESET Right Switch 0
* Show help
*/
void A_RS0_HELP() {
    CURR_VIEW.control = &SCREENS[CURR_SCREEN_INDEX].RS0;
    CURR_VIEW.event = EVENT_INF;
    CURR_VIEW.type = VIEW_HELP;
    CURR_VIEW.draw = true;
    CURR_VIEW.startTime = get_absolute_time();
}
static BUTTON B_RS0_HELP = (BUTTON) {
    .left = LCD_WIDTH-1-156, 
    .top = LCD_HEIGHT-1-38, 
    .width = 156, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_CYAN,
    .active = false,
    .frame = true,
    .font = &Font30x17,
    .name = "--------",
    .action = A_RS0_HELP
};

/*
* PRESET Right Switch 1
* Show help
*/
void A_RS1_HELP() {
    CURR_VIEW.control = &SCREENS[CURR_SCREEN_INDEX].RS1;
    CURR_VIEW.event = EVENT_INF;
    CURR_VIEW.type = VIEW_HELP;
    CURR_VIEW.draw = true;
    CURR_VIEW.startTime = get_absolute_time();
}
static BUTTON B_RS1_HELP = (BUTTON) {
    .left = LCD_WIDTH-1-156, 
    .top = LCD_HEIGHT-1-38-8-38, 
    .width = 156, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_CYAN,
    .active = false,
    .frame = true,
    .font = &Font30x17,
    .name = "--------",
    .action = A_RS1_HELP
};

/*
* PRESET Right Switch 2
* Show help
*/
void A_RS2_HELP() {
    CURR_VIEW.control = &SCREENS[CURR_SCREEN_INDEX].RS2;
    CURR_VIEW.event = EVENT_INF;
    CURR_VIEW.type = VIEW_HELP;
    CURR_VIEW.draw = true;
    CURR_VIEW.startTime = get_absolute_time();
}
static BUTTON B_RS2_HELP = (BUTTON) {
    .left = LCD_WIDTH-1-156, 
    .top = LCD_HEIGHT-1-38-8-38-8-38, 
    .width = 156, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_CYAN,
    .active = false,
    .frame = true,
    .font = &Font30x17,
    .name = "--------",
    .action = A_RS2_HELP
};



/*
* Sytem options - send sysex start
*/
void A_SYSTEM_SYSEX_SEND_START() {
    sendSysexConfig();
    //sendMidiSysex();
}
static BUTTON B_SYSTEM_SYSEX_SEND_START = (BUTTON) {
    .left = LCD_WIDTH-1-130,
    .top = LCD_HEIGHT-1-30-16-30-16-30-16-30, 
    .width = 130, 
    .height = 30, 
    .radius = 10, 
    .color = COLOR_RED,
    .active = true,
    .frame = true,
    .font = &Font20x12,
    .name = "START",
    .action = A_SYSTEM_SYSEX_SEND_START
};

/*
* Sytem options - send sysex
*/
void A_SYSTEM_SYSEX_SEND() {
    confirmButton(B_SYSTEM_SYSEX_SEND_START);
}
static BUTTON B_SYSTEM_SYSEX_SEND = (BUTTON) {
    .left = 0,
    .top = LCD_HEIGHT-1-30-16-30-16-30-16-30, 
    .width = 174, 
    .height = 30, 
    .radius = 10, 
    .color = COLOR_CYAN,
    .active = true,
    .frame = true,
    .font = &Font20x12,
    .name = "SYSEX SEND",
    .action = A_SYSTEM_SYSEX_SEND
};

/*
* Sytem options - recive sysex start
*/
void A_SYSTEM_SYSEX_RECV_START() {
    writeSysexConfigToUserFlash();
}
static BUTTON B_SYSTEM_SYSEX_RECV_START = (BUTTON) {
    .left = LCD_WIDTH-1-130,
    .top = LCD_HEIGHT-1-30-16-30-16-30, 
    .width = 130, 
    .height = 30, 
    .radius = 10, 
    .color = COLOR_RED,
    .active = true,
    .frame = true,
    .font = &Font20x12,
    .name = "START",
    .action = A_SYSTEM_SYSEX_RECV_START
};

/*
* Sytem options - receive sysex
*/
void A_SYSTEM_SYSEX_RECV() {
    confirmButton(B_SYSTEM_SYSEX_RECV_START);
}
static BUTTON B_SYSTEM_SYSEX_RECV = (BUTTON) {
    .left = 0,
    .top = LCD_HEIGHT-1-30-16-30-16-30, 
    .width = 174, 
    .height = 30, 
    .radius = 10, 
    .color = COLOR_CYAN,
    .active = true,
    .frame = true,
    .font = &Font20x12,
    .name = "SYSEX RECV",
    .action = A_SYSTEM_SYSEX_RECV
};

/*
* Sytem options - factory reset
*/
void A_SYSTEM_FACTORY_RESET_CONFIRM() {
    writeFactoryFlashToUserFlash();
}
static BUTTON B_SYSTEM_FACTORY_RESET_CONFIRM = (BUTTON) {
    .left = LCD_WIDTH-1-130,
    .top = LCD_HEIGHT-1-30-16-30, 
    .width = 130, 
    .height = 30, 
    .radius = 10, 
    .color = COLOR_RED,
    .active = true,
    .frame = true,
    .font = &Font20x12,
    .name = "CONFIRM",
    .action = A_SYSTEM_FACTORY_RESET_CONFIRM
};

/*
* Sytem options - factory reset confirm
*/
void A_SYSTEM_FACTORY_RESET() {
    confirmButton(B_SYSTEM_FACTORY_RESET_CONFIRM);
}
static BUTTON B_SYSTEM_FACTORY_RESET = (BUTTON) {
    .left = 0,
    .top = LCD_HEIGHT-1-30-16-30, 
    .width = 174, 
    .height = 30, 
    .radius = 10, 
    .color = COLOR_CYAN,
    .active = true,
    .frame = true,
    .font = &Font20x12,
    .name = "FACTORY RESET",
    .action = A_SYSTEM_FACTORY_RESET
};

/*
* Sytem options - load firmware
*/
void A_SYSTEM_FIRMWARE_LOAD() {
    loadFirmware();
}
static BUTTON B_SYSTEM_FIRMWARE_LOAD = (BUTTON) {
    .left = LCD_WIDTH-1-130,
    .top = LCD_HEIGHT-1-30, 
    .width = 130, 
    .height = 30, 
    .radius = 10, 
    .color = COLOR_RED,
    .active = true,
    .frame = true,
    .font = &Font20x12,
    .name = "LOAD UF2",
    .action = A_SYSTEM_FIRMWARE_LOAD
};

/*
* Sytem options - show firmware
*/
void A_SYSTEM_FIRMWARE_SHOW() {
    confirmButton(B_SYSTEM_FIRMWARE_LOAD);
}
static BUTTON B_SYSTEM_FIRMWARE_SHOW = (BUTTON) {
    .left = 0,
    .top = LCD_HEIGHT-1-30, 
    .width = 174, 
    .height = 30, 
    .radius = 10, 
    .color = COLOR_CYAN,
    .active = true,
    .frame = true,
    .font = &Font20x12,
    .name = "FIRMWARE",
    .action = A_SYSTEM_FIRMWARE_SHOW
};

