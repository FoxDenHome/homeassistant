import esphome.codegen as cg
from esphome.components import switch
import esphome.config_validation as cv
from esphome.const import CONF_ID

latching_relay_ns = cg.esphome_ns.namespace("latching_relay")
LatchingRelay = latching_relay_ns.class_(
    "LatchingRelay", switch.Switch, cg.Component
)

CONFIG_SET_PIN = "set_pin"
CONFIG_RESET_PIN = "reset_pin"
CONFIG_IS_ASSUMED_STATE = "is_assumed_state"

CONFIG_SCHEMA = cv.All(
    cv.Schema({
        cv.Required(CONFIG_SET_PIN): cv.int_,
        cv.Required(CONFIG_RESET_PIN): cv.int_,
        cv.Optional(CONFIG_IS_ASSUMED_STATE, default=True): cv.boolean,
    })
    .extend(switch.switch_schema(
        LatchingRelay,
    ))
    .extend(cv.COMPONENT_SCHEMA)
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID], config[CONFIG_SET_PIN], config[CONFIG_RESET_PIN], config[CONFIG_IS_ASSUMED_STATE])
    await cg.register_component(var, config)
    await switch.register_switch(var, config)
