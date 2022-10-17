/*
* common.c
* Contains common utility functions (core0 and core1)
*/


/*
* Find a screen by its unique name
*/
void setCurrScreenIndex(char *findName) {
    uint8_t lenScreens = sizeof(SCREENS)/sizeof(SCREEN);
    uint8_t i = 0;
    while (i<lenScreens && strcmp(SCREENS[i].name, findName) != 0) i++;  //strcmp returns 0 when strings are equal
    if (i<lenScreens) CURR_SCREEN_INDEX = i;  //found valid screen
    else CURR_SCREEN_INDEX = 0;  //Default to first system screen if findName was not found
    CURR_VIEW.active = false;  //Exit from currently running screen
}

/*
* Send flag to other core, wait for and verify acknowledged flag
*/
bool sendInterCoreFlag(uint32_t trxFlag, uint32_t ackFlag, uint32_t timeout) {
    if (multicore_fifo_push_timeout_us(trxFlag, timeout) == true) {
        uint32_t rcvFlag;
        if (multicore_fifo_pop_timeout_us(timeout, &rcvFlag) == true) {
            return (rcvFlag == ackFlag);
        }
    }
    return false;  //if we got here then something broke or timed out
}

/*
* Get touch status into global variable. Manage seperation of presses via .canRead.
*/
void getTouch() {
    TP_DRAW touch = TP_Touch();  //get touch status
    LAST_TOUCH.x = touch.Xpoint;
    LAST_TOUCH.y = touch.Ypoint;
    LAST_TOUCH.isPressed = (touch.Color == RED);  //hack to use LCD_Touch code
    if (LAST_TOUCH.canRead == false && LAST_TOUCH.isPressed == false) LAST_TOUCH.canRead = true;  //wait until released before setting canRead again
}

/*
* Check if button is touched, then execute action. Manage seperation of presses via .canRead.
*/
void touchButton(BUTTON B) {
    if (LAST_TOUCH.canRead == true && LAST_TOUCH.isPressed == true  //wait until previous action completed
        && LAST_TOUCH.x > (B.left) && LAST_TOUCH.x < (B.left + B.width) 
        && LAST_TOUCH.y > (B.top)  && LAST_TOUCH.y < (B.top + B.height)) {
            LAST_TOUCH.canRead = false;  //prevent further reads until getTouch sets this to true
            if (strcmp(B.name, "--------") != 0 && B.action == NULL) setCurrScreenIndex(B.name);  //we cannot define a function with a variable parameter at the button level, so we use this mechanism for specifically selectButtons
            else B.action();
        }
}

