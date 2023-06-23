/*
* core1.c
* Contains core 1 utility functions
* Functions are not guaranteed to be thread safe, but global variables are OK?
*/

bool midiClock();  //declared but not defined here - used in parseFunction, but midiClock also uses parseFunction

/*
* Parses a comma sperated list of text values into a uint8_t array, and returns the length
* strtol: If the value of base is 0, the numeric base is auto-detected: if the prefix is 0, the base is octal, if the prefix is 0x or 0X, the base is hexadecimal, otherwise the base is decimal.
*/
void parseCSV(char *txt, uint8_t *arr, uint8_t *len) {
    char *t;
    uint8_t i = 0;
    while((t = strsep(&txt,",")) != NULL) {  //arguments seperate by comma
        if (t[0] == '@') {
            t[0] = '0';  //prefix number with a 0 to get rid of the @
            uint8_t v = (uint8_t) strtol(t, NULL, 0);  //convert to value;
            arr[i] = USER_VAR[v];  //get variable value
        }
        else arr[i] = (uint8_t) strtol(t, NULL, 0);  //convert to value and save in array
        i++;
    }
    *len = i;
}

/*
* Run the appropriate code for switch, clock or preset event, depending on interval, function, and argument
* S can be NULL, for PST_ENT, PST_EXT and PST_CLK attributes
* counter can be NULL for PST_ENT, PST_EXT attribute
* arrInterval[0] = the interval to activate
* arrInterval[1] = the number of intervals in a cycle - max 256
*/
void parseFunction(SWITCH *S, uint64_t *counter, char *textInterval, char *textFunction, char *textArgument) {
    uint8_t lenInterval;
    uint8_t arrInterval[2];  //max is probably 2, we don't need a closing 0x00
    parseCSV(textInterval, arrInterval, &lenInterval);  //parse text into array and length

    if (arrInterval[0] != 0 && *counter % arrInterval[1] == arrInterval[0] % arrInterval[1]) {  //check step number versus number of steps in a cycle - both get modulo'd. The first counter increment should happen before this check, so start at 1
        if (strcmp(textFunction,"TAP_BPM") == 0) {  //24 midi clocks per BPM, takes one argument
            uint8_t lenArgument;
            uint8_t arrArgument[1];  //max is probably 1, we don't need a closing 0x00
            parseCSV(textArgument, arrArgument, &lenArgument);  //parse text into array and length
            if (S != NULL && arrArgument[0] != 0) {  //TAP_BPM(?) means tap to set BPM, only valid for switches, with BPM multiplier ?
                int64_t tap_us = absolute_time_diff_us(S->prevPressedTime, S->currPressedTime);
                if (tap_us > 200000 && tap_us < 2000000) {  //restrict tap BPM between 30bpm and 300bpm to avoid unintended presses affecting BPM
                    MIDI_CLOCK_TIMER_US = (int64_t) (-tap_us * arrArgument[0] / 24);  //Negative delay means we will call repeating_timer_callback, and call it again exactly ???ms later regardless of how long the callback took to execute
                    cancel_repeating_timer(&MIDI_CLOCK_TIMER);  //cancel is safe to do even if not previously activated
                    MIDI_CLOCK_COUNTER = 0;  //reset clock counter so we can start PRESET interval at 1
                    midiClock();  //so fire clock immediately in sync with press (or as close to)
                    add_repeating_timer_us(MIDI_CLOCK_TIMER_US, midiClock, NULL, &MIDI_CLOCK_TIMER);  //new timer will only fire after the first interval
                }
            }
        }
        else if (strcmp(textFunction,"SET_BPM") == 0) {  //24 midi clocks per BPM, takes one argument - if 0, reset clock counter only
            uint8_t lenArgument;
            uint8_t arrArgument[1];  //max is probably 1, we don't need a closing 0x00
            parseCSV(textArgument, arrArgument, &lenArgument);  //parse text into array and length
            if (arrArgument[0] != 0) {  //SET_BPM(?) with a nonzero BPM, set the BPM to the first array value
                MIDI_CLOCK_TIMER_US = (int64_t) (-60000000 / arrArgument[0] / 24);  //Negative delay means we will call repeating_timer_callback, and call it again exactly ???ms later regardless of how long the callback took to execute
                cancel_repeating_timer(&MIDI_CLOCK_TIMER);  //cancel is safe to do even if not previously activated
                add_repeating_timer_us(MIDI_CLOCK_TIMER_US, midiClock, NULL, &MIDI_CLOCK_TIMER);  //new timer will only fire after the first interval
            }
            MIDI_CLOCK_COUNTER = 0;  //reset clock counter so we can start PRESET interval at 1 (the interrupt function pre-increments)
        }
        else if (strcmp(textFunction,"SET_VAR") == 0) {  //set variable, takes two arguments (index, value)
            uint8_t lenArgument;
            uint8_t arrArgument[2];  //max is probably 2, we don't need a closing 0x00
            parseCSV(textArgument, arrArgument, &lenArgument);  //parse text into array and length
            USER_VAR[arrArgument[0]] = arrArgument[1];  //this should be thread/core safe, as we only change the global variable
        }
        else if (strcmp(textFunction,"MSG_USB") == 0) {  //send MIDI message, takes up to 3 arguments
            uint8_t lenArgument;
            uint8_t arrArgument[3];  //max is probably 3, we don't need a closing 0x00
            parseCSV(textArgument, arrArgument, &lenArgument);  //parse text into array and length
            MIDI_USB.send(arrArgument, lenArgument);
        }
        else if (strcmp(textFunction,"MSG_LFT") == 0) {  //send MIDI message, takes up to 3 arguments
            uint8_t lenArgument;
            uint8_t arrArgument[3];  //max is probably 3, we don't need a closing 0x00
            parseCSV(textArgument, arrArgument, &lenArgument);  //parse text into array and length
            MIDI_LEFT.send(arrArgument, lenArgument);
        }
        else if (strcmp(textFunction,"MSG_RGT") == 0) {  //send MIDI message, takes up to 3 arguments
            uint8_t lenArgument;
            uint8_t arrArgument[3];  //max is probably 3, we don't need a closing 0x00
            parseCSV(textArgument, arrArgument, &lenArgument);  //parse text into array and length
            MIDI_RIGHT.send(arrArgument, lenArgument);
        }
        else if (strcmp(textFunction,"SET_PST") == 0) {  //set next preset, takes one argument
            setCurrScreenIndex(textArgument);  //this should be thread/core safe, as we only change the global CURR_SCREEN_INDEX 
        }

    }
}

