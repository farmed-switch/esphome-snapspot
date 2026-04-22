from esphome import pins
import esphome.codegen as cg
from esphome.components import audio_dac, media_player, text_sensor, sensor
from esphome.components.esp32 import add_extra_script, add_idf_component, add_idf_sdkconfig_option, get_esp32_variant
from pathlib import Path
import esphome.config_validation as cv
from esphome.const import CONF_AUDIO_DAC, CONF_NAME
from esphome.core import CORE

CODEOWNERS = ["@farmed-switch"]

DEPENDENCIES = ["esp32", "snapspot"]
AUTO_LOAD = ["text_sensor", "sensor"]

CONF_DEVICE_NAME = "device_name"
CONF_SPOTIFY_MIN_DB = "spotify_min_db"
CONF_SPOTIFY_MAX_DB = "spotify_max_db"
CONF_MIN_VOLUME = "min_volume"
CONF_MAX_VOLUME = "max_volume"

CONF_TRACK_NAME = "track_name"
CONF_ARTIST = "artist"
CONF_ALBUM = "album"
CONF_ALBUM_ART_URL = "album_art_url"
CONF_DURATION = "duration"
CONF_POSITION = "position"

spotify_connect_ns = cg.esphome_ns.namespace("spotify_connect")
SpotifyConnectComponent = spotify_connect_ns.class_(
    "SpotifyConnectComponent", cg.Component, media_player.MediaPlayer
)

CONFIG_SCHEMA = cv.All(
    media_player.media_player_schema(SpotifyConnectComponent)
    .extend(
        {
            cv.GenerateID(): cv.declare_id(SpotifyConnectComponent),

            cv.Optional(CONF_DEVICE_NAME): cv.string,
            cv.Optional(CONF_AUDIO_DAC): cv.use_id(audio_dac.AudioDac),

            cv.Optional(CONF_SPOTIFY_MIN_DB, default=-60.0): cv.float_,
            cv.Optional(CONF_SPOTIFY_MAX_DB, default=0.0): cv.float_,
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
        }
    )
    .extend(cv.COMPONENT_SCHEMA),
    cv.require_framework_version(esp_idf=cv.Version(5, 1, 1)),
)

async def to_code(config):
    lib_dir_check = Path(__file__).parent / "lib"

    add_idf_sdkconfig_option("CONFIG_COMPILER_CXX_EXCEPTIONS", True)

    add_idf_sdkconfig_option("CONFIG_MBEDTLS_SSL_IN_CONTENT_LEN", 16384)
    add_idf_sdkconfig_option("CONFIG_MBEDTLS_SSL_OUT_CONTENT_LEN", 4096)
    add_idf_sdkconfig_option("CONFIG_MBEDTLS_ASYMMETRIC_CONTENT_LEN", True)
    add_idf_sdkconfig_option("CONFIG_MBEDTLS_CERTIFICATE_BUNDLE", True)
    add_idf_sdkconfig_option("CONFIG_MBEDTLS_CERTIFICATE_BUNDLE_DEFAULT_FULL", True)
    add_idf_sdkconfig_option("CONFIG_MBEDTLS_EXTERNAL_MEM_ALLOC", True)

    add_idf_sdkconfig_option("CONFIG_SPIRAM_BOOT_INIT", True)

    add_idf_sdkconfig_option("CONFIG_BELL_CODEC_OPUS", False)

    add_idf_sdkconfig_option("CONFIG_COMPILER_CXX_EXCEPTIONS_EMG_POOL_SIZE", 1024)

    add_idf_sdkconfig_option("CONFIG_LWIP_MAX_SOCKETS", 16)
    add_idf_sdkconfig_option("CONFIG_LWIP_MAX_ACTIVE_TCP", 16)
    add_idf_sdkconfig_option("CONFIG_LWIP_MAX_LISTENING_TCP", 8)
    add_idf_sdkconfig_option("CONFIG_LWIP_SO_REUSE", True)
    add_idf_sdkconfig_option("CONFIG_LWIP_TCP_RECVMBOX_SIZE", 12)
    add_idf_sdkconfig_option("CONFIG_MBEDTLS_DYNAMIC_BUFFER", True)
    add_idf_sdkconfig_option("CONFIG_MBEDTLS_PSK_MODES", True)
    add_idf_sdkconfig_option("CONFIG_HTTPD_MAX_URI_HANDLERS", 24)
    add_idf_sdkconfig_option("CONFIG_ESP_WIFI_STATIC_RX_BUFFER_NUM", 16)
    add_idf_sdkconfig_option("CONFIG_ESP_MAIN_TASK_STACK_SIZE", 3584)

    if not lib_dir_check.exists():
        cspot_path = Path(CORE.config_dir).parent / "cspot-lib"
        if not (cspot_path / "CMakeLists.txt").exists():
            cspot_path = Path(__file__).parent.parent.parent.parent / "cspot-lib"
        if not (cspot_path / "CMakeLists.txt").exists():
            raise cv.Invalid("cspot-lib directory not found. Expected at: " + str(cspot_path))
        add_idf_component(name="cspot-lib", path=str(cspot_path))
    else:
        lib_dir = lib_dir_check
        variant = str(get_esp32_variant()).lower().replace("-", "")
        sp_libs = sorted(lib_dir.glob(f"libspotifyconnect-*-{variant}.a"))
        cp_libs = sorted(lib_dir.glob(f"libcspot-*-{variant}.a"))
        if not cp_libs:
            raise RuntimeError(
                f"Prebuild mode: lib/ exists but libcspot-*.a missing in {lib_dir}."
            )
        vtable = "_ZTVN7esphome15spotify_connect23SpotifyConnectComponentE"
        cp_lib = cp_libs[0].as_posix()
        sp_entry = f'"-Wl,{sp_libs[0].as_posix()}", ' if sp_libs else ""
        script = "\n".join([
            'Import("env")',
            f'env.Prepend(LINKFLAGS=["-Wl,--undefined={vtable}"])',
            f'env.Append(LINKFLAGS=[{sp_entry}"-Wl,{cp_lib}"])',
            "",
        ])
        script_path = lib_dir / "spotify_connect_link.py"
        script_path.write_text(script)
        add_extra_script("post", "spotify_connect_link.py", script_path)

    device_name = config.get(CONF_DEVICE_NAME) or CORE.friendly_name or CORE.name or "SnapSpot"

    var = await media_player.new_media_player(config)
    await cg.register_component(var, config)

    from esphome.components import snapspot
    source = await snapspot.get_source_speaker("spotify_connect")
    cg.add(var.set_source_speaker(source))

    cg.add(var.set_device_name(device_name))
    cg.add(var.set_spotify_min_db(config[CONF_SPOTIFY_MIN_DB]))
    cg.add(var.set_spotify_max_db(config[CONF_SPOTIFY_MAX_DB]))
    cg.add(var.set_min_volume(config[CONF_MIN_VOLUME]))
    cg.add(var.set_max_volume(config[CONF_MAX_VOLUME]))

    if CONF_AUDIO_DAC in config:
        dac = await cg.get_variable(config[CONF_AUDIO_DAC])
        cg.add(var.set_audio_dac(dac))

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
