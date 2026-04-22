from esphome import pins
import esphome.codegen as cg
from esphome.components import audio_dac, media_player, text_sensor, sensor
from esphome.components.esp32 import add_idf_component, add_idf_sdkconfig_option
import esphome.config_validation as cv
from esphome.const import CONF_AUDIO_DAC, CONF_ID, CONF_NAME, CONF_PORT, ENTITY_CATEGORY_DIAGNOSTIC
from esphome.core import CORE
CODEOWNERS = ["@farmed-switch"]

DEPENDENCIES = ["esp32", "snapspot"]
AUTO_LOAD = ["text_sensor", "sensor"]

CONF_HOSTNAME = "hostname"
CONF_MUTE_PIN = "mute_pin"
CONF_WEBSERVER_PORT = "webserver_port"
CONF_SNAPCAST_MIN_DB = "snapcast_min_db"
CONF_SNAPCAST_MAX_DB = "snapcast_max_db"
CONF_MIN_VOLUME = "min_volume"
CONF_MAX_VOLUME = "max_volume"

CONF_TRACK_NAME = "track_name"
CONF_ARTIST = "artist"
CONF_ALBUM = "album"
CONF_ALBUM_ART_URL = "album_art_url"
CONF_DURATION = "duration"
CONF_POSITION = "position"

CONF_SYNC_AGE = "sync_age"
CONF_SHORT_MEDIAN = "short_median"
CONF_MINI_MEDIAN = "mini_median"
CONF_QUEUE_DEPTH = "queue_depth"
CONF_HEAP_FREE = "heap_free"
CONF_PSRAM_FREE = "psram_free"
CONF_DECODER_RECV = "decoder_recv"
CONF_DECODER_DECODED = "decoder_decoded"
CONF_DECODER_DROP = "decoder_drop"
CONF_DRIFT = "drift"

SNAPCLIENT_GIT_VERSION = "5cda3a75ed97572868a4303e35ffcf88244b6105"

snapclient_ns = cg.esphome_ns.namespace("snapclient")

SnapClientComponent = snapclient_ns.class_(
    "SnapClientComponent", cg.Component, media_player.MediaPlayer
)