/*
* Parse the property value text:
* Find the interval inside square brackets []
* Find the function between the interval and argument ] and (
* Find the argument inside round brackets ()
* Parse/run the function with arguments at the correct interval
*/
void parseValue(SWITCH *S, uint64_t *counter, char *valueText) {
    //parsing process
    bool readInterval = false;
    bool readFunction = false;
    bool readArgument = false;
    char textInterval[8];  //max is probably 8 (???,??? + 0x00)
    char textFunction[8];  //max is probably 8 (???_??? + 0x00)
    char textArgument[16];  //max is probably 16 (0x01,0x02,0x03 + 0x00 or ???,???,??? + 0x00 
    uint32_t i=0;  //valueText index - needs to support more than 256 characters
    uint8_t k=0;  //textInterval/textFunctiontextArgument index
    while (valueText[i] != 0x00) {
        if (valueText[i]=='[' && readInterval == false && readFunction == false && readArgument == false) {  //open [ bracket starts interval
            readInterval = true;
            readFunction = false;
            readArgument = false;
            k=0;  //start reading interval
        }
        if (valueText[i]==']' && readInterval == true && readFunction == false && readArgument == false) {  //close ] bracket ends interval
            readInterval = false;
            readFunction = true;
            readArgument = false;
            textInterval[k] = 0x00;  //terminate string
            k=0;  //start reading function
        }
        if (valueText[i]=='(' && readInterval == false && readFunction == true && readArgument == false) {  //open ( bracket starts argument
            readInterval = false;
            readFunction = false;
            readArgument = true;
            textFunction[k] = 0x00;  //terminate string
            k=0;  //start reading argument
        }
        if (valueText[i]==')' && readInterval == false && readFunction == false && readArgument == true) {  //close ) bracket ends argument
            readInterval = false;
            readFunction = false;
            readArgument = false;
            textArgument[k] = 0x00;  //terminate string
            parseFunction(S, counter, textInterval, textFunction, textArgument);  //parse/run interval, function and arguments
        }
        if (valueText[i]!='(' && valueText[i]!=')' && valueText[i]!='[' && valueText[i]!=']' && valueText[i]!=0x10 && valueText[i]!=0x13) {  //ignore argument markers and cr/lf
            if (readInterval == true) {
                textInterval[k] = valueText[i];
                k++;
            }
            if (readFunction == true) {
                textFunction[k] = valueText[i];
                k++;
            }
            if (readArgument == true) {
                textArgument[k] = valueText[i];
                k++;
            }
        }
        i++;
    }
}

