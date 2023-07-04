/*
* button.c
* Contains touch screen button stuff
*/


/*
* Check if button is touched, execute appropriate function.
*/
void checkButtons(BUTTON **buttons, uint8_t length) {
    SWITCH_TOUCH.check();  //get touch status into global, only after screen draw done
    for (int i=0; i<length; i++) {
        BUTTON *B = buttons[i];
        if (SWITCH_TOUCH.x > (B->left) && SWITCH_TOUCH.x < (B->left + B->width) 
         && SWITCH_TOUCH.y > (B->top)  && SWITCH_TOUCH.y < (B->top + B->height)) {  //within button bounds
            if (SWITCH_TOUCH.releasedShort == true && B->releasedShort != NULL) B->releasedShort();
            if (SWITCH_TOUCH.releasedLong == true && B->releasedLong != NULL) B->releasedLong();
            if (SWITCH_TOUCH.pressed == true && B->pressed != NULL) B->pressed();
            if (SWITCH_TOUCH.timedOut == true && B->timedOut != NULL) B->timedOut();
        }
    }
    //clear states if no buttons were activated
    SWITCH_TOUCH.pressed = false;  //safer to reset state before function as it may do its own state checking
    SWITCH_TOUCH.releasedShort = false;  //reset all the switch states regardless
    SWITCH_TOUCH.releasedLong = false; 
    SWITCH_TOUCH.timedOut = false;
}

/*
* Draw buttons
*/
void drawButtons(BUTTON **buttons, uint8_t length) {
    for (uint8_t i=0; i<length; i++) drawButton(buttons[i]);
}

/*
* Confirm system options
*/
void confirmButton(BUTTON *B) {
    BUTTON *confirmButtons[] = {B};
    int confirmLength = 1;
    drawButtons(confirmButtons, confirmLength);  //trying to keep the pattern consistent even with just 1 button

    absolute_time_t waitTime = get_absolute_time();
    while (absolute_time_diff_us(waitTime, get_absolute_time()) < 3000000) {  //3 seconds
        checkButtons(confirmButtons, confirmLength);  //check buttons for touch continuously
    }
    drawFilledRectangle(B->left, B->top, B->width+1, B->height+1, COLOR_BLACK);
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
    else if (CURR_VIEW.event == EVENT_RLL) {
        CURR_VIEW.event = EVENT_TIM;
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
    CURR_VIEW.startLine = 0;
}
static BUTTON B_FWD_HELP = (BUTTON) {
    .left = LCD_WIDTH-1-38,
    .top = 0, 
    .width = 38, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_CYAN,
    .chars = 0,
    .active = true,
    .frame = true,
    .buffer = false,
    .font = &Font30x17,
    .name = ">",
    .releasedShort = A_FWD_HELP,
    .releasedLong = NULL,
    .timedOut = NULL
};

/*
* Show back button, top left
* Cycle backward through events : PRS <- SRT <- LNG <- MAIN
*/
void A_BCK_HELP() {
    if (CURR_VIEW.event == EVENT_TIM) {
        CURR_VIEW.event = EVENT_RLL;
        CURR_VIEW.type = VIEW_HELP;
    }
    else if (CURR_VIEW.event == EVENT_RLL) {
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
    CURR_VIEW.startLine = 0;
}
static BUTTON B_BCK_HELP = (BUTTON) {
    .left = 0,
    .top = 0, 
    .width = 38, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_CYAN,
    .chars = 0,
    .active = true,
    .frame = true,
    .buffer = false,
    .font = &Font30x17,
    .name = "<",
    .releasedShort = A_BCK_HELP,
    .releasedLong = NULL,
    .timedOut = NULL
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
        setCurrScreenIndex(PRESETS_DEFAULT);
    }
}
static BUTTON B_FWD_PRESET = (BUTTON) {
    .left = LCD_WIDTH-1-38,
    .top = 0, 
    .width = 38, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_WHITE,
    .chars = 0,
    .active = true,
    .frame = true,
    .buffer = false,
    .font = &Font30x17,
    .name = ">",
    .releasedShort = A_FWD_PRESET,
    .releasedLong = NULL,
    .timedOut = NULL
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
        setCurrScreenIndex(PRESETS_DEFAULT);
    }
}
static BUTTON B_BCK_PRESET = (BUTTON) {
    .left = 0, 
    .top = 0, 
    .width = 38, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_WHITE,
    .chars = 0,
    .active = true,
    .frame = true,
    .buffer = false,
    .font = &Font30x17,
    .name = "<",
    .releasedShort = A_BCK_PRESET,
    .releasedLong = NULL,
    .timedOut = NULL
};

