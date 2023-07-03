/*
* midi.c
* Contains MIDI related stuff
*/


/*
* The LEFT port uses uart0, GPIO0, pin 1
*/
void midiInitUart0() {
    uart_init(uart0, 31250);  //initialise uart with midi baudrate
    gpio_set_function(0, GPIO_FUNC_UART);  //set the GPIO pin mux to the UART
}

/*
* Send all characters in array to uart0. Note: msg is an array of bytes, not a string!
*/
void midiSendUart0(uint8_t *msg, uint32_t len) {
    //uint8_t len = sizeof(msg)/sizeof(char);
    for (uint32_t i=0; i<len; i++) uart_putc_raw(uart0, msg[i]);
}

/*
* main.h: extern MIDI MIDI_LEFT
*/
MIDI MIDI_LEFT = (MIDI) {
    .init = midiInitUart0,
    .send = midiSendUart0
};

/*
* The RIGHT port uses uart1, GPIO4, pin 6
*/
void midiInitUart1() {
    uart_init(uart1, 31250);  //initialise uart with midi baudrate
    gpio_set_function(4, GPIO_FUNC_UART);  //set the GPIO pin mux to the UART
}

/*
* Send all characters in array to uart1. Note: msg is an array of bytes, not a string!
*/
void midiSendUart1(uint8_t *msg, uint32_t len) {
    //uint8_t len = sizeof(msg)/sizeof(char);
    for (uint32_t i=0; i<len; i++) uart_putc_raw(uart1, msg[i]);
}

/*
* main.h: extern MIDI MIDI_RIGHT
*/
MIDI MIDI_RIGHT = (MIDI) {
    .init = midiInitUart1,
    .send = midiSendUart1
};

/*
* The USB port uses Tiny USB
*/
void midiInitUsb() {
    board_init();
    tusb_init();
}

/*
* Send all characters in array to uart1. Note: msg is an array of bytes, not a string!
*/
void midiSendUsb(uint8_t *msg, uint32_t len) {
    tud_midi_stream_write(0, msg, len);  //first 0 is MIDI jack associated with USB endpoint
}

/*
* main.h: extern MIDI MIDI_USB
*/
MIDI MIDI_USB = (MIDI) {
    .init = midiInitUsb,
    .send = midiSendUsb
};

/*
* Tiny USB Device callback: Invoked when device is mounted
*/
void tud_mount_cb(void)
{
    TUSB_STATUS = TUSB_MOUNTED;
}

/*
* Tiny USB Device callback: Invoked when device is unmounted
*/
void tud_umount_cb(void)
{
    TUSB_STATUS = TUSB_NOT_MOUNTED;
}

/*
* Tiny USB Device callback: Invoked when usb bus is suspended
* remote_wakeup_en : if host allow us  to perform remote wakeup
* Within 7ms, device must draw an average of current less than 2.5 mA from bus
*/
void tud_suspend_cb(bool remote_wakeup_en)
{
    (void) remote_wakeup_en;
    TUSB_STATUS = TUSB_SUSPENDED;
}

/*
* Tiny USB Device callback: Invoked when usb bus is resumed
*/
void tud_resume_cb(void)
{
    TUSB_STATUS = TUSB_MOUNTED;
}


/*
* The MIDI interface always creates input and output port/jack descriptors regardless of these being used or not.
* Therefore incoming traffic should be read (possibly just discarded) to avoid the sender blocking in IO
*/
void discardMidi() {
    uint8_t packet[4];
    if (tud_task_event_ready() == true) {
        tud_task();  //tinyusb device task
        while ( tud_midi_available() ) {  //read usb midi to prevent buffer overflow
            if (tud_midi_packet_read(packet) == true) {
                //do nothing
            }
        }
    }
}


/*
* WHEN READING PRESETS_TEXT, ALWAYS APPEND 0X00  because client may not be able to do this always..?
* looks like max 3 bytes per packet
* DISABLE INTERRUPTS?
*/
void recvMidiSysex() {
    MIDI_SYSEX_INDEX = 0;  //although this is actually the CONFIG_INDEX
    uint8_t packet[4];
    bool sysexFinished = false;

    while (sysexFinished == false) {  //keep reading tusb
        if (tud_task_event_ready() == true) {
            tud_task();  //tinyusb device task
            while ( tud_midi_available() ) {
                if (tud_midi_packet_read(packet) == true) {
                    //scan for sysex start (0xF0) and sysex finish (0xF7)
                    for (int i=1; i<4; i++) {
                        if (packet[i] == 0xF0) {
                            MIDI_SYSEX_BUSY = true;
                        }
                        else if (packet[i] == 0xF7) {
                            MIDI_SYSEX_BUSY = false;
                            PRESETS_TEXT[MIDI_SYSEX_INDEX] = 0x00;  //string terminator
                            sysexFinished = true;
                        }
                        else if (MIDI_SYSEX_BUSY == true) {
                            PRESETS_TEXT[MIDI_SYSEX_INDEX] = packet[i];
                            MIDI_SYSEX_INDEX++;
                        }
                    }
                }
            }
        }
    }
}

