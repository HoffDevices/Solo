/*
* sync.c
* Contains 5V sync pulse related stuff. We are using a level converter circuit to change the 3V output to 5V.
*/


/*
* Sync 1 uses GPIO26, pin 31
*/
void initSync1() {
    gpio_init(26);  //(enabled I/O and set func to GPIO_FUNC_SIO)
    gpio_set_dir(26, GPIO_OUT);  //true for out, false for in
    gpio_put(26, 0);  //set low by default
}

/*
* Callback will be called once alarm is triggered
* Set level, do not reschedule alarm
*/
int64_t alarmCallbackSync1(alarm_id_t id, void *level) {
    gpio_put(26, level);  //set level
    return 0;  //do not reschedule
}

/*
* Set level, and duration in ms
* If duration is not 0, set alarm to reset level after duration
*/
void pulseSync1(uint8_t level, int64_t duration) {
    gpio_put(26, level);  //set level
    if (duration != 0) {
        if (level == 0) SYNC_1_ALARM_ID = alarm_pool_add_alarm_in_ms(SYNC_ALARM_POOL, duration, alarmCallbackSync1, (void *) 1, false);  //invert level
        else SYNC_1_ALARM_ID = alarm_pool_add_alarm_in_ms(SYNC_ALARM_POOL, duration, alarmCallbackSync1, (void *) 0, false);
    }
}

/*
* main.h: extern SYNC SYNC_1
*/
SYNC SYNC_1 = (SYNC) {
    .init = initSync1,
    .pulse = pulseSync1
};

/*
* Sync 2 uses GPIO27, pin 32
*/
void initSync2() {
    gpio_init(27);  //(enabled I/O and set func to GPIO_FUNC_SIO)
    gpio_set_dir(27, GPIO_OUT);  //true for out, false for in
    gpio_put(27, 0);  //set low by default
}

/*
* Callback will be called once alarm is triggered
* Set level, do not reschedule alarm
*/
int64_t alarmCallbackSync2(alarm_id_t id, void *level) {
    gpio_put(27, level);  //set level
    return 0;  //do not reschedule
}

/*
* Set level, and duration in ms
* If duration is not 0, set alarm to reset level after duration
*/
void pulseSync2(uint8_t level, int64_t duration) {
    gpio_put(27, level);  //set level
    if (duration != 0) {
        if (level == 0) SYNC_2_ALARM_ID = alarm_pool_add_alarm_in_ms(SYNC_ALARM_POOL, duration, alarmCallbackSync2, (void *) 1, false);  //invert level
        else SYNC_2_ALARM_ID = alarm_pool_add_alarm_in_ms(SYNC_ALARM_POOL, duration, alarmCallbackSync2, (void *) 0, false);
    }
}

/*
* main.h: extern SYNC SYNC_1
*/
SYNC SYNC_2 = (SYNC) {
    .init = initSync2,
    .pulse = pulseSync2
};

