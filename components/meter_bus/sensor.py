import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_ID,
    ICON_EMPTY,
    UNIT_WATT,
    UNIT_WATT_HOURS,
    UNIT_CUBIC_METER,
    UNIT_CUBIC_METER_PER_HOUR,
    UNIT_CELSIUS,
    UNIT_KELVIN,
    UNIT_KILOGRAM,
    STATE_CLASS_NONE,
    STATE_CLASS_MEASUREMENT,
    STATE_CLASS_TOTAL,
    DEVICE_CLASS_HEAT,
    DEVICE_CLASS_EMPTY,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_ENERGY,
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_PRESSURE,
    DEVICE_CLASS_VOLUME,
    DEVICE_CLASS_VOLUME_FLOW_RATE,
    DEVICE_CLASS_WEIGHT,
)
from . import meter_bus_ns, MeterBusComponent

DEPENDENCIES = ["meter_bus"]

MeterBusSensor = meter_bus_ns.class_(
    "MeterBusSensor",
    sensor.Sensor,
    cg.PollingComponent,
)

UNIT_JOULE = "J"
UNIT_JOULE_PER_HOUR = "J/h"
UNIT_CUBIC_METER_PER_MINUTE = "m³/min"
UNIT_CUBIC_METER_PER_SECOND = "m³/s"
UNIT_KILOGRAM_PER_HOUR = "kg/h"
UNIT_BAR = "bar"

CONF_METER_BUS_ID = "meter_bus_id"

SENS_ENERGY_WATTHOUR = "energy_watthour"
SENS_ENERGY_JOULE = "energy_joule"
SENS_VOLUME = "volume"
SENS_MASS = "mass"
SENS_POWER_WATT = "power_watt"
SENS_POWER_JOULEPERHOUR = "power_jouleperhour"
SENS_VOLUME_FLOW_HOUR = "volume_flow_hour"
SENS_VOLUME_FLOW_MINUTE = "volume_flow_minute"
SENS_VOLUME_FLOW_SECOND = "volume_flow_second"
SENS_MASS_FLOW = "mass_flow"
SENS_FLOW_TEMP = "flow_temp"
SENS_RETURN_TEMP = "return_temp"
SENS_DELTA_TEMP = "delta_temp"
SENS_EXTERNAL_TEMP = "external_temp"
SENS_PRESSURE = "pressure"

