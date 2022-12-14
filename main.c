/*
* Hoff Solo - (c) 2022 R. Haarhoff
* Last updated 2022-12-06
*/

//Raspberry Pi Pico
#include <stdio.h>
#include <stdlib.h>
#include <hardware/watchdog.h>
#include <pico/stdlib.h>
#include <pico/bootrom.h>
#include <pico/multicore.h>
#include <string.h>
#include <hardware/flash.h>

//LCD & touch
#include "LCD_Driver.h"
#include "LCD_Touch.h"
#include "DEV_Config.h"

//TUSB MIDI
#include "bsp/board.h"
#include "tusb.h"

//Hoff Solo - we're not using much in the way of header declarations, so the order of the includes are important
#include "main.h"
#include "common.c"
#include "switch.c"
#include "midi.c"
#include "core1.c"
#include "draw.c"
#include "button.c"
#include "screen.c"
#include "core0.c"


/*
* Main - core 1
* All timing sensitive routines happen here, i.e. response to switch events, clock events
*/
void main_core1() {
    //init midi
    MIDI_USB.init();
    MIDI_LEFT.init();
    MIDI_RIGHT.init();

    //init switches
    SWITCH_LS0.init();
    SWITCH_LS1.init();
    SWITCH_LS2.init();
    SWITCH_RS0.init();
    SWITCH_RS1.init();
    SWITCH_RS2.init();

    while (true) {
        checkSwitches();
        discardMidi();

        if (multicore_fifo_rvalid() == true) {  //read comms from core 0
            uint32_t rcvFlag;
            uint32_t timeout = 1000;  //sending flags back to core 0 should be really quick
            if (multicore_fifo_pop_timeout_us(timeout, &rcvFlag) == true) {
                if (rcvFlag == CORE_REQ_PRESET_ENTER) {
                    enterPreset();
                    multicore_fifo_push_timeout_us(CORE_ACK_PRESET_ENTER, timeout);  //send acknowledgment, but don't check for validity
                }
                if (rcvFlag == CORE_REQ_PRESET_EXIT) {
                    exitPreset();
                    multicore_fifo_push_timeout_us(CORE_ACK_PRESET_EXIT, timeout);  //send acknowledgment, but don't check for validity
                }
                if (rcvFlag == CORE_REQ_TIMER_DISABLE) {
                    cancel_repeating_timer(&MIDI_CLOCK_TIMER);  //cancel is safe to do even if not previously activated
                    multicore_fifo_push_timeout_us(CORE_ACK_TIMER_DISABLE, timeout);  //send acknowledgment, but don't check for validity
                }
                if (rcvFlag == CORE_REQ_SYSEX_SEND) {
                    multicore_fifo_push_timeout_us(CORE_ACK_SYSEX_SEND_STARTED, timeout);  //send acknowledgment, but don't check for validity
                    sendMidiSysex();
                    multicore_fifo_push_timeout_us(CORE_ACK_SYSEX_SEND_FINISHED, timeout);  //send acknowledgment, but don't check for validity
                }
                if (rcvFlag == CORE_REQ_SYSEX_RECV) {
                    multicore_fifo_push_timeout_us(CORE_ACK_SYSEX_RECV_STARTED, timeout);  //send acknowledgment, but don't check for validity
                    recvMidiSysex();
                    multicore_fifo_push_timeout_us(CORE_ACK_SYSEX_RECV_FINISHED, timeout);  //send acknowledgment, but don't check for validity
                }
                if (rcvFlag == CORE_REQ_BPM_PLUSONE) {  //no need to acknowledge, as the display will update the BPM
                    incrementBPM(1);
                }
                if (rcvFlag == CORE_REQ_BPM_PLUSTEN) {  //no need to acknowledge, as the display will update the BPM
                    incrementBPM(10);
                }
                if (rcvFlag == CORE_REQ_BPM_MINUSONE) {  //no need to acknowledge, as the display will update the BPM
                    incrementBPM(-1);
                }
                if (rcvFlag == CORE_REQ_BPM_MINUSTEN) {  //no need to acknowledge, as the display will update the BPM
                    incrementBPM(-10);
                }
            }
        }	
    }
}

/*
* Main - core 0
* each called screenFunction will exit before loading the next - this way we exit each function without overloading the stack
* we also let each screen run itself, so it can execute some things only once and not every loop (as opposed to if we were looping through all screens and checking screen)
* pass screen by reference (pointer) so any button can set a new screen
*/
int main(void) {
    if (watchdog_enable_caused_reboot()) reset_usb_boot(0, 0);  //put into firmware update mode if wathdog times out
    watchdog_enable(10000, 0);   //10 second watchdog

    System_Init();               //LCD GPIO setup
	LCD_Init(SCAN_DIR_DFT,400);  //init LCD
    SWITCH_TOUCH.init();         //init touch

    showLogo(1000);        //show the logo while we do housekeeping
    updateFactoryFlash();  //update FACTORY_FLASH to latest PRESETS_TEXT in firmware
    updateUserFlash();     //update USER_FLASH if needed and load PRESETS_TEXT from USER_FLASH
    loadSystemScreens();   //set up the system screens
    loadPresetScreens();   //set up the preset screens
    findSnake();           //update the SNAKE preset's show method

    multicore_launch_core1(main_core1);   //core1 handles all MIDI events

    setCurrScreenIndex(PRESETS_DEFAULT);             //PRESETS_DEFAULT gets populated by loadPresetScreens
    while (true) SCREENS[CURR_SCREEN_INDEX].show();  //main program loop - CURR_SCREEN_INDEX is modified by other events to show the appropriate screen
    
	return 0;
}
