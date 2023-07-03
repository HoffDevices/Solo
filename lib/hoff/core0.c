/*
* core0.c
* Contains core 0 utility functions
*/


/*
* Check if FACTORY_FLASH is same as PRESETS_TEXT.
* If not, write PRESETS_TEXT to FACTORY_FLASH (this should happen if there was a firmware update with an updated PRESETS_TEXT)
*/
void updateFactoryFlash(bool forceUpdate) {
    bool presetsSameAsFactoryFlash = true;
    uint32_t i = 0;
    while (PRESETS_TEXT[i] != 0x00 && presetsSameAsFactoryFlash == true) {
        presetsSameAsFactoryFlash = (PRESETS_TEXT[i] == FACTORY_FLASH[i]);
        i++;
    }
    if (presetsSameAsFactoryFlash == false || forceUpdate == true) {  //PRESETS_TEXT is different, so overwrite factory flash with PRESETS_TEXT
        uint32_t status = save_and_disable_interrupts();
        flash_range_erase(FACTORY_FLASH_OFFSET, PRESETS_SIZE);
        flash_range_program(FACTORY_FLASH_OFFSET, (const unsigned char *) PRESETS_TEXT, PRESETS_SIZE);
        restore_interrupts(status);
    }
    //get factory version
    i = 0;  //FACTORY_FLASH index
    char versionText[10] = "---------";  //placeholder for "<VERSION>"
    while (strcmp(versionText,"<VERSION>") != 0) {  //look for a valid <VERSION> string
        for (uint8_t j=0; j<9; j++) versionText[j] = FACTORY_FLASH[i+j];  //the literal definition above already contains 0x00 in pos [9]
        i++;
    }
    i += 8;  //our index was actually at the start of <VERSION> - move it past
    uint8_t j = 0;  //stored value index
    while (USER_FLASH[i+j] != '<') {  //store value until we get the next '<' character
        FACTORY_VERSION[j] = FACTORY_FLASH[i+j];  //the literal definition above already contains 0x00 in pos [9]
        j++;
    }
    FACTORY_VERSION[j] = 0x00;  //close string
}

/*
* Check if USER_FLASH has ever been initialised with presets.
* If not, write PRESETS_TEXT into USER_FLASH (this should only ever happen once when the very first firmware is loaded on a new device)
* Read USER_FLASH into PRESETS_TEXT
*/
void updateUserFlash(bool forceUpdate) {
    char userFlashText[9] = "--------";  //placeholder for "<SYSTEM>"
    for (uint8_t i=0; i<8; i++) userFlashText[i] = USER_FLASH[i];  //the literal definition above already contains 0x00 in pos [8]
    bool userFlashInitialised = (strcmp(userFlashText, "<SYSTEM>") == 0);
    if (userFlashInitialised == false || forceUpdate == true) {  //user flash has not been initialised yet, so overwrite with PRESETS_TEXT
        uint32_t status = save_and_disable_interrupts();
        flash_range_erase(USER_FLASH_OFFSET, PRESETS_SIZE);
        flash_range_program(USER_FLASH_OFFSET, (const unsigned char *) PRESETS_TEXT, PRESETS_SIZE);
        restore_interrupts(status);
    }
    strcpy(PRESETS_TEXT, USER_FLASH);  //load user flash into PRESETS_TEXT
}

/*
* Write factory flash to user flash
*/
void writeFactoryFlashToUserFlash() {
    strcpy(PRESETS_TEXT, FACTORY_FLASH);  //write FACTORY_FLASH to PRESETS_TEXT

    strcpy(B_SYSTEM_FACTORY_RESET_CONFIRM.name, "FLASHING");
    B_SYSTEM_FACTORY_RESET_CONFIRM.color = COLOR_GREEN;
    drawButton(&B_SYSTEM_FACTORY_RESET_CONFIRM);
    multicore_reset_core1();  //prevent interrupts from core 1
    uint32_t status = save_and_disable_interrupts();
    flash_range_erase(USER_FLASH_OFFSET, PRESETS_SIZE);
    flash_range_program(USER_FLASH_OFFSET, (const unsigned char *) PRESETS_TEXT, PRESETS_SIZE);  //write PRESETS_TEXT to USER_FLASH
    restore_interrupts(status);
    busy_wait_ms(2000);

    strcpy(B_SYSTEM_FACTORY_RESET_CONFIRM.name, "REBOOTING");
    B_SYSTEM_FACTORY_RESET_CONFIRM.color = COLOR_GREEN;
    drawButton(&B_SYSTEM_FACTORY_RESET_CONFIRM);
    busy_wait_ms(2000);
    watchdog_reboot(0, 0, 0);  //normal reboot
}

