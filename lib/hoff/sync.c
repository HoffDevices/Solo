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
}

/*
* Send 5V pulse
*/
void pulseSync1() {
    gpio_put(26, 1);  //set high
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
}

/*
* Send 5V pulse
*/
void pulseSync2() {
    gpio_put(27, 1);  //set high
}

/*
* main.h: extern SYNC SYNC_1
*/
SYNC SYNC_2 = (SYNC) {
    .init = initSync2,
    .pulse = pulseSync2
};

