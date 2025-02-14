#include "esphome/core/log.h"
#include "meter_bus.h"
#include "math.h"

namespace esphome {
namespace meter_bus {

static const char *TAG = "meter_bus.component";

// https://m-bus.com/assets/downloads/MBDOC48.PDF paragraph 5.2 Telegram format
static int MBUS_SHORTFRAME_STARTSIGN = 0x10;
static int MBUS_LONGFRAME_STARTSIGN = 0x68;
static int MBUS_FRAME_STOPSIGN = 0x16;
static int MBUS_REQ_CLASS2_DATA = 0x5B;

int telegram[256] = {0};
int index = 0;
unsigned long current_millis = 0;
unsigned long previous_millis = 0;
bool new_telegram_available = false;
bool data_is_actual = false;
bool realistic_delta_temp_value = false;
int number_of_data_bytes = 0;

float energy_watthour_value = 0;
float energy_joule_value = 0;
float volume_value = 0;
float mass_value = 0;
float power_watt_value = 0;
float power_jouleperhour_value = 0;
float volume_flow_hour_value = 0;
float volume_flow_minute_value = 0;
float volume_flow_second_value = 0;
float mass_flow_value = 0;
float flow_temp_value = 0;
float return_temp_value = 0;
float delta_temp_value = 0;
float external_temp_value = 0;
float pressure_value = 0;

void MeterBusComponent::setup() {

}

void MeterBusComponent::loop() {
    current_millis = esphome::millis();
    if((current_millis - previous_millis) >= (polling_interval_ * 1000)) {
        previous_millis = current_millis;
        mbus_short_frame(meter_address_, MBUS_REQ_CLASS2_DATA); // request data from meter with address meter_address_
    }

    if(!new_telegram_available) {
        if(available()) {
            telegram[index] = read();
            index++;     
            if(telegram[index-1] == MBUS_FRAME_STOPSIGN && index > 100) { // BEUNHAAS
                new_telegram_available = true;
                // ESP_LOGCONFIG(TAG, "stopsign=%02X", MBUS_FRAME_STOPSIGN);
                // ESP_LOGCONFIG(TAG, "telegram[index-1]=%02X", telegram[index-1]);
                // ESP_LOGCONFIG(TAG, "index=%d", (index-1));  
            }    
        }
    }
}

void MeterBusComponent::dump_config(){
    ESP_LOGCONFIG(TAG, "Meter-Bus component");
}

void MeterBusComponent::mbus_short_frame(int address, int C_field) {
    uint8_t data[5];

    data[0] = MBUS_SHORTFRAME_STARTSIGN;
    data[1] = C_field;
    data[2] = address;
    data[3] = data[1]+data[2];
    data[4] = MBUS_FRAME_STOPSIGN;

    write_array(data, sizeof(data));
}

MeterBusSensor::MeterBusSensor(MeterBusComponent *parent) : parent_(parent) {}

void MeterBusSensor::update() {

}

void MeterBusSensor::loop() {
    if(new_telegram_available) {
        new_telegram_available = false;
        if(!mbus_parse_frame(index)) {
            ESP_LOGCONFIG(TAG, "Error while parsing M-Bus telegram");
        }; 
    }
}

void MeterBusSensor::dump_config() {
    ESP_LOGCONFIG(TAG, "Meter-Bus sensor");
}

bool MeterBusSensor::mbus_parse_frame(int frame_length) {
    // ESP_LOGCONFIG(TAG, "framelength=%d", frame_length);
    // ESP_LOGCONFIG(TAG, "telegram[0]=%02X telegram[1]=%02X telegram[2]=%02X, telegram[3]=%02X", telegram[0], telegram[1], telegram[2], telegram[3]);
    // ESP_LOGCONFIG(TAG, "telegram[0]=%02X telegram[1]=%02X telegram[2]=%02X, telegram[3]=%02X, telegram[4]=%02X, telegram[5]=%02X, telegram[6]=%02X, telegram[7]=%02X, telegram[8]=%02X, telegram[9]=%02X", telegram[0], telegram[1], telegram[2], telegram[3], telegram[4], telegram[5], telegram[6], telegram[7], telegram[8], telegram[9]);
    // ESP_LOGCONFIG(TAG, "telegram[10]=%02X telegram[11]=%02X telegram[12]=%02X, telegram[13]=%02X, telegram[14]=%02X, telegram[15]=%02X, telegram[16]=%02X, telegram[17]=%02X, telegram[18]=%02X, telegram[19]=%02X", telegram[10], telegram[11], telegram[12], telegram[13], telegram[14], telegram[15], telegram[16], telegram[17], telegram[18], telegram[19]);
    // ESP_LOGCONFIG(TAG, "telegram[20]=%02X telegram[21]=%02X telegram[22]=%02X, telegram[23]=%02X, telegram[24]=%02X, telegram[25]=%02X, telegram[26]=%02X, telegram[27]=%02X, telegram[28]=%02X, telegram[29]=%02X", telegram[20], telegram[21], telegram[22], telegram[23], telegram[24], telegram[25], telegram[26], telegram[27], telegram[28], telegram[29]);
    // ESP_LOGCONFIG(TAG, "telegram[30]=%02X telegram[31]=%02X telegram[32]=%02X, telegram[33]=%02X, telegram[34]=%02X, telegram[35]=%02X, telegram[36]=%02X, telegram[37]=%02X, telegram[38]=%02X, telegram[39]=%02X", telegram[30], telegram[31], telegram[32], telegram[33], telegram[34], telegram[35], telegram[36], telegram[37], telegram[38], telegram[39]);
    // ESP_LOGCONFIG(TAG, "telegram[40]=%02X telegram[41]=%02X telegram[42]=%02X, telegram[43]=%02X, telegram[44]=%02X, telegram[45]=%02X, telegram[46]=%02X, telegram[47]=%02X, telegram[48]=%02X, telegram[49]=%02X", telegram[40], telegram[41], telegram[42], telegram[43], telegram[44], telegram[45], telegram[46], telegram[47], telegram[48], telegram[49]);
    // ESP_LOGCONFIG(TAG, "telegram[50]=%02X telegram[51]=%02X telegram[52]=%02X, telegram[53]=%02X, telegram[54]=%02X, telegram[55]=%02X, telegram[56]=%02X, telegram[57]=%02X, telegram[58]=%02X, telegram[59]=%02X", telegram[50], telegram[51], telegram[52], telegram[53], telegram[54], telegram[55], telegram[56], telegram[57], telegram[58], telegram[59]);
    // ESP_LOGCONFIG(TAG, "telegram[60]=%02X telegram[61]=%02X telegram[62]=%02X, telegram[63]=%02X, telegram[64]=%02X, telegram[65]=%02X, telegram[66]=%02X, telegram[67]=%02X, telegram[68]=%02X, telegram[69]=%02X", telegram[60], telegram[61], telegram[62], telegram[63], telegram[64], telegram[65], telegram[66], telegram[67], telegram[68], telegram[69]);
    // ESP_LOGCONFIG(TAG, "telegram[70]=%02X telegram[71]=%02X telegram[72]=%02X, telegram[73]=%02X, telegram[74]=%02X, telegram[75]=%02X, telegram[76]=%02X, telegram[77]=%02X, telegram[78]=%02X, telegram[79]=%02X", telegram[70], telegram[71], telegram[72], telegram[73], telegram[74], telegram[75], telegram[76], telegram[77], telegram[78], telegram[79]);
    // ESP_LOGCONFIG(TAG, "telegram[80]=%02X telegram[81]=%02X telegram[82]=%02X, telegram[83]=%02X, telegram[84]=%02X, telegram[85]=%02X, telegram[86]=%02X, telegram[87]=%02X, telegram[88]=%02X, telegram[89]=%02X", telegram[80], telegram[81], telegram[82], telegram[83], telegram[84], telegram[85], telegram[86], telegram[87], telegram[88], telegram[89]);
    // ESP_LOGCONFIG(TAG, "telegram[90]=%02X telegram[91]=%02X telegram[92]=%02X, telegram[93]=%02X, telegram[94]=%02X, telegram[95]=%02X, telegram[96]=%02X, telegram[97]=%02X, telegram[98]=%02X, telegram[99]=%02X", telegram[90], telegram[91], telegram[92], telegram[93], telegram[94], telegram[95], telegram[96], telegram[97], telegram[98], telegram[99]);
    // ESP_LOGCONFIG(TAG, "telegram[100]=%02X telegram[101]=%02X telegram[102]=%02X, telegram[103]=%02X, telegram[104]=%02X, telegram[105]=%02X, telegram[106]=%02X, telegram[107]=%02X, telegram[108]=%02X, telegram[109]=%02X", telegram[100], telegram[101], telegram[102], telegram[103], telegram[104], telegram[105], telegram[106], telegram[107], telegram[108], telegram[109]);
    // ESP_LOGCONFIG(TAG, "telegram[110]=%02X telegram[111]=%02X telegram[112]=%02X, telegram[113]=%02X, telegram[114]=%02X, telegram[115]=%02X, telegram[116]=%02X, telegram[117]=%02X, telegram[118]=%02X, telegram[119]=%02X", telegram[110], telegram[111], telegram[112], telegram[113], telegram[114], telegram[115], telegram[116], telegram[117], telegram[118], telegram[119]);
    // ESP_LOGCONFIG(TAG, "telegram[120]=%02X telegram[121]=%02X telegram[122]=%02X, telegram[123]=%02X, telegram[124]=%02X, telegram[125]=%02X, telegram[126]=%02X, telegram[127]=%02X, telegram[128]=%02X, telegram[129]=%02X", telegram[120], telegram[121], telegram[122], telegram[123], telegram[124], telegram[125], telegram[126], telegram[127], telegram[128], telegram[129]);
    frame_length--;
    uint16_t checksum = 0;
    int i;
    bool DIF = true; // The first byte we go have a look at is expected to be from DIF type
    bool VIF = false;
    bool VIFE = false;
    int testcounter = 0;
    // https://m-bus.com/assets/downloads/MBDOC48.PDF paragraph  6.3  Variable Data Structure

    if(telegram[0] != telegram[3]) {
        memset(telegram, 0, sizeof(telegram));
        index = 0;
        ESP_LOGCONFIG(TAG, "Error with START byte");
        return false; // Both bytes must contain MBUS_LONGFRAME_STARTSIGN
    }
    if(telegram[1] != telegram[2]) {
        memset(telegram, 0, sizeof(telegram));
        index = 0;
        ESP_LOGCONFIG(TAG, "Error with L-Field");
        return false; // Both bytes must contain length of telegram
    }
    if(telegram[6] != 0x72) {
        memset(telegram, 0, sizeof(telegram));
        index = 0;
        ESP_LOGCONFIG(TAG, "Error CI-Field");
        return false; // CI-Field: 0x72 = Variable data respond, Mode 1 (source: paragraph 6.1 CI-Field)
    }

    // paragraph 6.3.1 Fixed Data Header (First 12 bytes of the User Data Block)
    // telegram[7] = ID, Binary Coded Decimal, LSB
    // telegram[8] = ID, Binary Coded Decimal
    // telegram[9] = ID, Binary Coded Decimal
    // telegram[10] = ID, Binary Coded Decimal, MSB
    // telegram[11] = Manufacturer ID
    // telegram[12] = Manufacturer ID
    // telegram[13] = Version
    // telegram[14] = Medium
    // telegram[15] = Acces No.
    // telegram[16] = Status, see paragraph 6.6 Application Layer Status
    // telegram[17] = Signature
    // telegram[18] = Signature
    for(i=4; i<19; i++) {
        checksum += telegram[i];
        testcounter++;
    }
    // ESP_LOGCONFIG(TAG, "testcounter=%d", testcounter);
    for(i=19; i<(frame_length-1); i++) {
        // ESP_LOGCONFIG(TAG, "CHECKPOINT 1, index=%d", i);
        checksum += telegram[i];
        testcounter++;
        if(DIF) {
            // ESP_LOGCONFIG(TAG, "CHECKPOINT 2, index=%d", i);
            if((telegram[i] & 0b10000000) == 0) { // check if DIF extension bit is not set
                if(((telegram[i] & 0x0F) <= 0b00000100) && ((telegram[i] & 0x0F) != 0b00000101)) { // Filter and keep data with 1, 2, 3 or 4 byte integers
                    number_of_data_bytes = telegram[i] & 0x0F;
                    // ESP_LOGCONFIG(TAG, "number_of_data_bytes=%d, index=%d", number_of_data_bytes, i);
                    if((telegram[i] & 0b01000000) == 0) { // check if storage number bit is not set and therefore it is actual data and not historical
                        data_is_actual = true;
                    } else {
                        data_is_actual = false;
                    }
                    DIF = false;
                    VIF = true;
                    continue;
                } else {
                    ESP_LOGCONFIG(TAG, "Error DIF at index : %d", i);
                    return false; // Length of data can not be defined and will cause errors while parsing the telegram, so return false
                }               
            } else { // else there are DIFE bytes after the DIF byte
                ESP_LOGCONFIG(TAG, "Error DIFE at index : %d", i);
                return false; // The code can't handle DIFE bytes at this moment
            }
        }
        if(VIF) {
            // ESP_LOGCONFIG(TAG, "CHECKPOINT 3, index=%d", i);
            if((telegram[i] & 0b10000000) == 0) { // check if VIF extension bit is not set
                // ESP_LOGCONFIG(TAG, "CHECKPOINT 4, index=%d", i);
                if((telegram[i] & 0b01111000) == 0b00000000 && data_is_actual) { // Energy (Wh)
                    // ESP_LOGCONFIG(TAG, "CHECKPOINT 5, index=%d", i);
                    // ESP_LOGCONFIG(TAG, "Energy (Wh)=%02X", telegram[i]);
                    double multiplyer = telegram[i] & 0b00000111;
                    // ESP_LOGCONFIG(TAG, "multiplyer=%d", multiplyer);
                    multiplyer = pow(10, multiplyer-3);
                    int i_data = 0;
                    int helper = 0;
                    for(i_data=0; i_data<number_of_data_bytes; i_data++) {
                        helper |= telegram[1+i+i_data] << i_data*8;
                        checksum += telegram[1+i+i_data]; 
                    }
                    // ESP_LOGCONFIG(TAG, "helper=%d", helper);
                    energy_watthour_value = (float)(helper*multiplyer);
                    // ESP_LOGCONFIG(TAG, "energy_watthour_value=%.6f", energy_watthour_value);
                    i += number_of_data_bytes;
                    VIF = false;
                    DIF = true;
                    continue;
                }
                else if((telegram[i] & 0b01111000) == 0b00001000 && data_is_actual) { // Energy (Joule)
                    double multiplyer = telegram[i] & 0b00000111;
                    multiplyer = pow(10, multiplyer);
                    int i_data = 0;
                    int helper = 0;
                    for(i_data=0; i_data<number_of_data_bytes; i_data++) {
                        helper |= telegram[1+i+i_data] << i_data*8; 
                        checksum += telegram[1+i+i_data];
                    }
                    energy_joule_value = (float)(helper*multiplyer);
                    i += number_of_data_bytes;
                    VIF = false;
                    DIF = true;
                    continue;
                }
                else if((telegram[i] & 0b01111000) == 0b00010000 && data_is_actual) { // Volume (m3)
                    // ESP_LOGCONFIG(TAG, "CHECKPOINT 6, index=%d", i);
                    // ESP_LOGCONFIG(TAG, "Volume (Wh)=%02X", telegram[i]);
                    double multiplyer = telegram[i] & 0b00000111;
                    multiplyer = pow(10, multiplyer-6);
                    int i_data = 0;
                    int helper = 0;
                    for(i_data=0; i_data<number_of_data_bytes; i_data++) {
                        helper |= telegram[1+i+i_data] << i_data*8; 
                        checksum += telegram[1+i+i_data];
                    }
                    volume_value = (float)(helper*multiplyer);
                    i += number_of_data_bytes;
                    VIF = false;
                    DIF = true;
                    continue;
                }
                else if((telegram[i] & 0b01111000) == 0b00011000 && data_is_actual) { // Mass (kg)
                    double multiplyer = telegram[i] & 0b00000111;
                    multiplyer = pow(10, multiplyer-3);
                    int i_data = 0;
                    int helper = 0;
                    for(i_data=0; i_data<number_of_data_bytes; i_data++) {
                        helper |= telegram[1+i+i_data] << i_data*8; 
                        checksum += telegram[1+i+i_data];
                    }
                    mass_value = (float)(helper*multiplyer);
                    i += number_of_data_bytes;
                    VIF = false;
                    DIF = true;
                    continue;
                }
                else if((telegram[i] & 0b01111000) == 0b00101000 && data_is_actual) { // Power (watt)
                    // ESP_LOGCONFIG(TAG, "Power (Wh)=%02X", telegram[i]);
                    double multiplyer = telegram[i] & 0b00000111;
                    multiplyer = pow(10, multiplyer-3);
                    int i_data = 0;
                    int helper = 0;
                    for(i_data=0; i_data<number_of_data_bytes; i_data++) {
                        helper |= telegram[1+i+i_data] << i_data*8; 
                        checksum += telegram[1+i+i_data];
                    }
                    power_watt_value = (float)(helper*multiplyer);
                    i += number_of_data_bytes;
                    VIF = false;
                    DIF = true;
                    continue;
                }
                else if((telegram[i] & 0b01111000) == 0b00110000 && data_is_actual) { // Power (Joule/h)
                    double multiplyer = telegram[i] & 0b00000111;
                    multiplyer = pow(10, multiplyer);
                    int i_data = 0;
                    int helper = 0;
                    for(i_data=0; i_data<number_of_data_bytes; i_data++) {
                        helper |= telegram[1+i+i_data] << i_data*8; 
                        checksum += telegram[1+i+i_data];
                    }
                    power_jouleperhour_value = (float)(helper*multiplyer);
                    i += number_of_data_bytes;
                    VIF = false;
                    DIF = true;
                    continue;
                }
                else if((telegram[i] & 0b01111000) == 0b00111000 && data_is_actual) { // Volume Flow (m3/h)
                    double multiplyer = telegram[i] & 0b00000111;
                    multiplyer = pow(10, multiplyer-6);
                    int i_data = 0;
                    int helper = 0;
                    for(i_data=0; i_data<number_of_data_bytes; i_data++) {
                        helper |= telegram[1+i+i_data] << i_data*8; 
                        checksum += telegram[1+i+i_data];
                    }
                    volume_flow_hour_value = (float)(helper*multiplyer);
                    i += number_of_data_bytes;
                    VIF = false;
                    DIF = true;
                    continue;
                }
                else if((telegram[i] & 0b01111000) == 0b01000000 && data_is_actual) { // Volume Flow (m3/min)
                    double multiplyer = telegram[i] & 0b00000111;
                    multiplyer = pow(10, multiplyer-7);
                    int i_data = 0;
                    int helper = 0;
                    for(i_data=0; i_data<number_of_data_bytes; i_data++) {
                        helper |= telegram[1+i+i_data] << i_data*8; 
                        checksum += telegram[1+i+i_data];
                    }
                    volume_flow_minute_value = (float)(helper*multiplyer);
                    i += number_of_data_bytes;
                    VIF = false;
                    DIF = true;
                    continue;
                }
                else if((telegram[i] & 0b01111000) == 0b01001000 && data_is_actual) { // Volume Flow (m3/s)
                    double multiplyer = telegram[i] & 0b00000111;
                    multiplyer = pow(10, multiplyer-9);
                    int i_data = 0;
                    int helper = 0;
                    for(i_data=0; i_data<number_of_data_bytes; i_data++) {
                        helper |= telegram[1+i+i_data] << i_data*8; 
                        checksum += telegram[1+i+i_data];
                    }
                    volume_flow_second_value = (float)(helper*multiplyer);
                    i += number_of_data_bytes;
                    VIF = false;
                    DIF = true;
                    continue;
                }
                else if((telegram[i] & 0b01111000) == 0b01010000 && data_is_actual) { // Mass Flow (kg/h)
                    double multiplyer = telegram[i] & 0b00000111;
                    multiplyer = pow(10, multiplyer-3);
                    int i_data = 0;
                    int helper = 0;
                    for(i_data=0; i_data<number_of_data_bytes; i_data++) {
                        helper |= telegram[1+i+i_data] << i_data*8; 
                        checksum += telegram[1+i+i_data];
                    }
                    mass_flow_value = (float)(helper*multiplyer);
                    i += number_of_data_bytes;
                    VIF = false;
                    DIF = true;
                    continue;
                }
                else if((telegram[i] & 0b01111100) == 0b01011000 && data_is_actual) { // Flow temperature (degrees C)
                    double multiplyer = telegram[i] & 0b00000011;
                    multiplyer = pow(10, multiplyer-3);
                    int i_data = 0;
                    int helper = 0;
                    for(i_data=0; i_data<number_of_data_bytes; i_data++) {
                        helper |= telegram[1+i+i_data] << i_data*8; 
                        checksum += telegram[1+i+i_data];
                    }
                    flow_temp_value = (float)(helper*multiplyer);
                    i += number_of_data_bytes;
                    VIF = false;
                    DIF = true;
                    continue;
                }
                else if((telegram[i] & 0b01111100) == 0b01011100 && data_is_actual) { // Return temperature (degrees C)
                    double multiplyer = telegram[i] & 0b00000011;
                    multiplyer = pow(10, multiplyer-3);
                    int i_data = 0;
                    int helper = 0;
                    for(i_data=0; i_data<number_of_data_bytes; i_data++) {
                        helper |= telegram[1+i+i_data] << i_data*8; 
                        checksum += telegram[1+i+i_data];
                    }
                    return_temp_value = (float)(helper*multiplyer);
                    i += number_of_data_bytes;
                    VIF = false;
                    DIF = true;
                    continue;
                }
                else if((telegram[i] & 0b01111100) == 0b01100000 && data_is_actual) { // Temperature difference (degrees K)
                    double multiplyer = telegram[i] & 0b00000011;
                    multiplyer = pow(10, multiplyer-3);
                    int i_data = 0;
                    int helper = 0;
                    for(i_data=0; i_data<number_of_data_bytes; i_data++) {
                        helper |= telegram[1+i+i_data] << i_data*8; 
                        checksum += telegram[1+i+i_data];
                    }
                    delta_temp_value = (float)(helper*multiplyer);
                    if(delta_temp_value < 20) {
                        realistic_delta_temp_value = true;
                    } else {
                        realistic_delta_temp_value = false;
                    }
                    i += number_of_data_bytes;
                    VIF = false;
                    DIF = true;
                    continue;
                }
                else if((telegram[i] & 0b01111100) == 0b01100100 && data_is_actual) { // External temperature (degrees C)
                    double multiplyer = telegram[i] & 0b00000011;
                    multiplyer = pow(10, multiplyer-3);
                    int i_data = 0;
                    int helper = 0;
                    for(i_data=0; i_data<number_of_data_bytes; i_data++) {
                        helper |= telegram[1+i+i_data] << i_data*8; 
                        checksum += telegram[1+i+i_data];
                    }
                    external_temp_value = (float)(helper*multiplyer);
                    i += number_of_data_bytes;
                    VIF = false;
                    DIF = true;
                    continue;
                }
                else if((telegram[i] & 0b01111100) == 0b01101000 && data_is_actual) { // Pressure (Bar)
                    double multiplyer = telegram[i] & 0b00000011;
                    multiplyer = pow(10, multiplyer-3);
                    int i_data = 0;
                    int helper = 0;
                    for(i_data=0; i_data<number_of_data_bytes; i_data++) {
                        helper |= telegram[1+i+i_data] << i_data*8; 
                        checksum += telegram[1+i+i_data];
                    }
                    pressure_value = (float)(helper*multiplyer);
                    i += number_of_data_bytes;
                    VIF = false;
                    DIF = true;
                    continue;
                }
                else {
                    int i_data = 0;
                    for(i_data=0; i_data<number_of_data_bytes; i_data++) {
                        checksum += telegram[1+i+i_data];
                    }
                    i += number_of_data_bytes;
                    VIF = false;
                    DIF = true;
                    continue;
                }
            } else { // Detected VIFE
                // ESP_LOGCONFIG(TAG, "CHECKPOINT 7, index=%d", i);
                VIF = false;
                VIFE = true;
                continue;
            }
        }
        if(VIFE) {
            // ESP_LOGCONFIG(TAG, "CHECKPOINT 8, index=%d", i);
            int i_data = 0;
            for(i_data=0; i_data<number_of_data_bytes; i_data++) {
                checksum += telegram[1+i+i_data];
            }
            i += number_of_data_bytes;
            VIFE = false;
            DIF = true;
            continue;
        }
    }
    // ESP_LOGCONFIG(TAG, "CHECKPOINT 9, index=%d", i);
    if(telegram[frame_length-1] != (checksum & 0xFF)) {
        memset(telegram, 0, sizeof(telegram));
        index = 0;
        // ESP_LOGCONFIG(TAG, "Checksum error (tlgrm vs cs): %02X %02X", telegram[frame_length-1], (checksum & 0xFF));
        return false;
    }
    // ESP_LOGCONFIG(TAG, "CHECKPOINT 10, index=%d", i);
    publish_sensor_data();
    memset(telegram, 0, sizeof(telegram));
    index = 0;
    return true;
}

void MeterBusSensor::publish_sensor_data() {
    if(use_energy_watthour_sensor) { this->energy_watthour_->publish_state(energy_watthour_value); }
    if(use_energy_joule_sensor) { this->energy_joule_->publish_state(energy_joule_value); }
    if(use_volume_sensor) { this->volume_->publish_state(volume_value); }
    if(use_mass_sensor) { this->mass_->publish_state(mass_value); }
    if(use_power_watt_sensor) { this->power_watt_->publish_state(power_watt_value); }
    if(use_power_jouleperhour_sensor) { this->power_jouleperhour_->publish_state(power_jouleperhour_value); }
    if(use_volume_flow_hour_sensor) { this->volume_flow_hour_->publish_state(volume_flow_hour_value); }
    if(use_volume_flow_minute_sensor) { this->volume_flow_minute_->publish_state(volume_flow_minute_value); }
    if(use_volume_flow_second_sensor) { this->volume_flow_second_->publish_state(volume_flow_second_value); }
    if(use_mass_flow_sensor) { this->mass_flow_->publish_state(mass_flow_value); }
    if(use_flow_temp_sensor) { this->flow_temp_->publish_state(flow_temp_value); }
    if(use_return_temp_sensor) { this->return_temp_->publish_state(return_temp_value); }
    if(use_delta_temp_sensor && realistic_delta_temp_value) { this->delta_temp_->publish_state(delta_temp_value); }
    if(use_external_temp_sensor) { this->external_temp_->publish_state(external_temp_value); }
    if(use_pressure_sensor) { this->pressure_->publish_state(pressure_value); }
}

}  // namespace meter_bus
}  // namespace esphome