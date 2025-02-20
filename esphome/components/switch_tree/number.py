import esphome.codegen as cg
from esphome.components import number, switch
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_MIN_VALUE, CONF_MAX_VALUE, CONF_STEP, CONF_RESTORE_VALUE

switch_tree_ns = cg.esphome_ns.namespace("switch_tree")
SwitchTree = switch_tree_ns.class_(
    "SwitchTree", number.Number, cg.Component
)

CONFIG_S1 = "s1_id"
CONFIG_S2 = "s2_id"
CONFIG_S3 = "s3_id"
CONFIG_S4 = "s4_id"
CONFIG_S5 = "s5_id"
CONFIG_S6 = "s6_id"
CONFIG_S7 = "s7_id"

CONFIG_SCHEMA = cv.All(
    cv.Schema({
        cv.Required(CONFIG_S1): cv.use_id(switch.Switch),
        cv.Required(CONFIG_S2): cv.use_id(switch.Switch),
        cv.Required(CONFIG_S3): cv.use_id(switch.Switch),
        cv.Required(CONFIG_S4): cv.use_id(switch.Switch),
        cv.Required(CONFIG_S5): cv.use_id(switch.Switch),
        cv.Required(CONFIG_S6): cv.use_id(switch.Switch),
        cv.Required(CONFIG_S7): cv.use_id(switch.Switch),
        cv.Optional(CONF_RESTORE_VALUE, default=True): cv.boolean,
    })
    .extend(number.number_schema(
        SwitchTree,
    ))
    .extend(cv.COMPONENT_SCHEMA)
)

async def to_code(config):
    s1 = await cg.get_variable(config[CONFIG_S1])
    s2 = await cg.get_variable(config[CONFIG_S2])
    s3 = await cg.get_variable(config[CONFIG_S3])
    s4 = await cg.get_variable(config[CONFIG_S4])
    s5 = await cg.get_variable(config[CONFIG_S5])
    s6 = await cg.get_variable(config[CONFIG_S6])
    s7 = await cg.get_variable(config[CONFIG_S7])

    var = cg.new_Pvariable(
        config[CONF_ID],
        s1,
        s2,
        s3,
        s4,
        s5,
        s6,
        s7,
    )
    await cg.register_component(var, config)
    await number.register_number(var, config, min_value=1, max_value=8, step=1)
    cg.add(var.set_restore_value(config[CONF_RESTORE_VALUE]))