CONFIG_SCHEMA = ( 
    sensor.sensor_schema(MeterBusSensor)
    .extend(
        {
            cv.GenerateID(CONF_METER_BUS_ID): cv.use_id(MeterBusComponent),
            cv.Optional(SENS_ENERGY_WATTHOUR): sensor.sensor_schema(    unit_of_measurement = UNIT_WATT_HOURS,              icon = ICON_EMPTY,      accuracy_decimals = 3,      device_class = DEVICE_CLASS_ENERGY,             state_class = STATE_CLASS_TOTAL),
            cv.Optional(SENS_ENERGY_JOULE): sensor.sensor_schema(       unit_of_measurement = UNIT_JOULE,                   icon = ICON_EMPTY,      accuracy_decimals = 3,      device_class = DEVICE_CLASS_ENERGY,             state_class = STATE_CLASS_TOTAL),
            cv.Optional(SENS_VOLUME): sensor.sensor_schema(             unit_of_measurement = UNIT_CUBIC_METER,             icon = ICON_EMPTY,      accuracy_decimals = 3,      device_class = DEVICE_CLASS_VOLUME,             state_class = STATE_CLASS_TOTAL),
            cv.Optional(SENS_MASS): sensor.sensor_schema(               unit_of_measurement = UNIT_KILOGRAM,                icon = ICON_EMPTY,      accuracy_decimals = 3,      device_class = DEVICE_CLASS_WEIGHT,             state_class = STATE_CLASS_TOTAL),
            cv.Optional(SENS_POWER_WATT): sensor.sensor_schema(         unit_of_measurement = UNIT_WATT,                    icon = ICON_EMPTY,      accuracy_decimals = 3,      device_class = DEVICE_CLASS_POWER,              state_class = STATE_CLASS_MEASUREMENT),
            cv.Optional(SENS_POWER_JOULEPERHOUR): sensor.sensor_schema( unit_of_measurement = UNIT_JOULE_PER_HOUR,          icon = ICON_EMPTY,      accuracy_decimals = 3,      device_class = DEVICE_CLASS_POWER,              state_class = STATE_CLASS_MEASUREMENT),
            cv.Optional(SENS_VOLUME_FLOW_HOUR): sensor.sensor_schema(   unit_of_measurement = UNIT_CUBIC_METER_PER_HOUR,    icon = ICON_EMPTY,      accuracy_decimals = 3,      device_class = DEVICE_CLASS_VOLUME_FLOW_RATE,   state_class = STATE_CLASS_MEASUREMENT),
            cv.Optional(SENS_VOLUME_FLOW_MINUTE): sensor.sensor_schema( unit_of_measurement = UNIT_CUBIC_METER_PER_MINUTE,  icon = ICON_EMPTY,      accuracy_decimals = 3,      device_class = DEVICE_CLASS_VOLUME_FLOW_RATE,   state_class = STATE_CLASS_MEASUREMENT),
            cv.Optional(SENS_VOLUME_FLOW_SECOND): sensor.sensor_schema( unit_of_measurement = UNIT_CUBIC_METER_PER_SECOND,  icon = ICON_EMPTY,      accuracy_decimals = 3,      device_class = DEVICE_CLASS_VOLUME_FLOW_RATE,   state_class = STATE_CLASS_MEASUREMENT),
            cv.Optional(SENS_MASS_FLOW): sensor.sensor_schema(          unit_of_measurement = UNIT_KILOGRAM_PER_HOUR,       icon = ICON_EMPTY,      accuracy_decimals = 3,      device_class = DEVICE_CLASS_EMPTY,              state_class = STATE_CLASS_MEASUREMENT),
            cv.Optional(SENS_FLOW_TEMP): sensor.sensor_schema(          unit_of_measurement = UNIT_CELSIUS,                 icon = ICON_EMPTY,      accuracy_decimals = 3,      device_class = DEVICE_CLASS_TEMPERATURE,        state_class = STATE_CLASS_MEASUREMENT),
            cv.Optional(SENS_RETURN_TEMP): sensor.sensor_schema(        unit_of_measurement = UNIT_CELSIUS,                 icon = ICON_EMPTY,      accuracy_decimals = 3,      device_class = DEVICE_CLASS_TEMPERATURE,        state_class = STATE_CLASS_MEASUREMENT),
            cv.Optional(SENS_DELTA_TEMP): sensor.sensor_schema(         unit_of_measurement = UNIT_KELVIN,                  icon = ICON_EMPTY,      accuracy_decimals = 3,      device_class = DEVICE_CLASS_TEMPERATURE,               state_class = STATE_CLASS_MEASUREMENT),
            cv.Optional(SENS_EXTERNAL_TEMP): sensor.sensor_schema(      unit_of_measurement = UNIT_CELSIUS,                 icon = ICON_EMPTY,      accuracy_decimals = 3,      device_class = DEVICE_CLASS_TEMPERATURE,        state_class = STATE_CLASS_MEASUREMENT),
            cv.Optional(SENS_PRESSURE): sensor.sensor_schema(           unit_of_measurement = UNIT_BAR,                     icon = ICON_EMPTY,      accuracy_decimals = 3,      device_class = DEVICE_CLASS_PRESSURE,           state_class = STATE_CLASS_MEASUREMENT),
        }
    )
    .extend(cv.polling_component_schema("60s"))
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_METER_BUS_ID])

    var = cg.new_Pvariable(config[CONF_ID], parent)
    await sensor.register_sensor(var, config)
    await cg.register_component(var, config)

    if energy_watthour_sensor := config.get(SENS_ENERGY_WATTHOUR):          sens = await sensor.new_sensor(energy_watthour_sensor);     cg.add(var.set_energy_watthour_sensor(sens))
    if energy_joule_sensor := config.get(SENS_ENERGY_JOULE):                sens = await sensor.new_sensor(energy_joule_sensor);        cg.add(var.set_energy_joule_sensor(sens))
    if volume_sensor := config.get(SENS_VOLUME):                            sens = await sensor.new_sensor(volume_sensor);              cg.add(var.set_volume_sensor(sens))
    if mass_sensor := config.get(SENS_MASS):                                sens = await sensor.new_sensor(mass_sensor);                cg.add(var.set_mass_sensor(sens))
    if power_watt_sensor := config.get(SENS_POWER_WATT):                    sens = await sensor.new_sensor(power_watt_sensor);          cg.add(var.set_power_watt_sensor(sens))
    if power_jouleperhour_sensor := config.get(SENS_POWER_JOULEPERHOUR):    sens = await sensor.new_sensor(power_jouleperhour_sensor);  cg.add(var.set_power_jouleperhour_sensor(sens))
    if volume_flow_hour_sensor := config.get(SENS_VOLUME_FLOW_HOUR):        sens = await sensor.new_sensor(volume_flow_hour_sensor);    cg.add(var.set_volume_flow_hour_sensor(sens))
    if volume_flow_minute_sensor := config.get(SENS_VOLUME_FLOW_MINUTE):    sens = await sensor.new_sensor(volume_flow_minute_sensor);  cg.add(var.set_volume_flow_minute_sensor(sens))
    if volume_flow_second_sensor := config.get(SENS_VOLUME_FLOW_SECOND):    sens = await sensor.new_sensor(volume_flow_second_sensor);  cg.add(var.set_volume_flow_second_sensor(sens))
    if mass_flow_sensor := config.get(SENS_MASS_FLOW):                      sens = await sensor.new_sensor(mass_flow_sensor);           cg.add(var.set_mass_flow_sensor(sens))
    if flow_temp_sensor := config.get(SENS_FLOW_TEMP):                      sens = await sensor.new_sensor(flow_temp_sensor);           cg.add(var.set_flow_temp_sensor(sens))
    if return_temp_sensor := config.get(SENS_RETURN_TEMP):                  sens = await sensor.new_sensor(return_temp_sensor);         cg.add(var.set_return_temp_sensor(sens))
    if delta_temp_sensor := config.get(SENS_DELTA_TEMP):                    sens = await sensor.new_sensor(delta_temp_sensor);          cg.add(var.set_delta_temp_sensor(sens))
    if external_temp_sensor := config.get(SENS_EXTERNAL_TEMP):              sens = await sensor.new_sensor(external_temp_sensor);       cg.add(var.set_external_temp_sensor(sens))
    if pressure_sensor := config.get(SENS_PRESSURE):                        sens = await sensor.new_sensor(pressure_sensor);            cg.add(var.set_pressure_sensor(sens))