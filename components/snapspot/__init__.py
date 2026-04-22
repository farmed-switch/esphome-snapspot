import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID, CONF_NAME, CONF_DISABLED_BY_DEFAULT,
    CONF_ENTITY_CATEGORY, CONF_RESTORE_MODE, CONF_MODE,
    CONF_AUDIO_DAC, ENTITY_CATEGORY_CONFIG,
)
from esphome.core import CORE, ID
from esphome.components.esp32 import add_idf_sdkconfig_option, get_esp32_variant
from esphome.components import number, switch, select, audio_dac
from esphome.components.switch import SwitchRestoreMode

CODEOWNERS = ["@farmed-switch"]
DEPENDENCIES = ["esp32"]
AUTO_LOAD = ["number", "switch", "select"]

CONF_MIXER_ID = "mixer_id"

mixer_speaker_ns = cg.esphome_ns.namespace("mixer_speaker")
MixerSpeaker = mixer_speaker_ns.class_("MixerSpeaker", cg.Component)

shared_audio_ns = cg.esphome_ns.namespace("shared_audio")
SharedAudioEQ = shared_audio_ns.class_("SharedAudioEQ", cg.Component)
EQBandNumber = shared_audio_ns.class_(
    "EQBandNumber", number.Number, cg.Component
)
EQEnableSwitch = shared_audio_ns.class_(
    "EQEnableSwitch", switch.Switch, cg.Component
)
EQPresetSelect = shared_audio_ns.class_(
    "EQPresetSelect", select.Select, cg.Component
)

snapspot_ns = cg.esphome_ns.namespace("snapspot")
DacManager = snapspot_ns.class_("DacManager", cg.Component)

KNOWN_CLIENTS = {
    "spotify_connect": 0,
    "snapclient":      1,
    "squeezelite":     2,
    "sendspin":        3,
}

_source_speaker_ids: dict = {}

ISO_BANDS = [
    "---25Hz", "---40Hz", "---63Hz", "--100Hz", "--160Hz", "--250Hz",
    "--400Hz", "--630Hz", "-1000Hz", "-1600Hz", "-2500Hz", "-4000Hz",
    "-6300Hz", "10000Hz", "16000Hz",
]

EQ_PRESETS = [
    "Flat", "Bass Boost", "Treble Boost",
    "Loudness", "Vocal Clarity", "Small Speaker", "Night Mode", "Custom",
]

async def get_source_speaker(client_type: str):
    ""
    if client_type not in _source_speaker_ids:
        raise cv.Invalid(
            f"snapspot: no slot mapped for client '{client_type}'. "
            "Check that the mixer has enough source_speakers entries."
        )
    return await cg.get_variable(_source_speaker_ids[client_type])

_EQ_ENTITY_COUNT = 18

CONF_SOFTWARE_EQ = "software_eq"

def _reserve_eq_component_slots(config):
    ""
    if config.get(CONF_SOFTWARE_EQ, True):
        for i in range(_EQ_ENTITY_COUNT):
            CORE.component_ids.add(f"__snapspot_eq_slot_{i}")
    if CONF_AUDIO_DAC in config:
        CORE.component_ids.add("__snapspot_dac_manager")
    return config

CONFIG_SCHEMA = cv.All(
    cv.Schema({
        cv.Required(CONF_MIXER_ID): cv.use_id(MixerSpeaker),
        cv.Optional(CONF_SOFTWARE_EQ, default=False): cv.boolean,
        cv.Optional(CONF_AUDIO_DAC): cv.use_id(audio_dac.AudioDac),
    }),
    _reserve_eq_component_slots,
)

def _detect_active_clients() -> list:
    ""
    found = []
    for mp in CORE.config.get("media_player", []):
        if isinstance(mp, dict):
            platform = mp.get("platform")
            if platform == "spotify_connect" and "spotify_connect" not in found:
                found.append("spotify_connect")
            elif platform == "snapclient" and "snapclient" not in found:
                found.append("snapclient")

    if "snapclient" in CORE.config and "snapclient" not in found:
        found.append("snapclient")
    for known in ("squeezelite", "sendspin"):
        if known in CORE.config:
            found.append(known)
    return sorted(found, key=lambda c: KNOWN_CLIENTS.get(c, 99))

def _source_speakers_for_mixer(mixer_id_str: str) -> list:
    ""
    for spkr_cfg in CORE.config.get("speaker", []):
        if not isinstance(spkr_cfg, dict):
            continue
        if str(spkr_cfg.get("id", "")) == mixer_id_str:
            return [s["id"] for s in spkr_cfg.get("source_speakers", [])]
    return []

