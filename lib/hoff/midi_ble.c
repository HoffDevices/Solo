/*
* midi_ble.c
* Contains MIDI over Bluetooth LE
* seems like we don't need the batter service per se, but we need it in the adverts
*/
 

static int  le_notification_enabled;
//static btstack_packet_callback_registration_t hci_event_callback_registration;
static hci_con_handle_t con_handle;
//static uint8_t battery = 100;

static void packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
static uint16_t att_read_callback(hci_con_handle_t con_handle, uint16_t att_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size);
static int att_write_callback(hci_con_handle_t con_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
//static void  heartbeat_handler(struct btstack_timer_source *ts);

static char read_buffer[256];
static uint16_t read_buffer_len;

static char write_buffer[256];
static uint16_t write_buffer_len = 0;

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
  *        - send a notification when the requested ATT_EVENT_CAN_SEND_NOW is received
 */
/*
static void packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
    UNUSED(channel);
    UNUSED(size);

    if (packet_type != HCI_EVENT_PACKET) return;
    
    switch (hci_event_packet_get_type(packet)) {
        case HCI_EVENT_DISCONNECTION_COMPLETE:
            le_notification_enabled = 0;
            break;
        case ATT_EVENT_CAN_SEND_NOW:
            //this makes a memcpy of the buffer, and send the notification to the sever
            att_server_notify(con_handle, ATT_CHARACTERISTIC_7772E5DB_3868_4112_A1A9_F2669D106BF3_01_VALUE_HANDLE, (uint8_t*) write_buffer, write_buffer_len);
            write_buffer_len = 0;  //reset buffer len after memcpy
            break;
        default:
            break;
    }
}
*/

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
        return att_read_callback_handle_blob((const uint8_t *)read_buffer, read_buffer_len, offset, buffer, buffer_size);
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
 * We add bytes to write_buffer while we wait until we can send a packet - write_buffer_len keeps incrementing.
 * Then when we can send the packet, it copies the write_buffer and sends the packet via a notification.
 * As soon as write_buffer has been copied, we set write_buffer_len back to 0 and start filling the buffer again.
 * 
 * At high BPM rates we can exceed the ACL_BUFFER, in which case we get a BTSTACK_ACL_BUFFERS_FULL return code.
 * When this happens, we keep on filling the write_buffer and try to keep sending it.
 * If it sends successfully, we reset write_buffer_len to 0, and send small packets again.
 * If we exceed the 250(+5) byte buffer limit we simply discard the MIDI messages. In testing I occasionally used maybe 20 bytes of buffer at 200BPM, so 256 is plenty
 */

static void midiSendBle(uint8_t *msg, uint32_t len) {
    if (le_notification_enabled && write_buffer_len < 240) {  //one 3-byte MIDI messages results in a 5 byte buffer usage.
        //always add timestamp (because MIDI BLE is packet based and the client reassembles things again in the right order)
        uint32_t midi_timestamp = to_ms_since_boot(get_absolute_time()) & 0x01FFF;  //only need the lower 13 bits
        write_buffer[write_buffer_len] = ((midi_timestamp >> 7) & 0x3F) | 0x80;  //The first byte describe the upper 6 bits of the timestamp and has the MSB set.
        write_buffer_len++;  //keep index up to date
        write_buffer[write_buffer_len] = (midi_timestamp & 0x7F) | 0x80;  //The second byte describes the lower 7 bits of the timestamp and also has the MSB set.
        write_buffer_len++;  //keep index up to date
        for (uint8_t i = 0; i < len; i++) {  //copy all incoming MIDI messages to buffer
            write_buffer[write_buffer_len] = msg[i];  //MIDI message byte
            write_buffer_len++;  //keep index up to date
        }
        uint8_t att_status = att_server_notify(con_handle, ATT_CHARACTERISTIC_7772E5DB_3868_4112_A1A9_F2669D106BF3_01_VALUE_HANDLE, (uint8_t*) write_buffer, write_buffer_len);

        //if(att_status == ERROR_CODE_UNKNOWN_CONNECTION_IDENTIFIER) showInfo("Unknown conn",500);
        //if(att_status == BTSTACK_ACL_BUFFERS_FULL) showInfo("Buffers full",500);
        if (att_status == ERROR_CODE_SUCCESS) write_buffer_len = 0;  //reset buffer len after memcpy (att_server_notify)
    }
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
    //Init L2CAP SM ATT Server
    l2cap_init();
    // setup SM: Display only
    sm_init();
    // setup ATT server
    att_server_init(profile_data, att_read_callback, att_write_callback);    
    // setup battery service - this seems to be needed to retain the connection
    //battery_service_server_init(battery);

    // register for HCI events
    //hci_event_callback_registration.callback = &packet_handler;
    //hci_add_event_handler(&hci_event_callback_registration);
    // register for ATT event
    //att_server_register_packet_handler(packet_handler);

    // setup advertisements
    uint16_t adv_int_min = 0x0030;
    uint16_t adv_int_max = 0x0030;
    uint8_t adv_type = 0;
    bd_addr_t null_addr;
    memset(null_addr, 0, 6);
    gap_advertisements_set_params(adv_int_min, adv_int_max, adv_type, 0, null_addr, 0x07, 0x00);
    gap_advertisements_set_data(adv_data_len, (uint8_t*) adv_data);
    gap_advertisements_enable(1);

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