CONFIG_SCHEMA = cv.All(
    media_player.media_player_schema(SnapClientComponent)
    .extend(
        {
            cv.GenerateID(): cv.declare_id(SnapClientComponent),

            cv.Optional(CONF_NAME): cv.string,

            cv.Optional(CONF_HOSTNAME): cv.domain,

            cv.Optional(CONF_PORT, default=1704): cv.port,

            cv.Optional(CONF_MUTE_PIN): pins.gpio_output_pin_schema,

            cv.Optional(CONF_AUDIO_DAC): cv.use_id(audio_dac.AudioDac),

            cv.Optional(CONF_SNAPCAST_MIN_DB, default=-45.0): cv.float_,
            cv.Optional(CONF_SNAPCAST_MAX_DB, default=0.0): cv.float_,

            cv.Optional(CONF_MIN_VOLUME, default=0): cv.int_range(0, 100),
            cv.Optional(CONF_MAX_VOLUME, default=100): cv.int_range(0, 100),

            cv.Optional(CONF_TRACK_NAME): text_sensor.text_sensor_schema(),
            cv.Optional(CONF_ARTIST): text_sensor.text_sensor_schema(),
            cv.Optional(CONF_ALBUM): text_sensor.text_sensor_schema(),
            cv.Optional(CONF_ALBUM_ART_URL): text_sensor.text_sensor_schema(),

            cv.Optional(CONF_DURATION): sensor.sensor_schema(
                unit_of_measurement="s",
                accuracy_decimals=0,
            ),
            cv.Optional(CONF_POSITION): sensor.sensor_schema(
                unit_of_measurement="s",
                accuracy_decimals=0,
            ),

            cv.Optional(CONF_SYNC_AGE): sensor.sensor_schema(
                unit_of_measurement="\u00b5s",
                accuracy_decimals=0,
            ),
            cv.Optional(CONF_SHORT_MEDIAN): sensor.sensor_schema(
                unit_of_measurement="\u00b5s",
                accuracy_decimals=0,
            ),
            cv.Optional(CONF_MINI_MEDIAN): sensor.sensor_schema(
                unit_of_measurement="\u00b5s",
                accuracy_decimals=0,
            ),
            cv.Optional(CONF_QUEUE_DEPTH): sensor.sensor_schema(
                accuracy_decimals=0,
            ),
            cv.Optional(CONF_HEAP_FREE): sensor.sensor_schema(
                unit_of_measurement="B",
                accuracy_decimals=0,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
            cv.Optional(CONF_PSRAM_FREE): sensor.sensor_schema(
                unit_of_measurement="B",
                accuracy_decimals=0,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
            cv.Optional(CONF_DECODER_RECV): sensor.sensor_schema(
                accuracy_decimals=0,
                state_class="total_increasing",
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
            cv.Optional(CONF_DECODER_DECODED): sensor.sensor_schema(
                accuracy_decimals=0,
                state_class="total_increasing",
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
            cv.Optional(CONF_DECODER_DROP): sensor.sensor_schema(
                accuracy_decimals=0,
                state_class="total_increasing",
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
            cv.Optional(CONF_DRIFT): sensor.sensor_schema(
                unit_of_measurement="\u00b5s/s",
                accuracy_decimals=3,
                state_class="measurement",
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),

            cv.Optional(CONF_WEBSERVER_PORT): cv.port,
        }
    )
    .extend(cv.COMPONENT_SCHEMA),
    cv.require_framework_version(esp_idf=cv.Version(5, 1, 1)),
)

async def to_code(config):

    add_idf_component(name="espressif/esp-dsp", ref=">1.5.0")

    add_idf_component(name="espressif/mdns", ref=">1.2.3")

    from pathlib import Path
    lightsnapcast_path = Path(__file__).parent.parent / "lightsnapcast"
    add_idf_component(name="lightsnapcast", path=str(lightsnapcast_path))

    for component, path in [
        ("libbuffer",     "components/libbuffer"),
        ("libmedian",     "components/libmedian"),
        ("opus",          "components/opus"),
        ("flac",          "components/flac"),
        ("timefilter",    "components/timefilter"),
        ("dsp_processor", "components/dsp_processor"),
    ]:
        add_idf_component(
            name=component,
            ref=SNAPCLIENT_GIT_VERSION,
            repo="https://github.com/CarlosDerSeher/snapclient.git",
            path=path,
        )

    if CONF_WEBSERVER_PORT in config:
        cg.add_build_flag(f"-DCONFIG_WEB_PORT={config[CONF_WEBSERVER_PORT]}")
        add_idf_component(
            name="ui_http_server",
            ref=SNAPCLIENT_GIT_VERSION,
            repo="https://github.com/CarlosDerSeher/snapclient.git",
            path="components/ui_http_server",
        )

    add_idf_sdkconfig_option("CONFIG_SNAPCLIENT_USE_SOFT_VOL", False)

    if CONF_NAME not in config:
        config[CONF_NAME] = CORE.name or ""

    use_mdns = config.get(CONF_HOSTNAME) is None
    if use_mdns:
        cg.add_build_flag("-DCONFIG_SNAPCLIENT_USE_MDNS=1")
    else:
        cg.add_build_flag("-DCONFIG_SNAPCLIENT_USE_MDNS=0")

    add_idf_sdkconfig_option("CONFIG_FREERTOS_TASK_NOTIFICATION_ARRAY_ENTRIES", 2)

    add_idf_sdkconfig_option("CONFIG_SNAPCLIENT_USE_TIMEFILTER", True)
    cg.add_build_flag("-DCONFIG_SNAPCLIENT_USE_TIMEFILTER=1")

    cg.add_build_flag("-DCONFIG_USE_SAMPLE_INSERTION=1")

    cg.add_build_flag("-Wno-error=incompatible-pointer-types")

    var = await media_player.new_media_player(config)
    await cg.register_component(var, config)

    from esphome.components import snapspot
    source = await snapspot.get_source_speaker("snapclient")
    cg.add(var.set_source_speaker(source))

    cg.add(var.set_config(
        CORE.friendly_name or CORE.name,
        config.get(CONF_HOSTNAME, ""),
        config[CONF_PORT],
    ))

    if CONF_MUTE_PIN in config:
        pin = await cg.gpio_pin_expression(config[CONF_MUTE_PIN])
        cg.add(var.set_mute_pin(pin))

    if audio_dac_config := config.get(CONF_AUDIO_DAC):
        aud_dac = await cg.get_variable(audio_dac_config)
        cg.add(var.set_audio_dac(aud_dac))

    cg.add(var.set_snapcast_min_db(config[CONF_SNAPCAST_MIN_DB]))
    cg.add(var.set_snapcast_max_db(config[CONF_SNAPCAST_MAX_DB]))
    cg.add(var.set_min_volume(config[CONF_MIN_VOLUME]))
    cg.add(var.set_max_volume(config[CONF_MAX_VOLUME]))

    if CONF_TRACK_NAME in config:
        s = await text_sensor.new_text_sensor(config[CONF_TRACK_NAME])
        cg.add(var.set_track_name_sensor(s))
    if CONF_ARTIST in config:
        s = await text_sensor.new_text_sensor(config[CONF_ARTIST])
        cg.add(var.set_artist_sensor(s))
    if CONF_ALBUM in config:
        s = await text_sensor.new_text_sensor(config[CONF_ALBUM])
        cg.add(var.set_album_sensor(s))
    if CONF_ALBUM_ART_URL in config:
        s = await text_sensor.new_text_sensor(config[CONF_ALBUM_ART_URL])
        cg.add(var.set_album_art_url_sensor(s))
    if CONF_DURATION in config:
        s = await sensor.new_sensor(config[CONF_DURATION])
        cg.add(var.set_duration_sensor(s))
    if CONF_POSITION in config:
        s = await sensor.new_sensor(config[CONF_POSITION])
        cg.add(var.set_position_sensor(s))

    if CONF_SYNC_AGE in config:
        s = await sensor.new_sensor(config[CONF_SYNC_AGE])
        cg.add(var.set_sync_age_sensor(s))
    if CONF_SHORT_MEDIAN in config:
        s = await sensor.new_sensor(config[CONF_SHORT_MEDIAN])
        cg.add(var.set_short_median_sensor(s))
    if CONF_MINI_MEDIAN in config:
        s = await sensor.new_sensor(config[CONF_MINI_MEDIAN])
        cg.add(var.set_mini_median_sensor(s))
    if CONF_QUEUE_DEPTH in config:
        s = await sensor.new_sensor(config[CONF_QUEUE_DEPTH])
        cg.add(var.set_queue_depth_sensor(s))
    if CONF_HEAP_FREE in config:
        s = await sensor.new_sensor(config[CONF_HEAP_FREE])
        cg.add(var.set_heap_free_sensor(s))
    if CONF_PSRAM_FREE in config:
        s = await sensor.new_sensor(config[CONF_PSRAM_FREE])
        cg.add(var.set_psram_free_sensor(s))
    if CONF_DECODER_RECV in config:
        s = await sensor.new_sensor(config[CONF_DECODER_RECV])
        cg.add(var.set_decoder_recv_sensor(s))
    if CONF_DECODER_DECODED in config:
        s = await sensor.new_sensor(config[CONF_DECODER_DECODED])
        cg.add(var.set_decoder_decoded_sensor(s))
    if CONF_DECODER_DROP in config:
        s = await sensor.new_sensor(config[CONF_DECODER_DROP])
        cg.add(var.set_decoder_drop_sensor(s))
    if CONF_DRIFT in config:
        s = await sensor.new_sensor(config[CONF_DRIFT])
        cg.add(var.set_drift_sensor(s))
