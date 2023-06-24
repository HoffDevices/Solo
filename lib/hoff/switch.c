/*
* switch.c
* Contains switch related stuff
*/


/*
* Manages the debounce and tracking of presses and releases for a switch
*/
void checkSwitch(SWITCH *s) {
    if (s->debounceWait == false) {  //not waiting to debounce, we can accept new state change
        s->currState = s->state();
        if (s->currState != s->prevState) {  //assume first state change is valid, else we introduce latency while waiting for debounce
            if (s->currState == true) {  //new state is high, so button was released
                if (absolute_time_diff_us(s->currPressedTime, get_absolute_time()) > 1000000) s->releasedLong = true; //released after more than 1s = long release
                else s->releasedShort = true;  //else short release
                s->prevReleasedTime  = s->currReleasedTime;
                s->currReleasedTime = get_absolute_time();
            }
            else {  //new state is low, so button was pressed
                s->pressed = true;  //new state is low, so button was pressed
                s->prevPressedTime = s->currPressedTime;
                s->currPressedTime = get_absolute_time();
                s->counter++;  //keep track of number of presses for interval matching on PRESET
                s->timedBusy = false;  //we need to manually prevent recurrance of timedOut being set true if held down indefinitely
            }
            s->prevState = s->currState;  //inherit new state
            s->debounceWait = true;
        }
    }
    else {  //wait until debounce is complete before we check for state change again
        if (s->currState == true && absolute_time_diff_us(s->currReleasedTime, get_absolute_time()) > s->debounceTime ||  //currState == true => Released
            s->currState == false && absolute_time_diff_us(s->currPressedTime, get_absolute_time()) > s->debounceTime) { //Return the difference in microseconds between two timestamps
            s->debounceWait = false;
        }
    }
    //  timeout after long press
    if (absolute_time_diff_us(s->currPressedTime, s->currReleasedTime) > 0  //pressed but not released yet
        && s->timedBusy == false  //we haven't fired this event yet
        && absolute_time_diff_us(s->currPressedTime, get_absolute_time()) > 1000000) {  //held down for more than 1s = timed out
        s->timedOut = true;  //to be consumed by a button
        s->timedBusy = true;  //to prevent this event being again only after the next press
    }
}

/*
* SCREEN TOUCH
* main.h: extern SWITCH SWITCH_LS0 
*/
void initSwitchTouch() {
    TP_Init(SCAN_DIR_DFT);  //init touchpad
}
void checkSwitchTouch() {
    checkSwitch(&SWITCH_TOUCH);
}
bool stateSwitchTouch() {
    TP_DRAW touch = TP_Touch();  //get touch status
    SWITCH_TOUCH.x = touch.Xpoint;
    SWITCH_TOUCH.y = touch.Ypoint;
    return (touch.Color != RED);  //hack to use LCD_Touch code
}
SWITCH SWITCH_TOUCH = (SWITCH) {
    .x = 0,
    .y = 0,
    .debounceTime = 10000,  //debounce time in us
    .debounceWait = false,  //start state check immediately
    .currState = true,  //start with button not pressed = high
    .prevState = true,  //start with button not pressed = high
    .pressed = false,  //pressed
    .releasedShort = false,  //releasedShort
    .releasedLong = false,  //releasedLong
    .timedOut = false,  //timeout auto event trigger
    .timedBusy = false,  //timeout auto event management
    .counter = 0,  //interval counter
    .init = initSwitchTouch,
    .check = checkSwitchTouch,
    .state = stateSwitchTouch
};

/*
* Sets up the GPIO for a switch
*/
void initSwitch(SWITCH *s) {
    gpio_init(s->gpio);  //(enabled I/O and set func to GPIO_FUNC_SIO)
    gpio_set_dir(s->gpio, GPIO_IN);  //true for out, false for in
    gpio_pull_up(s->gpio);  //enable pull-up, pressed = LOW/FALSE
}

/*
* The LEFT switch uses GPIO2, pin 4
* main.h: extern SWITCH SWITCH_LS0 
*/
void initSwitchLS0() {
    initSwitch(&SWITCH_LS0);
}
void checkSwitchLS0() {
    checkSwitch(&SWITCH_LS0);
}
bool stateSwitchLS0() {
    return gpio_get(SWITCH_LS0.gpio);
}
SWITCH SWITCH_LS0 = (SWITCH) {
    .gpio = 1,  //GPIO1, pin 2
    .debounceTime = 100000,  //debounce time in us
    .debounceWait = false,  //start state check immediately
    .currState = true,  //start with button not pressed = high
    .prevState = true,  //start with button not pressed = high
    .pressed = false,  //pressed
    .releasedShort = false,  //releasedShort
    .releasedLong = false,  //releasedLong
    .timedOut = false,  //timeout auto event trigger
    .timedBusy = false,  //timeout auto event management
    .counter = 0,  //interval counter
    .init = initSwitchLS0,
    .check = checkSwitchLS0,
    .state = stateSwitchLS0
};

