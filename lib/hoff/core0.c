/*
* core0.c
* Contains core 0 utility functions
*/


/*
* Draws 8 buttons for a select screen
*/
void selectButtons(BUTTON *mainButtons, uint8_t offset, bool system, bool preset) {
    //BUTTON mainButtons[3+8];  //max, could be less
    //uint8_t lenScreens = sizeof(SCREENS)/sizeof(SCREEN);
    uint8_t j = 0;
    //draw candidate buttons up to 8, or until we run out of screens
    for (int i=0 + CURR_VIEW.page*8; i<MAX_SCREEN_INDEX && j<8; i++) {
        if (SCREENS[i].system == system && SCREENS[i].preset == preset) {
            mainButtons[j+offset].name = SCREENS[i].name;
            j++;
        }
    }
    //fill up rest of emptybuttons
    for (uint8_t i=j; i<8; i++) {
        mainButtons[i+offset].name = "--------";
    }
}


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
        drawString(160, 1.5 * Font20x12.Height, "RELEASED SHORT", &Font20x12, COLOR_CYAN, 0, -1);  //event title text
        breakLines(CURR_VIEW.control->RLS, breakText, &breakRows, 25, ' ');  //break text into lines of 25 chars = 100
    }
    else if (CURR_VIEW.event == EVENT_RLL) {
        drawString(160, 1.5 * Font20x12.Height, "RELEASED LONG", &Font20x12, COLOR_CYAN, 0, -1);  //event title text
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

    remainRows = 9;  //lines remaining for valueText
    if (breakRows < remainRows) {  //less lines than can fit on screen
        limitRows = breakRows;  //smallest row count wins
        limitStart = 0;
        limitExtra = 0;
    }
    else {  //more lines than can fit on screen
        limitRows = remainRows;
        limitExtra = (breakRows - remainRows + 1);  //lines extra that don't fit in the remaining space
        limitStart = (uint8_t) (absolute_time_diff_us(CURR_VIEW.startTime, get_absolute_time()) / 2000000) % limitExtra;  //start line increments every second
    }
    limitLines(breakText, limitText, limitStart, limitRows);  //limit text to limitRows
    drawString(160, 38 + 8 + 8 + limitRows * Font20x12.Height / 2, limitText, &Font20x12, COLOR_GREEN, COLOR_BLACK, 26);
}

/*
* Draws the appropriate screen and checks the buttons
*/
void handleScreen(BUTTON mainButtons[], int mainLength, void (*setupMain)(), void (*updateMain)()) {
    CURR_VIEW.type = VIEW_MAIN;  //default view is main
    CURR_VIEW.draw = true;  //force initial draw
    CURR_VIEW.active = true;  //remain active until set otherwise by another action, at which point we exit to next screen
    while (CURR_VIEW.active == true) {
        if (CURR_VIEW.type == VIEW_MAIN) {  //show main view
            if (CURR_VIEW.draw == true) {  //only draw this once when needed
                drawFilledRectangle(0,0,LCD_WIDTH,LCD_HEIGHT,COLOR_BLACK);
                for (uint8_t i=0; i<mainLength; i++) drawButton(mainButtons[i], 0);
                setupMain();
                CURR_VIEW.draw = false;
            }
            updateMain();
            getTouch();  //get touch status into global, only after screen draw done
            for (uint8_t i=0; i<mainLength; i++) touchButton(mainButtons[i]);  //check buttons for touch continuously
        }
        else if (CURR_VIEW.type == VIEW_HELP) {  //show help view
            BUTTON helpButtons[] = {B_BCK_HELP, B_FWD_HELP};  //help view buttons
            uint8_t helpLength = sizeof(helpButtons)/sizeof(BUTTON);
            if (CURR_VIEW.draw == true) {  //only draw this once when needed
                drawFilledRectangle(0,0,LCD_WIDTH,LCD_HEIGHT,COLOR_BLACK);
                for (uint8_t i=0; i<helpLength; i++) drawButton(helpButtons[i], 0);                
                CURR_VIEW.draw = false;
            }
            else {
                presetHelp();
            }
            getTouch();  //get touch status into global, only after screen draw done
            for (int i=0; i<helpLength; i++) touchButton(helpButtons[i]);  //check buttons for touch continuously
        }
        watchdog_update();  //watchdog will put us into usb firmware load mode if not updated
    }
}

