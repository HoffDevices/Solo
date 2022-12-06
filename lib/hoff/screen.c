/*
* screen.c
* Contains screen definitions and utility functions
*/


/*
* Breaks an input string every maxChars, or every breakChar or \n
* Output string gets a new line every break
*/
void breakLines(char *inputText, char *outputText, uint8_t *rows, uint8_t maxChars, char breakChar) {
    uint8_t i = 0;  //input text index
    uint8_t j = 0;  //output text index
    uint8_t c = 0;  //columns count in current row
    uint8_t r = 1;  //row count

    while (j < 255 && inputText[i] != 0x00) {  //keep within outputText bounds, and inputText limit
        if (inputText[i] == breakChar || inputText[i] == '\n') {
            outputText[j] = '\n';  //insert new line
            j++;  //track output chars
            r++;  //count rows with every new line
            c=0;  //reset columns
        }
        else if (c >= maxChars) {
            outputText[j] = '\n';  //insert new line
            j++;  //track output chars
            r++;  //count rows with every new line
            c=0;  //reset columns
            outputText[j] = inputText[i];  //insert character that overflowed
            j++;  //track output chars
            c++;  //track columns
        }
        else {
            outputText[j] = inputText[i];
            j++;  //track output chars
            c++;  //track columns
        }
        i++;  //track input
    }
    outputText[j] = 0x00;  //terminate string;
    *rows = r;
}

/*
* Limit which lines are shown
*/
void limitLines(char *inputText, char *outputText, uint8_t startRow, uint8_t maxRows) {
    uint8_t i = 0;  //input text index
    uint8_t j = 0;  //output text index
    uint8_t r = 0;  //row count
    while (i < 255 && inputText[i] != 0x00) {  //keep within outputText bounds, and inputText limit
        if (inputText[i] == '\n') r++;  //track rows
        if (r >= startRow && r < (startRow + maxRows) && !(j==0 && inputText[i] == '\n')) {
            outputText[j] = inputText[i];
            j++;
        }
        i++;  //track text
    }
    outputText[j] = 0x00;  //terminate string;
}