/*
* The LEFT tip uses GPIO26, pin 31
* main.h: extern SWITCH SWITCH_LS1 
*/
void initSwitchLS1() {
    initSwitch(&SWITCH_LS1);
}
void checkSwitchLS1() {
    checkSwitch(&SWITCH_LS1);
}
bool stateSwitchLS1() {
    return gpio_get(SWITCH_LS1.gpio);
}
SWITCH SWITCH_LS1 = (SWITCH) {
    .gpio = 2,  //GPIO2, pin 4
    .debounceTime = 100000,  //debounce time in us
    .debounceWait = false,  //start state check immediately
    .currState = true,  //start with button not pressed = high
    .prevState = true,  //start with button not pressed = high
    .pressed = false,  //pressed
    .releasedShort = false,  //releasedShort
    .releasedLong = false,  //releasedLong
    .timedOut = false,  //timeout auto event trigger
    .timedBusy = false,  //timeout auto event management
    .counter = 0,  //interval counter
    .init = initSwitchLS1,
    .check = checkSwitchLS1,
    .state = stateSwitchLS1
};

/*
* The LEFT ring uses GPIO7, pin 10
* main.h: extern SWITCH SWITCH_LS2 
*/
void initSwitchLS2() {
    initSwitch(&SWITCH_LS2);
}
void checkSwitchLS2() {
    checkSwitch(&SWITCH_LS2);
}
bool stateSwitchLS2() {
    return gpio_get(SWITCH_LS2.gpio);
}
SWITCH SWITCH_LS2 = (SWITCH) {
    .gpio = 3,  //GPIO3, pin 5
    .debounceTime = 100000,  //debounce time in us
    .debounceWait = false,  //start state check immediately
    .currState = true,  //start with button not pressed = high
    .prevState = true,  //start with button not pressed = high
    .pressed = false,  //pressed
    .releasedShort = false,  //releasedShort
    .releasedLong = false,  //releasedLong
    .timedOut = false,  //timeout auto event trigger
    .timedBusy = false,  //timeout auto event management
    .counter = 0,  //interval counter
    .init = initSwitchLS2,
    .check = checkSwitchLS2,
    .state = stateSwitchLS2
};

/*
* The RIGHT switch uses GPIO3, pin 5
* main.h: extern SWITCH SWITCH_RS0 
*/
void initSwitchRS0() {
    initSwitch(&SWITCH_RS0);
}
void checkSwitchRS0() {
    checkSwitch(&SWITCH_RS0);
}
bool stateSwitchRS0() {
    return gpio_get(SWITCH_RS0.gpio);
}
SWITCH SWITCH_RS0 = (SWITCH) {
    .gpio = 14,  //GPIO14, pin 19
    .debounceTime = 100000,  //debounce time in us
    .debounceWait = false,  //start state check immediately
    .currState = true,  //start with button not pressed = high
    .prevState = true,  //start with button not pressed = high
    .pressed = false,  //pressed
    .releasedShort = false,  //releasedShort
    .releasedLong = false,  //releasedLong
    .timedOut = false,  //timeout auto event trigger
    .timedBusy = false,  //timeout auto event management
    .counter = 0,  //interval counter
    .init = initSwitchRS0,
    .check = checkSwitchRS0,
    .state = stateSwitchRS0
};

/*
* The RIGHT tip uses GPIO27, pin 32
* main.h: extern SWITCH SWITCH_RS1 
*/
void initSwitchRS1() {
    initSwitch(&SWITCH_RS1);
}
void checkSwitchRS1() {
    checkSwitch(&SWITCH_RS1);
}
bool stateSwitchRS1() {
    return gpio_get(SWITCH_RS1.gpio);
}
SWITCH SWITCH_RS1 = (SWITCH) {
    .gpio = 6,  //GPIO6, pin 9
    .debounceTime = 100000,  //debounce time in us
    .debounceWait = false,  //start state check immediately
    .currState = true,  //start with button not pressed = high
    .prevState = true,  //start with button not pressed = high
    .pressed = false,  //pressed
    .releasedShort = false,  //releasedShort
    .releasedLong = false,  //releasedLong
    .timedOut = false,  //timeout auto event trigger
    .timedBusy = false,  //timeout auto event management
    .counter = 0,  //interval counter
    .init = initSwitchRS1,
    .check = checkSwitchRS1,
    .state = stateSwitchRS1
};

/*
* The RIGHT ring uses GPIO6, pin 9
* main.h: extern SWITCH SWITCH_RS2 
*/
void initSwitchRS2() {
    initSwitch(&SWITCH_RS2);
}
void checkSwitchRS2() {
    checkSwitch(&SWITCH_RS2);
}
bool stateSwitchRS2() {
    return gpio_get(SWITCH_RS2.gpio);
}
SWITCH SWITCH_RS2 = (SWITCH) {
    .gpio = 7,  //GPIO7, pin 10
    .debounceTime = 100000,  //debounce time in us
    .debounceWait = false,  //start state check immediately
    .currState = true,  //start with button not pressed = high
    .prevState = true,  //start with button not pressed = high
    .pressed = false,  //pressed
    .releasedShort = false,  //releasedShort
    .releasedLong = false,  //releasedLong
    .timedOut = false,  //timeout auto event trigger
    .timedBusy = false,  //timeout auto event management
    .counter = 0,  //interval counter
    .init = initSwitchRS2,
    .check = checkSwitchRS2,
    .state = stateSwitchRS2
};
