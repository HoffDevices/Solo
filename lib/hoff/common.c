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