/*
* Gets the label and property value for help display from the current VIEW
*/
void presetHelp() {
    uint8_t breakRows = 0;
    uint8_t limitRows = 0;
    uint8_t limitStart = 0;
    uint8_t limitExtra = 0;
    uint8_t remainRows = 0;
    char breakText[256];  //temp store for text with line breaks added
    char limitText[256];  //limited lines text

    drawString(160, 0.5 * Font20x12.Height, CURR_VIEW.control->LBL, &Font20x12, COLOR_CYAN, 0, -1);  //control (switch) label

    if      (CURR_VIEW.event == EVENT_INF) {
        drawString(160, 1.5 * Font20x12.Height, "INFO", &Font20x12, COLOR_CYAN, 0, -1);  //event title text
        breakLines(CURR_VIEW.control->INF, breakText, &breakRows, 25, '\n');  //break text into lines of 25 chars = 100
    }
    if      (CURR_VIEW.event == EVENT_PRS) {
        drawString(160, 1.5 * Font20x12.Height, "PRESSED", &Font20x12, COLOR_CYAN, 0, -1);  //event title text
        breakLines(CURR_VIEW.control->PRS, breakText, &breakRows, 25, ' ');  //break text into lines of 25 chars = 100
    }
    else if (CURR_VIEW.event == EVENT_RLS) {
        drawString(160, 1.5 * Font20x12.Height, "RELEASED(S)", &Font20x12, COLOR_CYAN, 0, -1);  //event title text
        breakLines(CURR_VIEW.control->RLS, breakText, &breakRows, 25, ' ');  //break text into lines of 25 chars = 100
    }
    else if (CURR_VIEW.event == EVENT_RLL) {
        drawString(160, 1.5 * Font20x12.Height, "RELEASED(L)", &Font20x12, COLOR_CYAN, 0, -1);  //event title text
        breakLines(CURR_VIEW.control->RLL, breakText, &breakRows, 25, ' ');  //break text into lines of 25 chars = 100
    }
    else if (CURR_VIEW.event == EVENT_DSC) {
        drawString(160, 1.5 * Font20x12.Height, "DESCRIPTION", &Font20x12, COLOR_CYAN, 0, -1);  //event title text
        breakLines(CURR_VIEW.control->DSC, breakText, &breakRows, 25, '\n');  //break text into lines of 25 chars = 100
    }
    else if (CURR_VIEW.event == EVENT_CLK) {
        drawString(160, 1.5 * Font20x12.Height, "MIDI CLOCK", &Font20x12, COLOR_CYAN, 0, -1);  //event title text
        breakLines(CURR_VIEW.control->CLK, breakText, &breakRows, 25, ' ');  //break text into lines of 25 chars = 100
    }
    else if (CURR_VIEW.event == EVENT_ENT) {
        drawString(160, 1.5 * Font20x12.Height, "ENTER", &Font20x12, COLOR_CYAN, 0, -1);  //event title text
        breakLines(CURR_VIEW.control->ENT, breakText, &breakRows, 25, ' ');  //break text into lines of 25 chars = 100
    }
    else if (CURR_VIEW.event == EVENT_EXT) {
        drawString(160, 1.5 * Font20x12.Height, "EXIT", &Font20x12, COLOR_CYAN, 0, -1);  //event title text
        breakLines(CURR_VIEW.control->EXT, breakText, &breakRows, 25, ' ');  //break text into lines of 25 chars = 100
    }

    //enable/disable scroll buttons depending on overflow state
    B_SCROLL_UP_PRESET.active = false;
    B_SCROLL_DN_PRESET.active = false;
    remainRows = 9;  //lines remaining for valueText
    if (breakRows < remainRows) {  //less lines than can fit on screen
        limitRows = breakRows;  //smallest row count wins
        limitStart = 0;
        limitExtra = 0;

    }
    else {  //more lines than can fit on screen
        limitRows = remainRows;
        limitExtra = (breakRows - remainRows);  //lines extra that don't fit in the remaining space
        if (CURR_VIEW.startLine >= limitExtra) CURR_VIEW.startLine = limitExtra;
        limitStart = CURR_VIEW.startLine;
        if (limitStart > 0) B_SCROLL_UP_PRESET.active = true;
        else B_SCROLL_DN_PRESET.active = true;
    }
    limitLines(breakText, limitText, limitStart, limitRows);  //limit text to limitRows
    drawString(160, 38 + 8 + 8 + limitRows * Font20x12.Height / 2, limitText, &Font20x12, COLOR_WHITE, 0, -1);
}

/*
* Menu to select presets
* Midi clock is not stopped, but may not be visible because there is no MSG_??? command assigned to the preset event.
*/
void S_MENU() {
    char s[10];
    sprintf(s, "MENU %d", (CURR_VIEW.page + 1));
    strcpy(B_LABEL_STATIC.name, s);
    drawFilledRectangle(0,0,LCD_WIDTH,LCD_HEIGHT,COLOR_BLACK);

    BUTTON *menuButtons[] = {&B_BCK_PRESET, &B_LABEL_STATIC, &B_FWD_PRESET, &B_SELECT_1, &B_SELECT_2, &B_SELECT_3, &B_SELECT_4, &B_SELECT_5, &B_SELECT_6, &B_SELECT_7, &B_SELECT_8};
    uint8_t menuLength = 11;
    
    uint8_t j = 0;
    uint8_t offset = 3;  //first 3 sbuttons are not presets
    //draw candidate buttons up to 8, or until we run out of screens
    for (int i=0 + CURR_VIEW.page*8; i<MAX_SCREEN_INDEX && j<8; i++) {
        if (SCREENS[i].preset == true) {
            strcpy(menuButtons[j+offset]->name, SCREENS[i].name);
            if (SCREENS[i].system == true) menuButtons[j+offset]->color = COLOR_WHITE;
            else menuButtons[j+offset]->color = COLOR_CYAN;
            menuButtons[j+offset]->active = true;
            j++;
        }
    }
    //fill up rest of empty buttons
    for (uint8_t i=j; i<8; i++) {
        strcpy(menuButtons[i+offset]->name, "--------");
        menuButtons[j+offset]->color = COLOR_CYAN;
        menuButtons[i+offset]->active = false;
    }
    drawButtons(menuButtons, menuLength);

    CURR_VIEW.type = VIEW_MAIN;  //default view is main
    CURR_VIEW.draw = true;  //force initial draw
    CURR_VIEW.active = true;  //remain active until set otherwise by another action, at which point we exit to next screen
    while (CURR_VIEW.active == true) {
        checkButtons(menuButtons, menuLength);  //check buttons for touch continuously
        watchdog_update();  //watchdog will put us into usb firmware load mode if not updated
    }
}