/*
* Show scroll up button, top left
* Scroll up a line
*/
void A_SCROLL_UP_PRESET() {
    CURR_VIEW.draw = true;
    if (CURR_VIEW.startLine > 0) CURR_VIEW.startLine--;
}
static BUTTON B_SCROLL_UP_PRESET = (BUTTON) {
    .left = 8+38,
    .top = 0, 
    .width = 38, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_BLUE,
    .chars = 0,
    .active = true,
    .frame = true,
    .buffer = false,
    .font = &Font30x17,
    .name = "^",
    .releasedShort = A_SCROLL_UP_PRESET,
    .releasedLong = NULL,
    .timedOut = NULL
};

/*
* Show scroll down button, top right
* Scroll down a line
*/
void A_SCROLL_DN_PRESET() 
{
    CURR_VIEW.draw = true;
    CURR_VIEW.startLine++;  //bounds checking done in the screen
}
static BUTTON B_SCROLL_DN_PRESET = (BUTTON) {
    .left = LCD_WIDTH-1-38-8-38,
    .top = 0, 
    .width = 38, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_BLUE,
    .chars = 0,
    .active = true,
    .frame = true,
    .buffer = false,
    .font = &Font30x17,
    .name = "`",  //actual remapped to down caret
    .releasedShort = A_SCROLL_DN_PRESET,
    .releasedLong = NULL,
    .timedOut = NULL
};

/*
* Displays current screen's name, top middle
* Does nothing
*/
static BUTTON B_LABEL_STATIC = (BUTTON) {
    .left = LCD_WIDTH/2-1-228/2, 
    .top = 0, 
    .width = 228, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_WHITE,
    .chars = 0,
    .active = true,
    .frame = false,
    .buffer = false,
    .font = &Font30x17,
    .name = "",
    .releasedShort = NULL,
    .releasedLong = NULL,
    .timedOut = NULL
};

/*
* Displays current screen's name, top middle
* Sets screen to menu - short press
* Shows preset help - timedout
*/
void A_LABEL_MENU_TIMEDOUT() {
    CURR_VIEW.control = &SCREENS[CURR_SCREEN_INDEX].PST;
    CURR_VIEW.event = EVENT_DSC;
    CURR_VIEW.type = VIEW_HELP;
    CURR_VIEW.draw = true;
    CURR_VIEW.startLine = 0;
}
void A_LABEL_MENU_RELEASEDSHORT() {
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
    .chars = 0,
    .active = true,
    .frame = false,
    .buffer = false,
    .font = &Font68x39,
    .name = "",//empty string gets populated by screen name
    .releasedShort = A_LABEL_MENU_RELEASEDSHORT,
    .releasedLong = NULL,
    .timedOut = A_LABEL_MENU_TIMEDOUT
};

/*
* Menu select button
* Sets screen to name of button, so releasedShort = NULL
*/
void A_SELECT_1();
static BUTTON B_SELECT_1 = (BUTTON) {
    .left = 0,
    .top = LCD_HEIGHT-1-38-8-38-8-38-8-38, 
    .width = 156, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_CYAN,
    .chars = 0,
    .active = true,
    .frame = true,
    .buffer = false,
    .font = &Font30x17,
    .name = "--------",
    .releasedShort = A_SELECT_1,
    .releasedLong = NULL,
    .timedOut = NULL
};
void A_SELECT_1() {
    setCurrScreenIndex(B_SELECT_1.name);
}

/*
* Menu select button
* Sets screen to name of button, so releasedShort = NULL
*/
void A_SELECT_2();
static BUTTON B_SELECT_2 = (BUTTON) {
    .left = 0,
    .top = LCD_HEIGHT-1-38-8-38-8-38, 
    .width = 156, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_CYAN,
    .chars = 0,
    .active = true,
    .frame = true,
    .buffer = false,
    .font = &Font30x17,
    .name = "--------",
    .releasedShort = A_SELECT_2,
    .releasedLong = NULL,
    .timedOut = NULL
};
void A_SELECT_2() {
    setCurrScreenIndex(B_SELECT_2.name);
}