/*
* Debounce and check all switches, take appropriate action by parsing the correct property from PRESETS_TEXT
*/
void checkSwitches() {

    SWITCH_LS0.check();
    if (SWITCH_LS0.pressed == true) {  //only do something if a press has been queued
        parseValue(&SWITCH_LS0, &SWITCH_LS0.counter, SCREENS[CURR_SCREEN_INDEX].LS0.PRS);  //parse & run functions in a property value, for specified switch and interval
        SWITCH_LS0.pressed = false;  //reset after consumption
        SNAKE_DIRECTION--;
    }
    if (SWITCH_LS0.releasedShort == true) {  //only do something if a short release has been queued
        parseValue(&SWITCH_LS0, &SWITCH_LS0.counter, SCREENS[CURR_SCREEN_INDEX].LS0.RLS);  //parse & run functions in a property value, for specified switch and interval
        SWITCH_LS0.releasedShort = false;  //reset after consumption
    }
    if (SWITCH_LS0.releasedLong == true) {  //only do something if a long release has been queued
        parseValue(&SWITCH_LS0, &SWITCH_LS0.counter, SCREENS[CURR_SCREEN_INDEX].LS0.RLL);  //parse & run functions in a property value, for specified switch and interval
        SWITCH_LS0.releasedLong = false;  //reset after consumption
    }

    SWITCH_LS1.check();
    if (SWITCH_LS1.pressed == true) {  //only do something if a press has been queued
        parseValue(&SWITCH_LS1, &SWITCH_LS1.counter, SCREENS[CURR_SCREEN_INDEX].LS1.PRS);  //parse & run functions in a property value, for specified switch and interval
        SWITCH_LS1.pressed = false;  //reset after consumption
    }
    if (SWITCH_LS1.releasedShort == true) {  //only do something if a short release has been queued
        parseValue(&SWITCH_LS1, &SWITCH_LS1.counter, SCREENS[CURR_SCREEN_INDEX].LS1.RLS);  //parse & run functions in a property value, for specified switch and interval
        SWITCH_LS1.releasedShort = false;  //reset after consumption
    }
    if (SWITCH_LS1.releasedLong == true) {  //only do something if a long release has been queued
        parseValue(&SWITCH_LS1, &SWITCH_LS1.counter, SCREENS[CURR_SCREEN_INDEX].LS1.RLL);  //parse & run functions in a property value, for specified switch and interval
        SWITCH_LS1.releasedLong = false;  //reset after consumption
    }

    SWITCH_LS2.check();
    if (SWITCH_LS2.pressed == true) {  //only do something if a press has been queued
        parseValue(&SWITCH_LS2, &SWITCH_LS2.counter, SCREENS[CURR_SCREEN_INDEX].LS2.PRS);  //parse & run functions in a property value, for specified switch and interval
        SWITCH_LS2.pressed = false;  //reset after consumption
    }
    if (SWITCH_LS2.releasedShort == true) {  //only do something if a short release has been queued
        parseValue(&SWITCH_LS2, &SWITCH_LS2.counter, SCREENS[CURR_SCREEN_INDEX].LS2.RLS);  //parse & run functions in a property value, for specified switch and interval
        SWITCH_LS2.releasedShort = false;  //reset after consumption
    }
    if (SWITCH_LS2.releasedLong == true) {  //only do something if a long release has been queued
        parseValue(&SWITCH_LS2, &SWITCH_LS2.counter, SCREENS[CURR_SCREEN_INDEX].LS2.RLL);  //parse & run functions in a property value, for specified switch and interval
        SWITCH_LS2.releasedLong = false;  //reset after consumption
    }

    SWITCH_RS0.check();
    if (SWITCH_RS0.pressed == true) {  //only do something if a press has been queued
        parseValue(&SWITCH_RS0, &SWITCH_RS0.counter, SCREENS[CURR_SCREEN_INDEX].RS0.PRS);  //parse & run functions in a property value, for specified switch and interval
        SWITCH_RS0.pressed = false;  //reset after consumption
        SNAKE_DIRECTION++;
    }
    if (SWITCH_RS0.releasedShort == true) {  //only do something if a short release has been queued
        parseValue(&SWITCH_RS0, &SWITCH_RS0.counter, SCREENS[CURR_SCREEN_INDEX].RS0.RLS);  //parse & run functions in a property value, for specified switch and interval
        SWITCH_RS0.releasedShort = false;  //reset after consumption
    }
    if (SWITCH_RS0.releasedLong == true) {  //only do something if a long release has been queued
        parseValue(&SWITCH_RS0, &SWITCH_RS0.counter, SCREENS[CURR_SCREEN_INDEX].RS0.RLL);  //parse & run functions in a property value, for specified switch and interval
        SWITCH_RS0.releasedLong = false;  //reset after consumption
    }

    SWITCH_RS1.check();
    if (SWITCH_RS1.pressed == true) {  //only do something if a press has been queued
        parseValue(&SWITCH_RS1, &SWITCH_RS1.counter, SCREENS[CURR_SCREEN_INDEX].RS1.PRS);  //parse & run functions in a property value, for specified switch and interval
        SWITCH_RS1.pressed = false;  //reset after consumption
    }
    if (SWITCH_RS1.releasedShort == true) {  //only do something if a short release has been queued
        parseValue(&SWITCH_RS1, &SWITCH_RS1.counter, SCREENS[CURR_SCREEN_INDEX].RS1.RLS);  //parse & run functions in a property value, for specified switch and interval
        SWITCH_RS1.releasedShort = false;  //reset after consumption
    }
    if (SWITCH_RS1.releasedLong == true) {  //only do something if a long release has been queued
        parseValue(&SWITCH_RS1, &SWITCH_RS1.counter, SCREENS[CURR_SCREEN_INDEX].RS1.RLL);  //parse & run functions in a property value, for specified switch and interval
        SWITCH_RS1.releasedLong = false;  //reset after consumption
    }

    SWITCH_RS2.check();
    if (SWITCH_RS2.pressed == true) {  //only do something if a press has been queued
        parseValue(&SWITCH_RS2, &SWITCH_RS2.counter, SCREENS[CURR_SCREEN_INDEX].RS2.PRS);  //parse & run functions in a property value, for specified switch and interval
        SWITCH_RS2.pressed = false;  //reset after consumption
    }
    if (SWITCH_RS2.releasedShort == true) {  //only do something if a short release has been queued
        parseValue(&SWITCH_RS2, &SWITCH_RS2.counter, SCREENS[CURR_SCREEN_INDEX].RS2.RLS);  //parse & run functions in a property value, for specified switch and interval
        SWITCH_RS2.releasedShort = false;  //reset after consumption
    }
    if (SWITCH_RS2.releasedLong == true) {  //only do something if a long release has been queued
        parseValue(&SWITCH_RS2, &SWITCH_RS2.counter, SCREENS[CURR_SCREEN_INDEX].RS2.RLL);  //parse & run functions in a property value, for specified switch and interval
        SWITCH_RS2.releasedLong = false;  //reset after consumption
    }

}