/*
* System "preset"
* Disable timer clock as it can mess with sysex and flash operations
*/
void S_SYSTEM() {
    if (sendInterCoreFlag(CORE_REQ_TIMER_DISABLE, CORE_ACK_TIMER_DISABLE, 10000) == false) showInfo("ERROR\nDISABLING TIMER",1000);  //disable clock timer - timeout of 10ms because we might have a lot of functions running in the midi clock
    strcpy(B_LABEL_STATIC.name, "SYSTEM");
    char presets[14] = "PRESETS ";  //leave space for 4 version characters - same size as button name
    strcpy(B_SYSTEM_FACTORY_RESET.name, strcat(presets, PRESETS_VERSION));
    char firmware[14] = "FIRMWARE ";  //leave space for 4 version characters - same size as button name
    strcpy(B_SYSTEM_FIRMWARE_SHOW.name, strcat(firmware, FIRMWARE_VERSION));
    drawFilledRectangle(0,0,LCD_WIDTH,LCD_HEIGHT,COLOR_BLACK);

    BUTTON *systemButtons[] = {&B_BCK_PRESET, &B_LABEL_STATIC, &B_SYSTEM_SYSEX_SEND, &B_SYSTEM_SYSEX_RECV, &B_SYSTEM_FACTORY_RESET, &B_SYSTEM_FIRMWARE_SHOW};
    uint8_t systemLength = 6;
    drawButtons(systemButtons, systemLength);

    CURR_VIEW.type = VIEW_MAIN;  //default view is main
    CURR_VIEW.draw = true;  //force initial draw
    CURR_VIEW.active = true;  //remain active until set otherwise by another action, at which point we exit to next screen
    while (CURR_VIEW.active == true) {
        checkButtons(systemButtons, systemLength);  //check buttons for touch continuously
        watchdog_update();  //watchdog will put us into usb firmware load mode if not updated
    }
}