/*
* Menu select button
* Sets screen to name of button, so releasedShort = NULL
*/
void A_SELECT_3();
static BUTTON B_SELECT_3 = (BUTTON) {
    .left = 0,
    .top = LCD_HEIGHT-1-38-8-38, 
    .width = 156, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_CYAN,
    .chars = 0,
    .active = true,
    .frame = true,
    .buffer = false,
    .font = &Font30x17,
    .name = "--------",
    .releasedShort = A_SELECT_3,
    .releasedLong = NULL,
    .timedOut = NULL
};
void A_SELECT_3() {
    setCurrScreenIndex(B_SELECT_3.name);
}

/*
* Menu select button
* Sets screen to name of button, so releasedShort = NULL
*/
void A_SELECT_4();
static BUTTON B_SELECT_4 = (BUTTON) {
    .left = 0,
    .top = LCD_HEIGHT-1-38, 
    .width = 156, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_CYAN,
    .chars = 0,
    .active = true,
    .frame = true,
    .buffer = false,
    .font = &Font30x17,
    .name = "--------",
    .releasedShort = A_SELECT_4,
    .releasedLong = NULL,
    .timedOut = NULL
};
void A_SELECT_4() {
    setCurrScreenIndex(B_SELECT_4.name);
}

/*
* Menu select button
* Sets screen to name of button, so releasedShort = NULL
*/
void A_SELECT_5();
static BUTTON B_SELECT_5 = (BUTTON) {
    .left = LCD_WIDTH-1-156, 
    .top = LCD_HEIGHT-1-38-8-38-8-38-8-38, 
    .width = 156, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_CYAN,
    .chars = 0,
    .active = true,
    .frame = true,
    .buffer = false,
    .font = &Font30x17,
    .name = "--------",
    .releasedShort = A_SELECT_5,
    .releasedLong = NULL,
    .timedOut = NULL
};
void A_SELECT_5() {
    setCurrScreenIndex(B_SELECT_5.name);
}

/*
* Menu select button
* Sets screen to name of button, so releasedShort = NULL
*/
void A_SELECT_6();
static BUTTON B_SELECT_6 = (BUTTON) {
    .left = LCD_WIDTH-1-156, 
    .top = LCD_HEIGHT-1-38-8-38-8-38, 
    .width = 156, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_CYAN,
    .chars = 0,
    .active = true,
    .frame = true,
    .buffer = false,
    .font = &Font30x17,
    .name = "--------",
    .releasedShort = A_SELECT_6,
    .releasedLong = NULL,
    .timedOut = NULL
};
void A_SELECT_6() {
    setCurrScreenIndex(B_SELECT_6.name);
}

/*
* Menu select button
* Sets screen to name of button, so releasedShort = NULL
*/
void A_SELECT_7();
static BUTTON B_SELECT_7 = (BUTTON) {
    .left = LCD_WIDTH-1-156, 
    .top = LCD_HEIGHT-1-38-8-38, 
    .width = 156, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_CYAN,
    .chars = 0,
    .active = true,
    .frame = true,
    .buffer = false,
    .font = &Font30x17,
    .name = "--------",
    .releasedShort = A_SELECT_7,
    .releasedLong = NULL,
    .timedOut = NULL
};
void A_SELECT_7() {
    setCurrScreenIndex(B_SELECT_7.name);
}

/*
* Menu select button
* Sets screen to name of button, so releasedShort = NULL
*/
void A_SELECT_8();
static BUTTON B_SELECT_8 = (BUTTON) {
    .left = LCD_WIDTH-1-156, 
    .top = LCD_HEIGHT-1-38, 
    .width = 156, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_CYAN,
    .chars = 0,
    .active = true,
    .frame = true,
    .buffer = false,
    .font = &Font30x17,
    .name = "--------",
    .releasedShort = A_SELECT_8,
    .releasedLong = NULL,
    .timedOut = NULL
};
void A_SELECT_8() {
    setCurrScreenIndex(B_SELECT_8.name);
}

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
    .chars = 0,
    .active = true,
    .frame = true,
    .buffer = false,
    .font = &Font30x17,
    .name = "+",
    .releasedShort = A_BPM_PLUSONE,
    .releasedLong = NULL,
    .timedOut = NULL
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
    .chars = 0,
    .active = true,
    .frame = true,
    .buffer = false,
    .font = &Font30x17,
    .name = "++",
    .releasedShort = A_BPM_PLUSTEN,
    .releasedLong = NULL,
    .timedOut = NULL
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
    .chars = 0,
    .active = true,
    .frame = true,
    .buffer = false,
    .font = &Font30x17,
    .name = "-",
    .releasedShort = A_BPM_MINUSONE,
    .releasedLong = NULL,
    .timedOut = NULL
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
    .chars = 0,
    .active = true,
    .frame = true,
    .buffer = false,
    .font = &Font30x17,
    .name = "--",
    .releasedShort = A_BPM_MINUSTEN,
    .releasedLong = NULL,
    .timedOut = NULL
};

