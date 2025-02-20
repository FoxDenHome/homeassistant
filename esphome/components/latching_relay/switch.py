import esphome.codegen as cg
from esphome.components import switch
import esphome.config_validation as cv
from esphome.const import CONF_ID

latching_relay_ns = cg.esphome_ns.namespace("latching_relay")
LatchingRelay = latching_relay_ns.class_(
    "LatchingRelay", switch.Switch, cg.Component
)

CONF_SET_PIN = "set_pin"
CONF_RESET_PIN = "reset_pin"
CONF_IS_ASSUMED_STATE = "is_assumed_state"

CONFIG_SCHEMA = cv.All(
    cv.Schema({
        cv.Required(CONF_SET_PIN): cv.int_,
        cv.Required(CONF_RESET_PIN): cv.int_,
        cv.Optional(CONF_IS_ASSUMED_STATE, default=True): cv.boolean,
    })
    .extend(switch.switch_schema(
        LatchingRelay,
    ))
    .extend(cv.COMPONENT_SCHEMA)
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID], config[CONF_SET_PIN], config[CONF_RESET_PIN], config[CONF_IS_ASSUMED_STATE])
    await cg.register_component(var, config)
    await switch.register_switch(var, config)