/*
* Check if factory flash is same as config text
* If not, write CONFIG_TEXT as the factory default
*/
bool checkFactoryFlash() {
    bool configSameAsFactoryFlash = true;
    uint32_t i = 0;
    while (CONFIG_TEXT[i] != 0x00 && configSameAsFactoryFlash == true) {
        configSameAsFactoryFlash = (CONFIG_TEXT[i] == FACTORY_FLASH[i]);
        i++;
    }
    if (configSameAsFactoryFlash == false) {  //CONFIG_TEXT is different, so overwrite factory flash with CONFIG_TEXT
        uint32_t status = save_and_disable_interrupts();
        flash_range_erase(FACTORY_FLASH_OFFSET, CONFIG_SIZE);
        flash_range_program(FACTORY_FLASH_OFFSET, CONFIG_TEXT, CONFIG_SIZE);
        restore_interrupts(status);
    }
    return configSameAsFactoryFlash;
}

/*
* Read user flash into CONFIG_TEXT
*/
bool checkUserFlash() {
    char userFlashText[9] = "--------";  //placeholder for "<SYSTEM>"
    for (uint8_t i=0; i<8; i++) userFlashText[i] = USER_FLASH[i];  //the literal definition above already contains 0x00 in pos [8]
    bool userFlashInitialised = (strcmp(userFlashText, "<SYSTEM>") == 0);

    if (userFlashInitialised == false) {  //user flash has not been initialised yet, so overwrite with CONFIG_TEXT
        uint32_t status = save_and_disable_interrupts();
        flash_range_erase(USER_FLASH_OFFSET, CONFIG_SIZE);
        flash_range_program(USER_FLASH_OFFSET, CONFIG_TEXT, CONFIG_SIZE);
        restore_interrupts(status);
    }
    strcpy(CONFIG_TEXT, USER_FLASH);  //load user flash into CONFIG_TEXT
    return userFlashInitialised;
}

/*
* Write factory flash to user flash
*/
void writeFactoryFlashToUserFlash() {
    strcpy(CONFIG_TEXT, FACTORY_FLASH);

    B_SYSTEM_FACTORY_RESET_CONFIRM.name = "FLASHING";
    B_SYSTEM_FACTORY_RESET_CONFIRM.color = COLOR_GREEN;
    drawButton(B_SYSTEM_FACTORY_RESET_CONFIRM, 10);
    multicore_reset_core1();  //prevent interrupts from core 1
    uint32_t status = save_and_disable_interrupts();
    flash_range_erase(USER_FLASH_OFFSET, CONFIG_SIZE);
    flash_range_program(USER_FLASH_OFFSET, CONFIG_TEXT, CONFIG_SIZE);
    restore_interrupts(status);
    busy_wait_ms(2000);

    B_SYSTEM_FACTORY_RESET_CONFIRM.name = "REBOOTING";
    B_SYSTEM_FACTORY_RESET_CONFIRM.color = COLOR_GREEN;
    drawButton(B_SYSTEM_FACTORY_RESET_CONFIRM, 10);
    busy_wait_ms(2000);
    watchdog_reboot(0, 0, 0);  //normal reboot
}