/*
* Displays current BPM
* Shows help, initialised and clock settings for PRESET
*/
void A_BPM_HELP() {
    CURR_VIEW.type = VIEW_BPM;
    CURR_VIEW.draw = true;
    CURR_VIEW.startLine = 0;
}
static BUTTON B_BPM_HELP = (BUTTON) {
    .left = LCD_WIDTH/2-1-35, 
    .top = LCD_HEIGHT-1-38-8-38-8-38-8-38, 
    .width = 70, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_GREEN,
    .chars = 3,
    .active = true,
    .frame = true,
    .buffer = false,
    .font = &Font30x17,
    .name = "---",
    .releasedShort = A_BPM_HELP,
    .releasedLong = NULL,
    .timedOut = NULL
};

/*
* PRESET Left Switch 0
* Show help
*/
void A_BPM_BIG_HELP() {
    CURR_VIEW.type = VIEW_MAIN;
    CURR_VIEW.draw = true;
    CURR_VIEW.startLine = 0;
}
static BUTTON B_BPM_BIG_HELP = (BUTTON) {
    .left = 54+8, 
    .top = LCD_HEIGHT-1-38-8-38-8-38, 
    .width = LCD_WIDTH-54-8-54-8-1, 
    .height = 38+8+38, 
    .radius = 10, 
    .color = COLOR_GREEN,
    .chars = 3,
    .active = true,
    .frame = true,
    .buffer = true,
    .font = &Font104x60,
    .name = "---",
    .releasedShort = A_BPM_BIG_HELP,
    .releasedLong = NULL,
    .timedOut = NULL
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
    CURR_VIEW.startLine = 0;
}
static BUTTON B_LS0_HELP = (BUTTON) {
    .left = 0, 
    .top = LCD_HEIGHT-1-38, 
    .width = 156, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_CYAN,
    .chars = 0,
    .active = false,
    .frame = true,
    .buffer = false,
    .font = &Font30x17,
    .name = "--------",
    .releasedShort = NULL,
    .releasedLong = NULL,
    .timedOut = A_LS0_HELP
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
    CURR_VIEW.startLine = 0;
}
static BUTTON B_LS1_HELP = (BUTTON) {
    .left = 0, 
    .top = LCD_HEIGHT-1-38-8-38, 
    .width = 156, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_CYAN,
    .chars = 0,
    .active = false,
    .frame = true,
    .buffer = false,
    .font = &Font30x17,
    .name = "--------",
    .releasedShort = NULL,
    .releasedLong = NULL,
    .timedOut = A_LS1_HELP
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
    CURR_VIEW.startLine = 0;
}
static BUTTON B_LS2_HELP = (BUTTON) {
    .left = 0, 
    .top = LCD_HEIGHT-1-38-8-38-8-38, 
    .width = 156, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_CYAN,
    .chars = 0,
    .active = false,
    .frame = true,
    .buffer = false,
    .font = &Font30x17,
    .name = "--------",
    .releasedShort = NULL,
    .releasedLong = NULL,
    .timedOut = A_LS2_HELP
};

/*
* PRESET Left Switch 1 - small version
* Show help
*/
void A_LS1_SML_HELP() {
    CURR_VIEW.control = &SCREENS[CURR_SCREEN_INDEX].LS1;
    CURR_VIEW.event = EVENT_INF;
    CURR_VIEW.type = VIEW_HELP;
    CURR_VIEW.draw = true;
    CURR_VIEW.startLine = 0;
}
static BUTTON B_LS1_SML_HELP = (BUTTON) {
    .left = 0, 
    .top = LCD_HEIGHT-1-38-8-38, 
    .width = 54, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_CYAN,
    .chars = 0,
    .active = false,
    .frame = true,
    .buffer = false,
    .font = &Font30x17,
    .name = "--------",
    .releasedShort = NULL,
    .releasedLong = NULL,
    .timedOut = A_LS1_SML_HELP
};