/*
* The game "Snake" - activated by using a reserved word "SNAKE" as a preset name
*/
void S_SNAKE() {
    if (sendInterCoreFlag(CORE_REQ_PRESET_ENTER, CORE_ACK_PRESET_ENTER, 10000) == false) showInfo("ERROR\nENTERING PRESET",1000);  //enable clock timer - timeout of 10ms because we might have a lot of functions in the preset init property

    strcpy(PRESETS_DEFAULT, SCREENS[CURR_SCREEN_INDEX].name);  //save preset screen name as default

    //arena size in blocks
    uint8_t arena_t = 7;
    uint8_t arena_l = 0;
    uint8_t arena_w = 31;
    uint8_t arena_h = 16;
    uint8_t arena_b = arena_t + arena_h;
    uint8_t arena_r = arena_l + arena_w;

    //snake setup
    typedef struct {
        int8_t x;
        int8_t y;
    } SNAKE_SEGMENT;

    SNAKE_SEGMENT snake[SNAKE_SIZE];
    SNAKE_SEGMENT target = (SNAKE_SEGMENT) {  //first target in fixed position
        .x=arena_w/4,
        .y=arena_w/4
    };
    uint8_t snakeLength = 1;
    bool snakeUpdated = false;
    bool snakeAlive = true;

    for (uint8_t i=0; i<SNAKE_SIZE; i++) {
        if (i <= snakeLength) {  //initial body length, inlcluding tail
            snake[i].x = arena_w/2;
            snake[i].y = arena_h/2+i;
        }
        else {
            snake[i].x = 0;
            snake[i].y = 0;
        }
    }
    SNAKE_DIRECTION = 0;  //reset direction on preset startup

    CURR_VIEW.type = VIEW_MAIN;  //default view is main
    CURR_VIEW.draw = true;  //force initial draw
    CURR_VIEW.active = true;  //remain active until set otherwise by another action, at which point we exit to next screen
    while (CURR_VIEW.active == true) {
        if (CURR_VIEW.type == VIEW_MAIN) {  //show main view
            BUTTON *mainButtons[] = {&B_LABEL_MENU};  //main view buttons
            uint8_t mainLength = 1;
            if (CURR_VIEW.draw == true) {  //only draw this once when needed
                drawFilledRectangle(0,0,LCD_WIDTH,LCD_HEIGHT,COLOR_BLACK);
                drawButtons(mainButtons, mainLength);
                drawString(160, 60, "TARGETS LEFT: ", &Font20x12, COLOR_BLUE, 0, 0);
                CURR_VIEW.draw = false;
            }
            if (MIDI_CLOCK_COUNTER % 6 == 0 && snakeUpdated == false && snakeAlive == true) {  //update snake 4x per beat
                //draw target
                drawFilledRectangle((arena_l + target.x)*10, (arena_t + target.y)*10, 10, 10, COLOR_BLUE);  //draw target
                //draw snake
                drawFilledRectangle((arena_l + snake[snakeLength].x)*10, (arena_t + snake[snakeLength].y)*10, 10, 10, COLOR_BLACK);  //erase tail
                drawFilledRectangle((arena_l + snake[0].x)*10, (arena_t + snake[0].y)*10, 10, 10, COLOR_GREEN);  //draw head
                //propagate body
                for (uint8_t i=snakeLength; i>0; i--) {
                    snake[i] = snake[i-1];
                }
                //manage direction
                if (SNAKE_DIRECTION > 3) SNAKE_DIRECTION = 0;  //bounds
                else if (SNAKE_DIRECTION < 0) SNAKE_DIRECTION = 3;
                if (SNAKE_DIRECTION==0) {  //north
                    snake[0].x = snake[1].x + 0;
                    snake[0].y = snake[1].y - 1;   
                }
                else if (SNAKE_DIRECTION==1) {  //east
                    snake[0].x = snake[1].x + 1;
                    snake[0].y = snake[1].y + 0;   
                }
                else if (SNAKE_DIRECTION==2) {  //south
                    snake[0].x = snake[1].x + 0;
                    snake[0].y = snake[1].y + 1;   
                }
                else if (SNAKE_DIRECTION==3) { //west
                    snake[0].x = snake[1].x - 1;
                    snake[0].y = snake[1].y + 0;   
                }
                //arena bounds
                if (snake[0].x > arena_w) snake[0].x = 0;  //x bounds
                else if (snake[0].x < 0) snake[0].x =  arena_w;
                if (snake[0].y > arena_h) snake[0].y = 0;  //y bounds
                else if (snake[0].y < 0) snake[0].y =  arena_h;
                //self collision check
                for (uint8_t i=1; i<snakeLength; i++) {  //YOU LOSE!
                    if (snake[i].x == snake[0].x && snake[i].y == snake[0].y) {
                        snakeAlive = false;
                        drawFilledRectangle((arena_l + snake[0].x)*10, (arena_t + snake[0].y)*10, 10, 10, COLOR_RED);  //draw head
                        drawString((320/2)+1, (50/2)+2, "YOU LOSE", &Font68x39, COLOR_RED, COLOR_BLACK, 8);
                        drawString(160, 60, "TARGETS LEFT: ", &Font20x12, COLOR_BLUE, 0, 0);
                    }
                }
                //target collision check
                if (target.x == snake[0].x && target.y == snake[0].y) {
                    snakeLength++;  //grow snake
                    snake[snakeLength].x = snake[snakeLength-1].x;
                    snake[snakeLength].y = snake[snakeLength-1].y;   

                    if (snakeLength >= SNAKE_SIZE) {  //YOU WIN!
                        snakeAlive = false;
                        drawString((320/2)+1, (50/2)+2, "YOU WIN!", &Font68x39, COLOR_GREEN, COLOR_BLACK, 8);
                        drawString(160, 60, "TARGETS LEFT: ", &Font20x12, COLOR_BLUE, 0, 0);
                    }
                    bool targetOnSnake = true;
                    while (targetOnSnake == true) {  //repeat until new target is not on snake
                        target.x = rand() % arena_w;  //reasonably random
                        target.y = rand() % arena_h;
                        targetOnSnake = false;  //reset to false before search
                        for (uint8_t i=1; i<snakeLength; i++) 
                            if (target.x == snake[i].x && target.y == snake[i].y) targetOnSnake = true;
                    }
                }
                //score
                char scoreText[4];  //3 digits + 0x00
                sprintf(scoreText, "%d", (SNAKE_SIZE-snakeLength));
                drawString(250, 60, scoreText, &Font20x12, COLOR_BLUE, COLOR_BLACK, 2);
                snakeUpdated = true;
            }
            else if (MIDI_CLOCK_COUNTER % 6 != 0) snakeUpdated = false;

            checkButtons(mainButtons, mainLength);  //check buttons for touch continuously
        }
        watchdog_update();  //watchdog will put us into usb firmware load mode if not updated
    }
    if (sendInterCoreFlag(CORE_REQ_PRESET_EXIT, CORE_ACK_PRESET_EXIT, 10000) == false) showInfo("ERROR\nEXITING PRESET",1000);  //disable clock timer - timeout of 10ms because we might have a lot of functions running in the midi clock
}

