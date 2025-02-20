import esphome.codegen as cg
from esphome.components import binary_sensor, ble_client, script
import esphome.config_validation as cv
from esphome.const import CONF_ID

telink_ns = cg.esphome_ns.namespace("telink")
Telink = telink_ns.class_(
    "Telink", binary_sensor.BinarySensor, cg.Component
)

CONF_BLE_ID = "ble_id"
CONF_SEND_COMMAND_ID = "send_command_id"
CONF_MAC = "mac"
CONF_TELINK_NAME = "telink_name"

CONFIG_SCHEMA = cv.All(
    cv.Schema({
        cv.Required(CONF_BLE_ID): cv.use_id(ble_client.BLEClient),
        cv.Required(CONF_SEND_COMMAND_ID): cv.use_id(script.Script),
        cv.Required(CONF_MAC): cv.string,
        cv.Required(CONF_TELINK_NAME): cv.string,
    })
    .extend(binary_sensor.binary_sensor_schema(
        Telink,
    ))
    .extend(cv.COMPONENT_SCHEMA)
)

async def to_code(config):
    ble_dev = await cg.get_variable(config[CONF_BLE_ID])
    send_command = await cg.get_variable(config[CONF_SEND_COMMAND_ID])
    var = cg.new_Pvariable(
        config[CONF_ID],
        ble_dev,
        send_command,
        config[CONF_MAC],
        config[CONF_TELINK_NAME]
    )
    await cg.register_component(var, config)
    await binary_sensor.register_binary_sensor(var, config)
