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
            if(telegram[index-1] == MBUS_FRAME_STOPSIGN && index > 100) {
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
        ESP_LOGCONFIG(TAG, "CHECKPOINT 1, index=%d", i);
        checksum += telegram[i];
        testcounter++;
        if(DIF) {
            ESP_LOGCONFIG(TAG, "CHECKPOINT 2, index=%d", i);
            if((telegram[i] & 0b10000000) == 0) { // check if DIF extension bit is not set
                if(((telegram[i] & 0x0F) <= 0b00000100) && ((telegram[i] & 0x0F) != 0b00000101)) { // Filter and keep data with 1, 2, 3 or 4 byte integers
                    number_of_data_bytes = telegram[i] & 0x0F;
                    DIF = false;
                    VIF = true;
                    continue;
                } else {
                    ESP_LOGCONFIG(TAG, "Error DIF at index : %d", i);
                    return false; // Length of data can not be defined and will cause errors while parsing the telegram, so return false
                }
            } // else there are DIFE bytes after the DIF byte
        }
        if(VIF) {
            ESP_LOGCONFIG(TAG, "CHECKPOINT 3, index=%d", i);
            if((telegram[i] & 0b10000000) == 0) { // check if VIF extension bit is not set
                ESP_LOGCONFIG(TAG, "CHECKPOINT 4, index=%d", i);
                if((telegram[i] & 0b01111000) == 0b00000000) { // Energy (Wh)
                    ESP_LOGCONFIG(TAG, "CHECKPOINT 5, index=%d", i);
                    ESP_LOGCONFIG(TAG, "Energy (Wh)=%02X", telegram[i]);
                    double multiplyer = telegram[i] & 0b00000111;
                    ESP_LOGCONFIG(TAG, "multiplyer=%d", multiplyer);
                    multiplyer = pow(10, multiplyer-3);
                    int i_data = 0;
                    int helper = 0;
                    for(i_data=0; i_data<number_of_data_bytes; i_data++) {
                        helper |= telegram[1+i+i_data] << i_data*8;
                        checksum += telegram[1+i+i_data]; 
                    }
                    ESP_LOGCONFIG(TAG, "helper=%d", helper);
                    energy_watthour_value = (float)(helper*multiplyer);
                    ESP_LOGCONFIG(TAG, "energy_watthour_value=%.6f", energy_watthour_value);
                    i += number_of_data_bytes;
                    VIF = false;
                    DIF = true;
                    continue;
                }
                else if((telegram[i] & 0b01111000) == 0b00001000) { // Energy (Joule)
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
                else if((telegram[i] & 0b01111000) == 0b00010000) { // Volume (m3)
                    ESP_LOGCONFIG(TAG, "CHECKPOINT 6, index=%d", i);
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
                else if((telegram[i] & 0b01111000) == 0b00011000) { // Mass (kg)
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
                else if((telegram[i] & 0b01111000) == 0b00101000) { // Power (watt)
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
                else if((telegram[i] & 0b01111000) == 0b00110000) { // Power (Joule/h)
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
                else if((telegram[i] & 0b01111000) == 0b00111000) { // Volume Flow (m3/h)
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
                else if((telegram[i] & 0b01111000) == 0b01000000) { // Volume Flow (m3/min)
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
                else if((telegram[i] & 0b01111000) == 0b01001000) { // Volume Flow (m3/s)
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
                else if((telegram[i] & 0b01111000) == 0b01010000) { // Mass Flow (kg/h)
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
                else if((telegram[i] & 0b01111100) == 0b01011000) { // Flow temperature (degrees C)
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
                else if((telegram[i] & 0b01111100) == 0b01011100) { // Return temperature (degrees C)
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
                else if((telegram[i] & 0b01111100) == 0b01100000) { // Temperature difference (degrees K)
                    double multiplyer = telegram[i] & 0b00000011;
                    multiplyer = pow(10, multiplyer-3);
                    int i_data = 0;
                    int helper = 0;
                    for(i_data=0; i_data<number_of_data_bytes; i_data++) {
                        helper |= telegram[1+i+i_data] << i_data*8; 
                        checksum += telegram[1+i+i_data];
                    }
                    delta_temp_value = (float)(helper*multiplyer);
                    i += number_of_data_bytes;
                    VIF = false;
                    DIF = true;
                    continue;
                }
                else if((telegram[i] & 0b01111100) == 0b01100100) { // External temperature (degrees C)
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
                else if((telegram[i] & 0b01111100) == 0b01101000) { // Pressure (Bar)
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
                ESP_LOGCONFIG(TAG, "CHECKPOINT 7, index=%d", i);
                VIF = false;
                VIFE = true;
                continue;
            }
        }
        if(VIFE) {
            ESP_LOGCONFIG(TAG, "CHECKPOINT 8, index=%d", i);
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
    ESP_LOGCONFIG(TAG, "CHECKPOINT 9, index=%d", i);
    if(telegram[frame_length-1] != (checksum & 0xFF)) {
        memset(telegram, 0, sizeof(telegram));
        index = 0;
        // ESP_LOGCONFIG(TAG, "Checksum error (tlgrm vs cs): %02X %02X", telegram[frame_length-1], (checksum & 0xFF));
        return false;
    }
    ESP_LOGCONFIG(TAG, "CHECKPOINT 10, index=%d", i);
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
    if(use_delta_temp_sensor) { this->delta_temp_->publish_state(delta_temp_value); }
    if(use_external_temp_sensor) { this->external_temp_->publish_state(external_temp_value); }
    if(use_pressure_sensor) { this->pressure_->publish_state(pressure_value); }
}

}  // namespace meter_bus
}  // namespace esphome