/*
* PRESET Left Switch 2 - small version
* Show help
*/
void A_LS2_SML_HELP() {
    CURR_VIEW.control = &SCREENS[CURR_SCREEN_INDEX].LS2;
    CURR_VIEW.event = EVENT_INF;
    CURR_VIEW.type = VIEW_HELP;
    CURR_VIEW.draw = true;
    CURR_VIEW.startLine = 0;
}
static BUTTON B_LS2_SML_HELP = (BUTTON) {
    .left = 0, 
    .top = LCD_HEIGHT-1-38-8-38-8-38, 
    .width = 54, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_CYAN,
    .chars = 0,
    .active = false,
    .frame = true,
    .buffer = false,
    .font = &Font30x17,
    .name = "--------",
    .releasedShort = NULL,
    .releasedLong = NULL,
    .timedOut = A_LS2_SML_HELP
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
    CURR_VIEW.startLine = 0;
}
static BUTTON B_RS0_HELP = (BUTTON) {
    .left = LCD_WIDTH-1-156, 
    .top = LCD_HEIGHT-1-38, 
    .width = 156, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_CYAN,
    .chars = 0,
    .active = false,
    .frame = true,
    .buffer = false,
    .font = &Font30x17,
    .name = "--------",
    .releasedShort = NULL,
    .releasedLong = NULL,
    .timedOut = A_RS0_HELP
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
    CURR_VIEW.startLine = 0;
}
static BUTTON B_RS1_HELP = (BUTTON) {
    .left = LCD_WIDTH-1-156, 
    .top = LCD_HEIGHT-1-38-8-38, 
    .width = 156, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_CYAN,
    .chars = 0,
    .active = false,
    .frame = true,
    .buffer = false,
    .font = &Font30x17,
    .name = "--------",
    .releasedShort = NULL,
    .releasedLong = NULL,
    .timedOut = A_RS1_HELP
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
    CURR_VIEW.startLine = 0;
}
static BUTTON B_RS2_HELP = (BUTTON) {
    .left = LCD_WIDTH-1-156, 
    .top = LCD_HEIGHT-1-38-8-38-8-38, 
    .width = 156, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_CYAN,
    .chars = 0,
    .active = false,
    .frame = true,
    .buffer = false,
    .font = &Font30x17,
    .name = "--------",
    .releasedShort = NULL,
    .releasedLong = NULL,
    .timedOut = A_RS2_HELP
};

/*
* PRESET Right Switch 1 - small version
* Show help
*/
void A_RS1_SML_HELP() {
    CURR_VIEW.control = &SCREENS[CURR_SCREEN_INDEX].RS1;
    CURR_VIEW.event = EVENT_INF;
    CURR_VIEW.type = VIEW_HELP;
    CURR_VIEW.draw = true;
    CURR_VIEW.startLine = 0;
}
static BUTTON B_RS1_SML_HELP = (BUTTON) {
    .left = LCD_WIDTH-1-54, 
    .top = LCD_HEIGHT-1-38-8-38, 
    .width = 54, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_CYAN,
    .chars = 0,
    .active = false,
    .frame = true,
    .buffer = false,
    .font = &Font30x17,
    .name = "--------",
    .releasedShort = NULL,
    .releasedLong = NULL,
    .timedOut = A_RS1_SML_HELP
};

/*
* PRESET Right Switch 2 - small version
* Show help
*/
void A_RS2_SML_HELP() {
    CURR_VIEW.control = &SCREENS[CURR_SCREEN_INDEX].RS2;
    CURR_VIEW.event = EVENT_INF;
    CURR_VIEW.type = VIEW_HELP;
    CURR_VIEW.draw = true;
    CURR_VIEW.startLine = 0;
}
static BUTTON B_RS2_SML_HELP = (BUTTON) {
    .left = LCD_WIDTH-1-54, 
    .top = LCD_HEIGHT-1-38-8-38-8-38, 
    .width = 54, 
    .height = 38, 
    .radius = 10, 
    .color = COLOR_CYAN,
    .chars = 0,
    .active = false,
    .frame = true,
    .buffer = false,
    .font = &Font30x17,
    .name = "--------",
    .releasedShort = NULL,
    .releasedLong = NULL,
    .timedOut = A_RS2_SML_HELP
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
    .chars = 10,
    .active = true,
    .frame = true,
    .buffer = false,
    .font = &Font20x12,
    .name = "START",
    .releasedShort = A_SYSTEM_SYSEX_SEND_START,
    .releasedLong = NULL,
    .timedOut = NULL
};