/*
* Send user contents with sysex
*/
void sendMidiSysex() {
    MIDI_SYSEX_INDEX = 0;  //USER_FLASH index - use global se we can show this to the user
    uint8_t k = 0;  //msg index
    uint8_t msg[4] = { 0,0,0,0 };
    uint8_t ctr[1] = { 0 };
    
    ctr[0] = 0xF0;  //sysex start control byte
    MIDI_USB.send(ctr,1);
    while (USER_FLASH[MIDI_SYSEX_INDEX] != 0x00) {
        msg[k] = USER_FLASH[MIDI_SYSEX_INDEX];
        k++;
        MIDI_SYSEX_INDEX++;
        if (k > 3) {  //send in chunks of 4 bytes
            MIDI_USB.send(msg, k);
            busy_wait_ms(20);  //USB doesn't throttle very well so we need a delay
            discardMidi();
            k = 0;
        }
    }
    if (k > 0) MIDI_USB.send(msg, k);  //send remainder if not full 4 bytes
    ctr[0] = 0xF7;  //sysex finish control byte
    MIDI_USB.send(ctr,1);
}







/*
  usbMIDI.sendNoteOn(note, velocity, channel);
  usbMIDI.sendNoteOff(note, velocity, channel);
  usbMIDI.sendAfterTouchPoly(note, pressure, channel);
  usbMIDI.sendControlChange(control, value, channel);
  usbMIDI.sendProgramChange(program, channel);
  usbMIDI.sendAfterTouch(pressure, channel);
  usbMIDI.sendPitchBend(value, channel);
  usbMIDI.sendSysEx(length, array, hasBeginEnd);
  usbMIDI.sendTimeCodeQuarterFrame(index, value);
  usbMIDI.sendSongPosition(beats);
  usbMIDI.sendSongSelect(song);
  usbMIDI.sendTuneRequest();
  usbMIDI.sendRealTime(usbMIDI.Clock);
  usbMIDI.sendRealTime(usbMIDI.Start);
  usbMIDI.sendRealTime(usbMIDI.Continue);
  usbMIDI.sendRealTime(usbMIDI.Stop);
  usbMIDI.sendRealTime(usbMIDI.ActiveSensing);
  usbMIDI.sendRealTime(usbMIDI.SystemReset);


void midiSendRealTimeClock(MIDI device) {
    uint8_t msg[1] = { 0xF8 };
    tud_midi_stream_write(0, msg, 1);  //first 0 is MIDI jack associated with USB endpoint
}

void tusb_midi_sendNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
    uint8_t note_on[3] = { 0x90 | channel, note, velocity };
    tud_midi_stream_write(0, note_on, 3);  //first 0 is MIDI jack associated with USB endpoint
}


void tusb_midi_sendNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
    uint8_t note_off[3] = { 0x80 | channel, note, velocity };
    tud_midi_stream_write(0, note_off, 3);  //first 0 is MIDI jack associated with USB endpoint
}


    busy_wait_ms(5000);
    cancel_repeating_timer(&midiTimer1);
    add_repeating_timer_ms(-400, midiClock1, NULL, &midiTimer1);


void handleSwitches() {
    bool swLeftCurr = true;  //start with button not pressed = high
    bool swLeftPrev = true;  //start with button not pressed = high

    absolute_time_t swLeftDebTime;
    bool busyCheckingLeft = false;

    while (true) {
        swLeftCurr = gpio_get(SWLEFT);
        if (!busyCheckingLeft && swLeftCurr != swLeftPrev) {  //new state change
            swLeftDebTime = delayed_by_ms(get_absolute_time(), 1);  //Return a timestamp value obtained by adding a number of milliseconds to another timestamp - 10ms
            busyCheckingLeft = true;
        }
        if (busyCheckingLeft && absolute_time_diff_us(get_absolute_time(), swLeftDebTime) < 0)  {  //Return the difference in microseconds between two timestamps
            if (swLeftCurr != swLeftPrev) {  //state change is valid
                if (swLeftCurr) SWLEFT_RELEASED++;  //new state is high, so button was released;
                    else SWLEFT_PRESSED++;  //new state is low, so button was pressed
                swLeftPrev = swLeftCurr;  //inherit new state
                busyCheckingLeft = false;  //done checking
            }
        }
    }
}




    */