/*
* Write sysex to user flash
*/
void writeSysexConfigToUserFlash() {
    strcpy(B_SYSTEM_SYSEX_RECV_START.name, "READY");
    B_SYSTEM_SYSEX_RECV_START.color = COLOR_GREEN;
    drawButton(&B_SYSTEM_SYSEX_RECV_START);
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
                    strcpy(B_SYSTEM_SYSEX_RECV_START.name, "FINISHED");
                    drawButton(&B_SYSTEM_SYSEX_RECV_START);
                }
            }
        }
        if (sysexStarted == true && sysexFinished == false) {
            char s[10];
            sprintf(s, "%d", MIDI_SYSEX_INDEX);
            strcpy(B_SYSTEM_SYSEX_RECV_START.name, s);
            drawButton(&B_SYSTEM_SYSEX_RECV_START);
        }
    }
    busy_wait_ms(2000);

    strcpy(B_SYSTEM_SYSEX_RECV_START.name, "FLASHING");
    B_SYSTEM_SYSEX_RECV_START.color = COLOR_RED;
    drawButton(&B_SYSTEM_SYSEX_RECV_START);
    multicore_reset_core1();  //prevent interrupts from core 1
    uint32_t status = save_and_disable_interrupts();
    flash_range_erase(USER_FLASH_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(USER_FLASH_OFFSET, (const unsigned char *) PRESETS_TEXT, FLASH_SECTOR_SIZE);
    restore_interrupts(status);
    busy_wait_ms(2000);

    strcpy(B_SYSTEM_SYSEX_RECV_START.name, "REBOOTING");
    B_SYSTEM_SYSEX_RECV_START.color = COLOR_GREEN;
    drawButton(&B_SYSTEM_SYSEX_RECV_START);
    busy_wait_ms(2000);
    watchdog_reboot(0, 0, 0);  //normal reboot
}

/*
* Send user flash to sysex
*/
void sendSysexConfig() {
    strcpy(B_SYSTEM_SYSEX_SEND_START.name, "READY");
    B_SYSTEM_SYSEX_SEND_START.color = COLOR_GREEN;
    drawButton(&B_SYSTEM_SYSEX_SEND_START);
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
                    strcpy(B_SYSTEM_SYSEX_SEND_START.name, "FINISHED");
                    drawButton(&B_SYSTEM_SYSEX_SEND_START);
                }
            }
        }
        if (sysexStarted == true && sysexFinished == false) {
            char s[10];
            sprintf(s, "%d", MIDI_SYSEX_INDEX);
            strcpy(B_SYSTEM_SYSEX_SEND_START.name, s);
            drawButton(&B_SYSTEM_SYSEX_SEND_START);
        }
    }
    busy_wait_ms(2000);
    strcpy(B_SYSTEM_SYSEX_SEND_START.name, "START");  //reset button to original state
    B_SYSTEM_SYSEX_SEND_START.color = COLOR_RED;
}

/*
* Load new .uf2 firmware over USB
*/
void loadFirmware() {
    strcpy(B_SYSTEM_FIRMWARE_LOAD.name, "REBOOTING");
    B_SYSTEM_FIRMWARE_LOAD.color = COLOR_GREEN;
    drawButton(&B_SYSTEM_FIRMWARE_LOAD);
    busy_wait_ms(2000);
    reset_usb_boot(0, 0);  //reboot to usb storage mode
}