/*
* Generic Preset screen
*/
void S_PRESET() {
    if (sendInterCoreFlag(CORE_REQ_PRESET_ENTER, CORE_ACK_PRESET_ENTER, 10000) == false) showInfo("ERROR\nENTERING PRESET",1000);  //enable clock timer and preset entry stuff- timeout of 10ms because we might have a lot of functions in the preset init property

    strcpy(PRESETS_DEFAULT, SCREENS[CURR_SCREEN_INDEX].name);  //save preset screen name as default
    strcpy(B_BPM_BIG_HELP.name, "---");
    strcpy(B_BPM_HELP.name, "---");  //prevent old pointer from displaying garbage

    if (strlen(SCREENS[CURR_SCREEN_INDEX].LS0.LBL) > 0) {
        B_LS0_HELP.active = true;
        strcpy(B_LS0_HELP.name, SCREENS[CURR_SCREEN_INDEX].LS0.LBL);
    }
    else {
        B_LS0_HELP.active = false;
        strcpy(B_LS0_HELP.name, " ");
    }

    if (strlen(SCREENS[CURR_SCREEN_INDEX].LS1.LBL) > 0) {
        B_LS1_HELP.active = true;
        strcpy(B_LS1_HELP.name, SCREENS[CURR_SCREEN_INDEX].LS1.LBL);
        B_LS1_SML_HELP.active = true;
        strcpy(B_LS1_SML_HELP.name, "L1");
    }
    else {
        B_LS1_HELP.active = false;
        strcpy(B_LS1_HELP.name, " ");
        B_LS1_SML_HELP.active = false;
        strcpy(B_LS1_SML_HELP.name, " ");
    }

    if (strlen(SCREENS[CURR_SCREEN_INDEX].LS2.LBL) > 0) {
        B_LS2_HELP.active = true;
        strcpy(B_LS2_HELP.name, SCREENS[CURR_SCREEN_INDEX].LS2.LBL);
        B_LS2_SML_HELP.active = true;
        strcpy(B_LS2_SML_HELP.name, "L2");
    }
    else {
        B_LS2_HELP.active = false;
        strcpy(B_LS2_HELP.name, " ");
        B_LS2_SML_HELP.active = false;
        strcpy(B_LS2_SML_HELP.name, " ");
    }

    if (strlen(SCREENS[CURR_SCREEN_INDEX].RS0.LBL) > 0) {
        B_RS0_HELP.active = true;
        strcpy(B_RS0_HELP.name, SCREENS[CURR_SCREEN_INDEX].RS0.LBL);
    }
    else {
        B_RS0_HELP.active = false;
        strcpy(B_RS0_HELP.name, " ");
    }

    if (strlen(SCREENS[CURR_SCREEN_INDEX].RS1.LBL) > 0) {
        B_RS1_HELP.active = true;
        strcpy(B_RS1_HELP.name, SCREENS[CURR_SCREEN_INDEX].RS1.LBL);
        B_RS1_SML_HELP.active = true;
        strcpy(B_RS1_SML_HELP.name, "R1");
    }
    else {
        B_RS1_HELP.active = false;
        strcpy(B_RS1_HELP.name, " ");
        B_RS1_SML_HELP.active = false;
        strcpy(B_RS1_SML_HELP.name, " ");
    }

    if (strlen(SCREENS[CURR_SCREEN_INDEX].RS2.LBL) > 0) {
        B_RS2_HELP.active = true;
        strcpy(B_RS2_HELP.name, SCREENS[CURR_SCREEN_INDEX].RS2.LBL);
        B_RS2_SML_HELP.active = true;
        strcpy(B_RS2_SML_HELP.name, "R2");
    }
    else {
        B_RS2_HELP.active = false;
        strcpy(B_RS2_HELP.name, " ");
        B_RS2_SML_HELP.active = false;
        strcpy(B_RS2_SML_HELP.name, " ");
    }

    CURR_VIEW.type = VIEW_MAIN;  //default view is main
    CURR_VIEW.draw = true;  //force initial draw
    CURR_VIEW.active = true;  //remain active until set otherwise by another action, at which point we exit to next screen
    while (CURR_VIEW.active == true) {
        if (CURR_VIEW.type == VIEW_MAIN) {  //show main view
            BUTTON *mainButtons[] = {&B_LABEL_MENU, &B_BPM_MINUSTEN, &B_BPM_MINUSONE, &B_BPM_HELP, &B_BPM_PLUSONE, &B_BPM_PLUSTEN, &B_LS2_HELP, &B_RS2_HELP, &B_LS1_HELP, &B_RS1_HELP, &B_LS0_HELP, &B_RS0_HELP};  //main view buttons
            uint8_t mainLength = 12;
            if (CURR_VIEW.draw == true) {  //only draw this once when needed
                initBpmBuffer();
                drawFilledRectangle(0,0,LCD_WIDTH,LCD_HEIGHT,COLOR_BLACK);
                drawButtons(mainButtons, mainLength);
                CURR_VIEW.draw = false;
            }
            //show BPM
            int bpm = (int64_t) (-60000000 / 24 / MIDI_CLOCK_TIMER_US);  //24 midi clocks per BPM
            char bpmText[4];  //3 digits + 0x00
            sprintf(bpmText, "%d", bpm);
            strcpy(B_BPM_HELP.name, bpmText);
            drawButton(&B_BPM_HELP);
            checkButtons(mainButtons, mainLength);  //check buttons for touch continuously
        }
        else if (CURR_VIEW.type == VIEW_BPM) {  //show bpm view
            BUTTON *bpmButtons[] = {&B_LABEL_MENU, &B_BPM_MINUSTEN, &B_BPM_MINUSONE, &B_BPM_BIG_HELP, &B_BPM_PLUSONE, &B_BPM_PLUSTEN, &B_LS2_SML_HELP, &B_RS2_SML_HELP, &B_LS1_SML_HELP, &B_RS1_SML_HELP, &B_LS0_HELP, &B_RS0_HELP};  //bpm view buttons
            uint8_t bpmLength = 12;
            if (CURR_VIEW.draw == true) {  //only draw this once when needed
                initBpmBuffer();
                drawFilledRectangle(0,0,LCD_WIDTH,LCD_HEIGHT,COLOR_BLACK);
                drawButtons(bpmButtons, bpmLength);
                CURR_VIEW.draw = false;
            }
            //show BPM
            int bpm = (int64_t) (-60000000 / 24 / MIDI_CLOCK_TIMER_US);  //24 midi clocks per BPM
            char bpmText[4];  //3 digits + 0x00
            sprintf(bpmText, "%d", bpm);
            strcpy(B_BPM_BIG_HELP.name, bpmText);
            drawButton(&B_BPM_BIG_HELP);
            checkButtons(bpmButtons, bpmLength);  //check buttons for touch continuously
        }
        else if (CURR_VIEW.type == VIEW_HELP) {  //show help view
            BUTTON *helpButtons[] = {&B_BCK_HELP, &B_FWD_HELP, &B_SCROLL_UP_PRESET, &B_SCROLL_DN_PRESET};  //help view buttons
            uint8_t helpLength = 4;
            if (CURR_VIEW.draw == true) {  //only draw this once when needed
                drawFilledRectangle(0,0,LCD_WIDTH,LCD_HEIGHT,COLOR_BLACK);
                presetHelp();  //modifies active state of scroll buttons
                drawButtons(helpButtons, helpLength);                
                CURR_VIEW.draw = false;
            }
            checkButtons(helpButtons, helpLength);  //check buttons for touch continuously
        }
        watchdog_update();  //watchdog will put us into usb firmware load mode if not updated
    }
    if (sendInterCoreFlag(CORE_REQ_PRESET_EXIT, CORE_ACK_PRESET_EXIT, 10000) == false) showInfo("ERROR\nEXITING PRESET",1000);  //disable clock timer - timeout of 10ms because we might have a lot of functions running in the midi clock
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
        .system = true,
        .preset = false,
        .name = "MENU",
        .show = S_MENU
    };
    SCREENS[1] = (SCREEN) {
        .system = true,
        .preset = true,
        .name = "SYSTEM",
        .show = S_SYSTEM
    };
}

