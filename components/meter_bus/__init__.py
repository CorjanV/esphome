import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import (
    CONF_ID
)

DEPENDENCIES = ["uart"]
AUTO_LOAD = ["sensor"]

CONF_METER_ADDRESS = "meter_address"
CONF_POLLING_INTERVAL = "polling_interval"

meter_bus_ns = cg.esphome_ns.namespace("meter_bus")
MeterBusComponent = meter_bus_ns.class_("MeterBusComponent", cg.Component, uart.UARTDevice)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(MeterBusComponent),
        cv.Required(CONF_METER_ADDRESS): cv.int_range(min=1, max=250),
        cv.Optional(CONF_POLLING_INTERVAL, default=60): cv.int_range(min=10, max=3600),
    }
).extend(cv.COMPONENT_SCHEMA).extend(uart.UART_DEVICE_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    cg.add(var.set_meter_address(config[CONF_METER_ADDRESS]))
    cg.add(var.set_polling_interval(config[CONF_POLLING_INTERVAL]))
