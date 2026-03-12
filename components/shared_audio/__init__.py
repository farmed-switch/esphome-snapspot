CODEOWNERS = ["@farmed-switch"]

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

shared_audio_ns = cg.esphome_ns.namespace("shared_audio")
SharedAudioEQ = shared_audio_ns.class_("SharedAudioEQ", cg.Component)

# shared_audio provides a standalone 18-band graphic EQ as a C API.
# Adding shared_audio: to your YAML instantiates a SharedAudioEQ component.
# ESPHome includes SharedAudioEQ.h in main.cpp which pulls in shared_audio_eq.h,
# making eq_init(), set_eq_band(), apply_eq_preset() and enable_eq() available
# in all lambdas without per-lambda extern declarations.
CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(SharedAudioEQ),
}).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    cg.add_define("USE_SHARED_AUDIO_EQ")