/*
* Sytem options - send sysex
*/
void A_SYSTEM_SYSEX_SEND() {
    confirmButton(&B_SYSTEM_SYSEX_SEND_START);
}
static BUTTON B_SYSTEM_SYSEX_SEND = (BUTTON) {
    .left = 0,
    .top = LCD_HEIGHT-1-30-16-30-16-30-16-30, 
    .width = 174, 
    .height = 30, 
    .radius = 10, 
    .color = COLOR_WHITE,
    .chars = 0,
    .active = true,
    .frame = true,
    .buffer = false,
    .font = &Font20x12,
    .name = "SYSEX SEND",
    .releasedShort = A_SYSTEM_SYSEX_SEND,
    .releasedLong = NULL,
    .timedOut = NULL
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
    .chars = 10,
    .active = true,
    .frame = true,
    .buffer = false,
    .font = &Font20x12,
    .name = "START",
    .releasedShort = A_SYSTEM_SYSEX_RECV_START,
    .releasedLong = NULL,
    .timedOut = NULL
};

/*
* Sytem options - receive sysex
*/
void A_SYSTEM_SYSEX_RECV() {
    confirmButton(&B_SYSTEM_SYSEX_RECV_START);
}
static BUTTON B_SYSTEM_SYSEX_RECV = (BUTTON) {
    .left = 0,
    .top = LCD_HEIGHT-1-30-16-30-16-30, 
    .width = 174, 
    .height = 30, 
    .radius = 10, 
    .color = COLOR_WHITE,
    .chars = 0,
    .active = true,
    .frame = true,
    .buffer = false,
    .font = &Font20x12,
    .name = "SYSEX RECV",
    .releasedShort = A_SYSTEM_SYSEX_RECV,
    .releasedLong = NULL,
    .timedOut = NULL
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
    .chars = 10,
    .active = true,
    .frame = true,
    .buffer = false,
    .font = &Font20x12,
    .name = "CONFIRM",
    .releasedShort = A_SYSTEM_FACTORY_RESET_CONFIRM,
    .releasedLong = NULL,
    .timedOut = NULL
};

/*
* Sytem options - factory reset confirm
*/
void A_SYSTEM_FACTORY_RESET() {
    char presets[14] = "LOAD ";  //leave space for 4 version characters - same size as button name
    strcpy(B_SYSTEM_FACTORY_RESET_CONFIRM.name, strcat(presets, FACTORY_VERSION));
    confirmButton(&B_SYSTEM_FACTORY_RESET_CONFIRM);
}
static BUTTON B_SYSTEM_FACTORY_RESET = (BUTTON) {
    .left = 0,
    .top = LCD_HEIGHT-1-30-16-30, 
    .width = 174, 
    .height = 30, 
    .radius = 10, 
    .color = COLOR_WHITE,
    .chars = 10,
    .active = true,
    .frame = true,
    .buffer = false,
    .font = &Font20x12,
    .name = "FACTORY RESET",
    .releasedShort = A_SYSTEM_FACTORY_RESET,
    .releasedLong = NULL,
    .timedOut = NULL
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
    .chars = 10,
    .active = true,
    .frame = true,
    .buffer = false,
    .font = &Font20x12,
    .name = "LOAD UF2",
    .releasedShort = A_SYSTEM_FIRMWARE_LOAD,
    .releasedLong = NULL,
    .timedOut = NULL
};

/*
* Sytem options - show firmware
*/
void A_SYSTEM_FIRMWARE_SHOW() {
    confirmButton(&B_SYSTEM_FIRMWARE_LOAD);
}
static BUTTON B_SYSTEM_FIRMWARE_SHOW = (BUTTON) {
    .left = 0,
    .top = LCD_HEIGHT-1-30, 
    .width = 174, 
    .height = 30, 
    .radius = 10, 
    .color = COLOR_WHITE,
    .chars = 0,
    .active = true,
    .frame = true,
    .buffer = false,
    .font = &Font20x12,
    .name = "FIRMWARE",
    .releasedShort = A_SYSTEM_FIRMWARE_SHOW,
    .releasedLong = NULL,
    .timedOut = NULL
};
