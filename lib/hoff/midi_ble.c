/*
* midi_ble.c
* Contains MIDI over Bluetooth LE
*/
 

static int  le_notification_enabled;
static btstack_packet_callback_registration_t hci_event_callback_registration;
static hci_con_handle_t con_handle;
static uint8_t battery = 100;

static void packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
static uint16_t att_read_callback(hci_con_handle_t con_handle, uint16_t att_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size);
static int att_write_callback(hci_con_handle_t con_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
static void  heartbeat_handler(struct btstack_timer_source *ts);

static char midi_ble_string[5];
static int midi_ble_string_len;


/*
https://docs.silabs.com/bluetooth/4.0/general/adv-and-scanning/bluetooth-adv-data-basics

Advertising data consists of one or more Advertising Data (AD) elements. Each element is formatted as follows:

1st byte: length of the element (excluding the length byte itself)
2nd byte: AD type – specifies what data is included in the element
AD data – one or more bytes - the meaning is defined by AD type

the maximum length i.e., 31 bytes

see bluetooth_data_types.h for constants used below

MIDI BLE SERVICE
03B80E5A-EDE8-4B33-A751-6CE34EC4C700
back-words version:
0x00, 0xc7, 0xc4, 0x4e, 0xe3, 0x6c, 0x51, 0xa7, 0x33, 0x4b, 0xe8, 0xed, 0x5a, 0x0e, 0xb8, 0x03, 

// Flags general discoverable, BR/EDR supported (== not supported flag not set) when ENABLE_GATT_OVER_CLASSIC is enabled
#define APP_AD_FLAGS 0x06
*/

const uint8_t adv_data[] = {
    // Flags general discoverable
    0x02, BLUETOOTH_DATA_TYPE_FLAGS, 0x06,
    // Name - length is 9 characters for "HoffSolo" plus the AD type byte
    0x09, BLUETOOTH_DATA_TYPE_SHORTENED_LOCAL_NAME, 'H', 'o', 'f', 'f', 'S', 'o', 'l', 'o', 
    // MIDI BLUE UUID -- note that the byte order is reversed, length is 16 bytes + AD type byte
    0x11, BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_128_BIT_SERVICE_CLASS_UUIDS, 0x00, 0xc7, 0xc4, 0x4e, 0xe3, 0x6c, 0x51, 0xa7, 0x33, 0x4b, 0xe8, 0xed, 0x5a, 0x0e, 0xb8, 0x03, 
};
const uint8_t adv_data_len = sizeof(adv_data);


/* 
 * @text The packet handler is used to:
 *        - stop the counter after a disconnect
 *        - send a notification when the requested ATT_EVENT_CAN_SEND_NOW is received
 */

static void packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
    UNUSED(channel);
    UNUSED(size);

    if (packet_type != HCI_EVENT_PACKET) return;
    
    switch (hci_event_packet_get_type(packet)) {
        case HCI_EVENT_DISCONNECTION_COMPLETE:
            le_notification_enabled = 0;
            break;
        case ATT_EVENT_CAN_SEND_NOW:
            att_server_notify(con_handle, ATT_CHARACTERISTIC_7772E5DB_3868_4112_A1A9_F2669D106BF3_01_VALUE_HANDLE, (uint8_t*) midi_ble_string, midi_ble_string_len);
            break;
        default:
            break;
    }
}


/*
 * @text The ATT Server handles all reads to constant data. For dynamic data like the custom characteristic, the registered
 * att_read_callback is called. To handle long characteristics and long reads, the att_read_callback is first called
 * with buffer == NULL, to request the total value length. Then it will be called again requesting a chunk of the value.
 * See Listing attRead.
 */

// ATT Client Read Callback for Dynamic Data
// - if buffer == NULL, don't copy data, just return size of value
// - if buffer != NULL, copy data and return number bytes copied
// @param offset defines start of attribute value