/*
* MIDI clock timer, runs 24x per beat.
* With each clock trigger, the appropriate function wll be called at the specified interval as per the current PRESET in PRESETS_TEXT, for attribute PST_CLK
*/
bool midiClock() {
    MIDI_CLOCK_COUNTER++;  //pre-increment same as switches, so we start off with 1 in the PRESET interval
    parseValue(NULL, &MIDI_CLOCK_COUNTER, SCREENS[CURR_SCREEN_INDEX].PST.CLK);  //parse & run functions in a property value, for specified switch and interval
    return true;
}

/*
* Parse/run the functions in the PST_INI property when entering PRESET
* CURR_SCREEN_INDEX should contain the current PRESET's name by the time this is called from S_PRESET (via intercore fifo)
* Interval counter set to 1
*/
void enterPreset() {
    //reset switch interval counters
    SWITCH_LS0.counter = 0;
    SWITCH_LS1.counter = 0;
    SWITCH_LS2.counter = 0;
    SWITCH_RS0.counter = 0;
    SWITCH_RS1.counter = 0;
    SWITCH_RS2.counter = 0;
    uint64_t presetCounter = 1;
    parseValue(NULL, &presetCounter, SCREENS[CURR_SCREEN_INDEX].PST.ENT);  //parse & run functions in a property value, for specified switch and interval
}

/*
* Disable clock timer when exiting PRESET
* Interval counter set to 1
*/
void exitPreset() {
    uint64_t presetCounter = 1;
    parseValue(NULL, &presetCounter, SCREENS[CURR_SCREEN_INDEX].PST.EXT);  //parse & run functions in a property value, for specified switch and interval
}

/*
* Increment/decrement clock BPM and restart timer
*/
void incrementBPM(int64_t inc) {
    int64_t bpm = (int64_t) (-60000000 / 24 / MIDI_CLOCK_TIMER_US);  //24 midi clocks per BPM
    bpm += inc;
    MIDI_CLOCK_TIMER_US = (int64_t) (-60000000 / bpm / 24);  //Negative delay means we will call repeating_timer_callback, and call it again exactly ???ms later regardless of how long the callback took to execute
    cancel_repeating_timer(&MIDI_CLOCK_TIMER);  //cancel is safe to do even if not previously activated
    add_repeating_timer_us(MIDI_CLOCK_TIMER_US, midiClock, NULL, &MIDI_CLOCK_TIMER);  //new timer will only fire after the first interval
}