async def _create_eq_entities():
    ""
    from esphome.components.number import NumberMode

    parent_id = ID("shared_audio_eq", is_declaration=True, type=SharedAudioEQ)
    parent = cg.new_Pvariable(parent_id)
    cg.add(cg.App.register_component_(parent))

    sw_id = ID("sw_eq_switch", is_declaration=True, type=EQEnableSwitch)
    sw_config = {
        CONF_ID: sw_id,
        CONF_NAME: "Software EQ",
        CONF_DISABLED_BY_DEFAULT: False,
        CONF_ENTITY_CATEGORY: ENTITY_CATEGORY_CONFIG,
        CONF_RESTORE_MODE: SwitchRestoreMode.SWITCH_RESTORE_DEFAULT_OFF,
    }
    sw_var = await switch.new_switch(sw_config)
    cg.add(cg.App.register_component_(sw_var))
    cg.add(sw_var.set_eq_parent(parent))
    cg.add(parent.set_eq_switch(sw_var))

    sel_id = ID("eq_preset", is_declaration=True, type=EQPresetSelect)
    sel_config = {
        CONF_ID: sel_id,
        CONF_NAME: "EQ Preset",
        CONF_DISABLED_BY_DEFAULT: False,
        CONF_ENTITY_CATEGORY: ENTITY_CATEGORY_CONFIG,
    }
    sel_var = await select.new_select(sel_config, options=EQ_PRESETS)
    cg.add(cg.App.register_component_(sel_var))
    cg.add(sel_var.set_eq_parent(parent))
    cg.add(parent.set_eq_preset(sel_var))

    for i, label in enumerate(ISO_BANDS):
        band_id = ID(f"eq_band_{i}", is_declaration=True, type=EQBandNumber)
        num_config = {
            CONF_ID: band_id,
            CONF_NAME: f"EQ {label}",
            CONF_DISABLED_BY_DEFAULT: False,
            CONF_ENTITY_CATEGORY: ENTITY_CATEGORY_CONFIG,
            CONF_MODE: NumberMode.NUMBER_MODE_SLIDER,
        }
        num_var = await number.new_number(
            num_config, min_value=-15.0, max_value=15.0, step=0.5
        )
        cg.add(cg.App.register_component_(num_var))
        cg.add(num_var.set_band_index(i))
        cg.add(num_var.set_eq_parent(parent))
        cg.add(parent.add_band_number(num_var))

