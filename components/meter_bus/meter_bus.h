#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace meter_bus {

class MeterBusComponent : public uart::UARTDevice, public Component {
    public:
        void setup() override;
        void loop() override;
        void dump_config() override;
        void set_meter_address(int meter_address) { this->meter_address_ = meter_address; }
        void set_polling_interval(int polling_interval) { this->polling_interval_ = polling_interval; }

    protected:
        void mbus_short_frame(int address, int C_field);

    private:
        int meter_address_;
        int polling_interval_;
};

class MeterBusSensor : public sensor::Sensor, public PollingComponent {
    public:
        MeterBusSensor(MeterBusComponent *parent);
        void update() override;
        void loop() override;
        void dump_config() override;
        void set_energy_watthour_sensor(sensor::Sensor *sens) { energy_watthour_ = sens; use_energy_watthour_sensor = true; }
        void set_energy_joule_sensor(sensor::Sensor *sens) { energy_joule_ = sens; use_energy_joule_sensor = true; }
        void set_volume_sensor(sensor::Sensor *sens) { volume_ = sens; use_volume_sensor = true; }
        void set_mass_sensor(sensor::Sensor *sens) { mass_ = sens; use_mass_sensor = true; }
        void set_power_watt_sensor(sensor::Sensor *sens) { power_watt_ = sens; use_power_watt_sensor = true; }
        void set_power_jouleperhour_sensor(sensor::Sensor *sens) { power_jouleperhour_ = sens; use_power_jouleperhour_sensor = true; }
        void set_volume_flow_hour_sensor(sensor::Sensor *sens) { volume_flow_hour_ = sens; use_volume_flow_hour_sensor = true; }
        void set_volume_flow_minute_sensor(sensor::Sensor *sens) { volume_flow_minute_ = sens; use_volume_flow_minute_sensor = true; }
        void set_volume_flow_second_sensor(sensor::Sensor *sens) { volume_flow_second_ = sens; use_volume_flow_second_sensor = true; }
        void set_mass_flow_sensor(sensor::Sensor *sens) { mass_flow_ = sens; use_mass_flow_sensor = true; }
        void set_flow_temp_sensor(sensor::Sensor *sens) { flow_temp_ = sens; use_flow_temp_sensor = true; }
        void set_return_temp_sensor(sensor::Sensor *sens) { return_temp_ = sens; use_return_temp_sensor = true; }
        void set_delta_temp_sensor(sensor::Sensor *sens) { delta_temp_ = sens; use_delta_temp_sensor = true; }
        void set_external_temp_sensor(sensor::Sensor *sens) { external_temp_ = sens; use_external_temp_sensor = true; }
        void set_pressure_sensor(sensor::Sensor *sens) { pressure_ = sens; use_pressure_sensor = true; }

    protected:
        MeterBusComponent *parent_;
        bool mbus_parse_frame(int frame_length);
        void publish_sensor_data();
        sensor::Sensor *energy_watthour_;
        sensor::Sensor *energy_joule_;
        sensor::Sensor *volume_;
        sensor::Sensor *mass_;
        sensor::Sensor *power_watt_;
        sensor::Sensor *power_jouleperhour_;
        sensor::Sensor *volume_flow_hour_;
        sensor::Sensor *volume_flow_minute_;
        sensor::Sensor *volume_flow_second_;
        sensor::Sensor *mass_flow_;
        sensor::Sensor *flow_temp_;
        sensor::Sensor *return_temp_;
        sensor::Sensor *delta_temp_;
        sensor::Sensor *external_temp_;
        sensor::Sensor *pressure_;

    private:
        bool use_energy_watthour_sensor = false;
        bool use_energy_joule_sensor = false;
        bool use_volume_sensor = false;
        bool use_mass_sensor = false;
        bool use_power_watt_sensor = false;
        bool use_power_jouleperhour_sensor = false;
        bool use_volume_flow_hour_sensor = false;
        bool use_volume_flow_minute_sensor = false;
        bool use_volume_flow_second_sensor = false;
        bool use_mass_flow_sensor = false;
        bool use_flow_temp_sensor = false;
        bool use_return_temp_sensor = false;
        bool use_delta_temp_sensor = false;
        bool use_external_temp_sensor = false;
        bool use_pressure_sensor = false;

};

}  // namespace meter_bus
}  // namespace esphome