static uint16_t att_read_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size){
    UNUSED(connection_handle);

    if (att_handle == ATT_CHARACTERISTIC_7772E5DB_3868_4112_A1A9_F2669D106BF3_01_VALUE_HANDLE){
        return att_read_callback_handle_blob((const uint8_t *)midi_ble_string, midi_ble_string_len, offset, buffer, buffer_size);
    }
    return 0;
}


/*
 * @text The only valid ATT write in this example is to the Client Characteristic Configuration, which configures notification
 * and indication. If the ATT handle matches the client configuration handle, the new configuration value is stored and used
 * in the heartbeat handler to decide if a new value should be sent. See Listing attWrite.
 */

static int att_write_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size){
    UNUSED(transaction_mode);
    UNUSED(offset);
    UNUSED(buffer_size);
    
    if (att_handle != ATT_CHARACTERISTIC_7772E5DB_3868_4112_A1A9_F2669D106BF3_01_CLIENT_CONFIGURATION_HANDLE) return 0;
    le_notification_enabled = little_endian_read_16(buffer, 0) == GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION;
    con_handle = connection_handle;
    return 0;
}


/*
 * @text The handler updates the value of the single Characteristic provided in this example,
 * and request a ATT_EVENT_CAN_SEND_NOW to send a notification if enabled.
 * 
 * supposed to be thread safe, even if called from core1?
 * 
 */

static void midiSendBle(uint8_t *msg, uint32_t len) {
    if (le_notification_enabled) {
        midi_ble_string_len = 5;
        midi_ble_string[0] = 0x80;  //The first byte describe the upper 6 bits of the timestamp and has the MSB set.
        midi_ble_string[1] = 0x80;  //The second byte describes the lower 7 bits of the timestamp and also has the MSB set.
        midi_ble_string[2] = msg[0];  //MIDI status & channel: (0x90 = note ON, 0x80 = note off), channel 0
        midi_ble_string[3] = msg[1];  //MIDI data byte 1: Note number
        midi_ble_string[4] = msg[2];  //MIDI data byte 2: Note value
        puts(midi_ble_string);
        att_server_request_can_send_now_event(con_handle);
    }
    //keeping this around for future use, since we need the battery service regardless
    //battery_service_server_set_battery_value(battery);
}


/*
 * @text Listing MainConfiguration shows main application code.
 * It initializes L2CAP, the Security Manager and configures the ATT Server with the pre-compiled
 * ATT Database generated from $le_counter.gatt$. 
 * Additionally, it enables the Battery Service Server with the current battery level.
 * Finally, it configures the advertisements 
 * and the heartbeat handler and boots the Bluetooth stack. 
 * In this example, the Advertisement contains the Flags attribute and the device name.
 * The flag 0x06 indicates: LE General Discoverable Mode and BR/EDR not supported.
 */

void midiInitBle() {
    // initialize CYW43 driver architecture (will enable BT if/because CYW43_ENABLE_BLUETOOTH == 1)
    if (cyw43_arch_init()) return; //return -1;  //failed to initialise cyw43_arch
    // inform about BTstack state
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);
    //Init L2CAP SM ATT Server and start heartbeat timer
    l2cap_init();
    // setup SM: Display only
    sm_init();
    // setup ATT server
    att_server_init(profile_data, att_read_callback, att_write_callback);    
    // setup battery service - this seems to be needed to retain the connection
    battery_service_server_init(battery);

    // setup advertisements
    uint16_t adv_int_min = 0x0030;
    uint16_t adv_int_max = 0x0030;
    uint8_t adv_type = 0;
    bd_addr_t null_addr;
    memset(null_addr, 0, 6);
    gap_advertisements_set_params(adv_int_min, adv_int_max, adv_type, 0, null_addr, 0x07, 0x00);
    gap_advertisements_set_data(adv_data_len, (uint8_t*) adv_data);
    gap_advertisements_enable(1);

    // register for HCI events
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);
    // register for ATT event
    att_server_register_packet_handler(packet_handler);

    // turn on!
	hci_power_control(HCI_POWER_ON);
}

/*
* main.h: extern MIDI MIDI_RIGHT
*/
MIDI MIDI_BLE= (MIDI) {
    .init = midiInitBle,
    .send = midiSendBle
};
