CODEOWNERS = ["@farmed-switch"]
DEPENDENCIES = ["esp32"]

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

priority_lock_ns = cg.esphome_ns.namespace("priority_lock")
PriorityLockManager = priority_lock_ns.class_("PriorityLockManager", cg.Component)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(PriorityLockManager),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