async def to_code(config):
    global _source_speaker_ids
    _source_speaker_ids = {}

    cg.add_define("USE_SHARED_AUDIO_EQ")

    add_idf_sdkconfig_option("CONFIG_SPIRAM_USE_MALLOC", True)
    add_idf_sdkconfig_option("CONFIG_SPIRAM_USE_CAPS_ALLOC", False)

    add_idf_sdkconfig_option("CONFIG_SPIRAM_MALLOC_ALWAYSINTERNAL", 16384)
    add_idf_sdkconfig_option("CONFIG_SPIRAM_ALLOW_BSS_SEG_EXTERNAL_MEMORY", True)

    add_idf_sdkconfig_option("CONFIG_SPIRAM_MALLOC_RESERVE_INTERNAL", 49152)
    add_idf_sdkconfig_option("CONFIG_SPIRAM_ALLOW_STACK_EXTERNAL_MEMORY", True)

    _is_s3 = str(get_esp32_variant()).lower().replace("-", "") == "esp32s3"
    add_idf_sdkconfig_option("CONFIG_SPIRAM_TRY_ALLOCATE_WIFI_LWIP", _is_s3)

    add_idf_sdkconfig_option("CONFIG_ESP_WIFI_RX_BA_WIN", 16)
    add_idf_sdkconfig_option("CONFIG_ESP_WIFI_TX_BA_WIN", 16)

    add_idf_sdkconfig_option("CONFIG_COMPILER_CXX_EXCEPTIONS", True)
    add_idf_sdkconfig_option("CONFIG_COMPILER_CXX_EXCEPTIONS_EMG_POOL_SIZE", 1024)

    add_idf_sdkconfig_option("CONFIG_MBEDTLS_PSK_MODES", True)
    add_idf_sdkconfig_option("CONFIG_MBEDTLS_KEY_EXCHANGE_PSK", True)
    add_idf_sdkconfig_option("CONFIG_MBEDTLS_SSL_IN_CONTENT_LEN", 16384)
    add_idf_sdkconfig_option("CONFIG_MBEDTLS_SSL_OUT_CONTENT_LEN", 4096)
    add_idf_sdkconfig_option("CONFIG_MBEDTLS_ASYMMETRIC_CONTENT_LEN", True)
    add_idf_sdkconfig_option("CONFIG_MBEDTLS_CERTIFICATE_BUNDLE", True)
    add_idf_sdkconfig_option("CONFIG_MBEDTLS_CERTIFICATE_BUNDLE_DEFAULT_FULL", True)
    add_idf_sdkconfig_option("CONFIG_MBEDTLS_EXTERNAL_MEM_ALLOC", True)
    add_idf_sdkconfig_option("CONFIG_MBEDTLS_DYNAMIC_BUFFER", True)

    add_idf_sdkconfig_option("CONFIG_LWIP_TCP_MAXRTX", 6)
    add_idf_sdkconfig_option("CONFIG_LWIP_TCP_MSL", 10000)
    add_idf_sdkconfig_option("CONFIG_LWIP_TCP_SYNMAXRTX", 3)
    add_idf_sdkconfig_option("CONFIG_LWIP_SO_RCVBUF", True)
    add_idf_sdkconfig_option("CONFIG_LWIP_TCP_WND_DEFAULT", 65535)
    add_idf_sdkconfig_option("CONFIG_LWIP_MAX_SOCKETS", 16)
    add_idf_sdkconfig_option("CONFIG_LWIP_MAX_ACTIVE_TCP", 16)
    add_idf_sdkconfig_option("CONFIG_LWIP_MAX_LISTENING_TCP", 8)
    add_idf_sdkconfig_option("CONFIG_LWIP_SO_REUSE", True)
    add_idf_sdkconfig_option("CONFIG_LWIP_TCP_RECVMBOX_SIZE", 12)

    add_idf_sdkconfig_option("CONFIG_ESP_WIFI_DYNAMIC_RX_BUFFER_NUM", 64)
    add_idf_sdkconfig_option("CONFIG_ESP_WIFI_STATIC_RX_BUFFER_NUM", 16)
    add_idf_sdkconfig_option("CONFIG_ESP_WIFI_IRAM_OPT", True)
    add_idf_sdkconfig_option("CONFIG_ESP_WIFI_RX_IRAM_OPT", True)

    add_idf_sdkconfig_option("CONFIG_FREERTOS_HZ", 1000)
    add_idf_sdkconfig_option("CONFIG_FREERTOS_TASK_NOTIFICATION_ARRAY_ENTRIES", 2)
    add_idf_sdkconfig_option("CONFIG_FREERTOS_PLACE_FUNCTIONS_INTO_FLASH", False)

    add_idf_sdkconfig_option("CONFIG_HTTPD_MAX_URI_HANDLERS", 24)

    add_idf_sdkconfig_option("CONFIG_ESP_MAIN_TASK_STACK_SIZE", 3584)

    add_idf_sdkconfig_option("CONFIG_COMPILER_OPTIMIZATION_PERF", True)

    add_idf_sdkconfig_option("CONFIG_NEWLIB_NANO_FORMAT", False)

    add_idf_sdkconfig_option("CONFIG_BELL_CODEC_OPUS", False)
    add_idf_sdkconfig_option("CONFIG_SPIRAM_BOOT_INIT", True)

    add_idf_sdkconfig_option("CONFIG_USE_SAMPLE_INSERTION", True)

    add_idf_sdkconfig_option("CONFIG_BT_ENABLED", False)

    add_idf_sdkconfig_option("CONFIG_ESP32_WIFI_AMPDU_TX_ENABLED", True)
    add_idf_sdkconfig_option("CONFIG_ESP32_WIFI_AMPDU_RX_ENABLED", True)

    add_idf_sdkconfig_option("CONFIG_LWIP_IRAM_OPTIMIZATION", True)

    add_idf_sdkconfig_option("CONFIG_LWIP_TCPIP_CORE_LOCKING", True)

    if str(get_esp32_variant()).lower().replace("-", "") == "esp32s3":
        add_idf_sdkconfig_option("CONFIG_ESP32S3_DATA_CACHE_64KB", True)
        add_idf_sdkconfig_option("CONFIG_ESP32S3_DATA_CACHE_LINE_64B", True)

        add_idf_sdkconfig_option("CONFIG_SPIRAM_FETCH_INSTRUCTIONS", True)

    active = _detect_active_clients()
    if not active:
        raise cv.Invalid(
            "snapspot: no recognized audio clients found in config "
            "(expected snapclient, spotify_connect, squeezelite, or sendspin)."
        )

    slots = _source_speakers_for_mixer(str(config[CONF_MIXER_ID]))
    if len(slots) < len(active):
        raise cv.Invalid(
            f"snapspot: mixer has {len(slots)} source_speaker(s) but "
            f"{len(active)} client(s) are active ({', '.join(active)}). "
            "Add more source_speakers to the mixer declaration."
        )

    for client_type, slot_id in zip(active, slots):
        _source_speaker_ids[client_type] = slot_id

    if config.get(CONF_SOFTWARE_EQ, True):
        await _create_eq_entities()

    if CONF_AUDIO_DAC in config:
        dac_mgr_id = ID("snapspot_dac_manager", is_declaration=True, type=DacManager)
        dac_mgr = cg.new_Pvariable(dac_mgr_id)
        cg.add(cg.App.register_component_(dac_mgr))
        mixer_var = await cg.get_variable(config[CONF_MIXER_ID])
        cg.add(dac_mgr.set_output_speaker(mixer_var.get_output_speaker()))
        dac_var = await cg.get_variable(config[CONF_AUDIO_DAC])
        cg.add(dac_mgr.set_audio_dac(dac_var))