/*
* Write sysex to user flash
*/
void writeSysexConfigToUserFlash() {
    B_SYSTEM_SYSEX_RECV_START.name = "READY";
    B_SYSTEM_SYSEX_RECV_START.color = COLOR_GREEN;
    drawButton(B_SYSTEM_SYSEX_RECV_START, 10);
    multicore_fifo_push_timeout_us(CORE_REQ_SYSEX_RECV, 10000);  //send notification to core 1, no validation done

    bool sysexFinished = false;
    bool sysexStarted = false;
    while (sysexFinished == false) {
        if (multicore_fifo_rvalid() == true) {  //read comms from core 1
            uint32_t rcvFlag;
            uint32_t timeout = 10000;
            if (multicore_fifo_pop_timeout_us(timeout, &rcvFlag) == true) {
                if (rcvFlag == CORE_ACK_SYSEX_RECV_STARTED) {
                    sysexStarted = true;
                }
                if (rcvFlag == CORE_ACK_SYSEX_RECV_FINISHED) {
                    sysexFinished = true;
                    B_SYSTEM_SYSEX_RECV_START.name = "FINISHED";
                    drawButton(B_SYSTEM_SYSEX_RECV_START, 10);
                }
            }
        }
        if (sysexStarted == true && sysexFinished == false) {
            char s[10];
            sprintf(s, "%d", MIDI_SYSEX_INDEX);
            B_SYSTEM_SYSEX_RECV_START.name = s;
            drawButton(B_SYSTEM_SYSEX_RECV_START, 10);
        }
    }
    busy_wait_ms(2000);

    B_SYSTEM_SYSEX_RECV_START.name = "FLASHING";
    B_SYSTEM_SYSEX_RECV_START.color = COLOR_RED;
    drawButton(B_SYSTEM_SYSEX_RECV_START, 10);
    multicore_reset_core1();  //prevent interrupts from core 1
    uint32_t status = save_and_disable_interrupts();
    flash_range_erase(USER_FLASH_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(USER_FLASH_OFFSET, CONFIG_TEXT, FLASH_SECTOR_SIZE);
    restore_interrupts(status);
    busy_wait_ms(2000);

    B_SYSTEM_SYSEX_RECV_START.name = "REBOOTING";
    B_SYSTEM_SYSEX_RECV_START.color = COLOR_GREEN;
    drawButton(B_SYSTEM_SYSEX_RECV_START, 10);
    busy_wait_ms(2000);
    watchdog_reboot(0, 0, 0);  //normal reboot
}

/*
* Send user flash to sysex
*/
void sendSysexConfig() {
    B_SYSTEM_SYSEX_SEND_START.name = "READY";
    B_SYSTEM_SYSEX_SEND_START.color = COLOR_GREEN;
    drawButton(B_SYSTEM_SYSEX_SEND_START, 10);
    multicore_fifo_push_timeout_us(CORE_REQ_SYSEX_SEND, 10000);  //send notification to core 1, no validation done

    bool sysexFinished = false;
    bool sysexStarted = false;
    while (sysexFinished == false) {
        if (multicore_fifo_rvalid() == true) {  //read comms from core 1
            uint32_t rcvFlag;
            uint32_t timeout = 10000;
            if (multicore_fifo_pop_timeout_us(timeout, &rcvFlag) == true) {
                if (rcvFlag == CORE_ACK_SYSEX_SEND_STARTED) {
                    sysexStarted = true;
                }
                if (rcvFlag == CORE_ACK_SYSEX_SEND_FINISHED) {
                    sysexFinished = true;
                    B_SYSTEM_SYSEX_SEND_START.name = "FINISHED";
                    drawButton(B_SYSTEM_SYSEX_SEND_START, 10);
                }
            }
        }
        if (sysexStarted == true && sysexFinished == false) {
            char s[10];
            sprintf(s, "%d", MIDI_SYSEX_INDEX);
            B_SYSTEM_SYSEX_SEND_START.name = s;
            drawButton(B_SYSTEM_SYSEX_SEND_START, 10);
        }
    }
    busy_wait_ms(2000);
    B_SYSTEM_SYSEX_SEND_START.name = "START";  //reset button to original state
    B_SYSTEM_SYSEX_SEND_START.color = COLOR_RED;
}

/*
* Write factory flash to user flash
*/
void loadFirmware() {
    B_SYSTEM_FIRMWARE_LOAD.name = "REBOOTING";
    B_SYSTEM_FIRMWARE_LOAD.color = COLOR_GREEN;
    drawButton(B_SYSTEM_FIRMWARE_LOAD, 10);
    busy_wait_ms(2000);
    reset_usb_boot(0, 0);  //reboot to usb storage mode
}