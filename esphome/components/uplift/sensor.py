import esphome.codegen as cg
from esphome.components import sensor, uart
import esphome.config_validation as cv
from esphome.const import CONF_ID, UNIT_MILLIMETER, STATE_CLASS_MEASUREMENT

uplift_ns = cg.esphome_ns.namespace("uplift")
Uplift = uplift_ns.class_(
    "Uplift", sensor.Sensor, cg.Component, uart.UARTDevice
)

CONFIG_SCHEMA = cv.All(
    sensor.sensor_schema(
        Uplift,
        unit_of_measurement=UNIT_MILLIMETER,
        accuracy_decimals=0,
        state_class=STATE_CLASS_MEASUREMENT,
    )
    .extend(uart.UART_DEVICE_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await sensor.register_sensor(var, config)
    await uart.register_uart_device(var, config)