/*
* Loads the PRESETS_TEXT flash string and parses it into pointers for SCREEN properties.
* Also destructively converts '<' char to 0x00 to indicate end of strings
*/
void loadPresetScreens() {
    //find first empty screen slot
    uint8_t p = 0;
    while (strcmp(SCREENS[p].name, "--------") != 0) p++;
    p--;  //we pre-increment below with each new preset
    //parsing process
    bool readProperty = false;
    char textProperty[8];  //max is probably 8 (???_??? + 0x00)
    char parentProperty[8];  //max is probably 8 (???_??? + 0x00)
    uint32_t i=0;  //PRESETS_TEXT index
    uint8_t k=0;  //property/value string index
    while (PRESETS_TEXT[i] != 0x00) {

        if (PRESETS_TEXT[i] == '<') {
            readProperty = true;
            PRESETS_TEXT[i] = 0x00;  //start of new property implies termination of previous property string (including comment)
            k=0;
        }
        if (PRESETS_TEXT[i] == '>' && readProperty == true) {  //update property
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
                if      (strcmp(textProperty, "VERSION") == 0) PRESETS_VERSION = &PRESETS_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "DEFAULT") == 0) PRESETS_DEFAULT = &PRESETS_TEXT[i+1];  //pointer to start of property
            }
            else if (strcmp(parentProperty, "PRESET") == 0) {  //preset properties
                if      (strcmp(textProperty, "PST_LBL") == 0) {
                    SCREENS[p].PST.LBL = &PRESETS_TEXT[i+1];  //pointer to start of property, which is the next char
                    SCREENS[p].name = &PRESETS_TEXT[i+1];  //pointer to start of property, which is the next char
                }
                else if (strcmp(textProperty, "PST_DSC") == 0) SCREENS[p].PST.DSC = &PRESETS_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "PST_CLK") == 0) SCREENS[p].PST.CLK = &PRESETS_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "PST_ENT") == 0) SCREENS[p].PST.ENT = &PRESETS_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "PST_EXT") == 0) SCREENS[p].PST.EXT = &PRESETS_TEXT[i+1];  //pointer to start of property

                else if (strcmp(textProperty, "LS0_LBL") == 0) SCREENS[p].LS0.LBL = &PRESETS_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "LS0_INF") == 0) SCREENS[p].LS0.INF = &PRESETS_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "LS0_PRS") == 0) SCREENS[p].LS0.PRS = &PRESETS_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "LS0_RLS") == 0) SCREENS[p].LS0.RLS = &PRESETS_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "LS0_RLL") == 0) SCREENS[p].LS0.RLL = &PRESETS_TEXT[i+1];  //pointer to start of property

                else if (strcmp(textProperty, "LS1_LBL") == 0) SCREENS[p].LS1.LBL = &PRESETS_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "LS1_INF") == 0) SCREENS[p].LS1.INF = &PRESETS_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "LS1_PRS") == 0) SCREENS[p].LS1.PRS = &PRESETS_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "LS1_RLS") == 0) SCREENS[p].LS1.RLS = &PRESETS_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "LS1_RLL") == 0) SCREENS[p].LS1.RLL = &PRESETS_TEXT[i+1];  //pointer to start of property

                else if (strcmp(textProperty, "LS2_LBL") == 0) SCREENS[p].LS2.LBL = &PRESETS_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "LS2_INF") == 0) SCREENS[p].LS2.INF = &PRESETS_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "LS2_PRS") == 0) SCREENS[p].LS2.PRS = &PRESETS_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "LS2_RLS") == 0) SCREENS[p].LS2.RLS = &PRESETS_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "LS2_RLL") == 0) SCREENS[p].LS2.RLL = &PRESETS_TEXT[i+1];  //pointer to start of property

                else if (strcmp(textProperty, "RS0_LBL") == 0) SCREENS[p].RS0.LBL = &PRESETS_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "RS0_INF") == 0) SCREENS[p].RS0.INF = &PRESETS_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "RS0_PRS") == 0) SCREENS[p].RS0.PRS = &PRESETS_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "RS0_RLS") == 0) SCREENS[p].RS0.RLS = &PRESETS_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "RS0_RLL") == 0) SCREENS[p].RS0.RLL = &PRESETS_TEXT[i+1];  //pointer to start of property

                else if (strcmp(textProperty, "RS1_LBL") == 0) SCREENS[p].RS1.LBL = &PRESETS_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "RS1_INF") == 0) SCREENS[p].RS1.INF = &PRESETS_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "RS1_PRS") == 0) SCREENS[p].RS1.PRS = &PRESETS_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "RS1_RLS") == 0) SCREENS[p].RS1.RLS = &PRESETS_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "RS1_RLL") == 0) SCREENS[p].RS1.RLL = &PRESETS_TEXT[i+1];  //pointer to start of property

                else if (strcmp(textProperty, "RS2_LBL") == 0) SCREENS[p].RS2.LBL = &PRESETS_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "RS2_INF") == 0) SCREENS[p].RS2.INF = &PRESETS_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "RS2_PRS") == 0) SCREENS[p].RS2.PRS = &PRESETS_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "RS2_RLS") == 0) SCREENS[p].RS2.RLS = &PRESETS_TEXT[i+1];  //pointer to start of property
                else if (strcmp(textProperty, "RS2_RLL") == 0) SCREENS[p].RS2.RLL = &PRESETS_TEXT[i+1];  //pointer to start of property
            }
        }
        if (PRESETS_TEXT[i]!='<' && PRESETS_TEXT[i]!='>' && PRESETS_TEXT[i]!=0x10 && PRESETS_TEXT[i]!=0x13 && PRESETS_TEXT[i]!=0x00) {  //ignore property markers and cr/lf and 0x00
            if (readProperty == true) {
                textProperty[k] = PRESETS_TEXT[i];
                k++;
            }
        }
        i++;
    }
}

/*
* Change the SNAKE preset's show method to S_SNAKE(), and makes it a system preset
*/
void findSnake() {
    uint8_t lenScreens = sizeof(SCREENS)/sizeof(SCREEN);
    uint8_t i = 0;
    while (i<lenScreens && strcmp(SCREENS[i].name, "SNAKE") != 0) i++;  //strcmp returns 0 when strings are equal
    if (i<lenScreens) {  //found valid screen
        SCREENS[i].show = S_SNAKE;
        SCREENS[i].system = true;
    }
}
