from esphome import pins
import esphome.codegen as cg
from esphome.components import audio_dac, media_player, text_sensor, sensor
from esphome.components.esp32 import add_extra_script, add_idf_component, add_idf_sdkconfig_option
from pathlib import Path
from esphome.components.i2s_audio import (
    CONF_I2S_DOUT_PIN,
    CONF_STEREO,
    I2SAudioOut,
    i2s_audio_component_schema,
    register_i2s_audio_component,
)
import esphome.config_validation as cv
from esphome.const import CONF_AUDIO_DAC, CONF_NAME
from esphome.core import CORE

CODEOWNERS = ["@farmed-switch"]

DEPENDENCIES = ["esp32", "i2s_audio"]
AUTO_LOAD = ["text_sensor", "sensor"]

CONF_DEVICE_NAME = "device_name"
CONF_PRIORITY = "priority"
CONF_PRIORITY_LOCK = "priority_lock"
# Spotify dB window: standard consumer range.
# 0% volume maps to spotify_min_db, 100% maps to spotify_max_db.
# Override in your YAML to match your DAC/amplifier headroom.
CONF_SPOTIFY_MIN_DB = "spotify_min_db"
CONF_SPOTIFY_MAX_DB = "spotify_max_db"

# Optional metadata sensors published to HA when a track is playing.
CONF_TRACK_NAME = "track_name"
CONF_ARTIST = "artist"
CONF_ALBUM = "album"
CONF_ALBUM_ART_URL = "album_art_url"
CONF_DURATION = "duration"
CONF_POSITION = "position"

# TODO (TASK-3 full integration): Register cspot-lib as an IDF component.
# cspot-lib lives at cspot-lib/ (local directory in this repo). Options:
#   1. Push cspot-lib to a public git repo and use:
#        add_idf_component(name="cspot", ref="<commit>", repo="<url>")
#   2. Add cspot-lib to EXTRA_COMPONENT_DIRS via a project-level CMakeLists.txt
#        override (see PETRA.md section 7.2 for CMakeLists design).
# For the skeleton the component registers sdkconfig options only; the actual
# idf_component_register / add_subdirectory wiring is deferred.
#
# Required sdkconfig for cspot:
#   CONFIG_COMPILER_CXX_EXCEPTIONS=y      (bell / cspot use exceptions)
#   CONFIG_MBEDTLS_CERTIFICATE_BUNDLE=y   (HTTPS to Spotify CDN)
#   CONFIG_SPIRAM_BOOT_INIT=y             (2 MB audio ring buffer in PSRAM)

spotify_connect_ns = cg.esphome_ns.namespace("spotify_connect")
SpotifyConnectComponent = spotify_connect_ns.class_(
    "SpotifyConnectComponent", cg.Component, media_player.MediaPlayer, I2SAudioOut
)

CONFIG_SCHEMA = cv.All(
    media_player.media_player_schema(SpotifyConnectComponent)
    .extend(
        i2s_audio_component_schema(
            SpotifyConnectComponent,
            default_sample_rate=44100,
            default_channel=CONF_STEREO,
            default_bits_per_sample="16bit",
        )
    )
    .extend(
        {
            cv.GenerateID(): cv.declare_id(SpotifyConnectComponent),
            # Human-readable name shown in the Spotify app (Connect device name).
            # Defaults to the ESPHome node name if not specified.
            cv.Optional(CONF_DEVICE_NAME): cv.string,
            # Override i2s_audio_component_schema's Optional with Required + full
            # pin schema so the YAML can pass {number: N, allow_other_uses: true}
            # when snapclient and spotify_connect share the same DOUT GPIO.
            cv.Required(CONF_I2S_DOUT_PIN): pins.internal_gpio_output_pin_schema,
            cv.Optional(CONF_AUDIO_DAC): cv.use_id(audio_dac.AudioDac),
            # dB window for volume scaling. cspot reports volume 0-65535;
            # this maps the HA media_player 0.0-1.0 slider to the DAC dB range.
            # Spotify uses a steeper curve than Snapcast (goes deeper into silence).
            cv.Optional(CONF_SPOTIFY_MIN_DB, default=-60.0): cv.float_,
            cv.Optional(CONF_SPOTIFY_MAX_DB, default=0.0): cv.float_,
            # Lock priority: higher value wins I2S arbitration.
            # 50 = default; snapclient uses 50 implicitly.
            # Increase to e.g. 80 when Spotify should preempt Snapcast (v2.1).
            cv.Optional(CONF_PRIORITY, default=50): cv.int_range(min=0, max=255),
            # Optional: reference to a shared priority_lock component (id: my_lock).
            # When omitted the component falls back to i2s_audio try_lock() directly.
            cv.Optional(CONF_PRIORITY_LOCK): cv.use_id(
                cg.MockObjClass(
                    "priority_lock::PriorityLockManager",
                    parents=[cg.Component],
                )
            ),
            # Optional metadata text sensors.
            cv.Optional(CONF_TRACK_NAME): text_sensor.text_sensor_schema(),
            cv.Optional(CONF_ARTIST): text_sensor.text_sensor_schema(),
            cv.Optional(CONF_ALBUM): text_sensor.text_sensor_schema(),
            cv.Optional(CONF_ALBUM_ART_URL): text_sensor.text_sensor_schema(),
            # Duration and position in seconds (numeric).
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
    print(f"[SPOTIFY-PREBUILD] to_code() called -- __file__={__file__} lib_exists={lib_dir_check.exists()}")
    # Enable C++ exceptions — required by bell/cspot STL usage.
    add_idf_sdkconfig_option("CONFIG_COMPILER_CXX_EXCEPTIONS", True)
    # mbedTLS — CRITICAL for Spotify AP + CDN TLS connections.
    # 16384 = standard TLS max record size; allocates from PSRAM via EXTERNAL_MEM_ALLOC.
    add_idf_sdkconfig_option("CONFIG_MBEDTLS_SSL_IN_CONTENT_LEN", 16384)
    add_idf_sdkconfig_option("CONFIG_MBEDTLS_SSL_OUT_CONTENT_LEN", 4096)
    add_idf_sdkconfig_option("CONFIG_MBEDTLS_ASYMMETRIC_CONTENT_LEN", True)
    add_idf_sdkconfig_option("CONFIG_MBEDTLS_CERTIFICATE_BUNDLE", True)
    add_idf_sdkconfig_option("CONFIG_MBEDTLS_CERTIFICATE_BUNDLE_DEFAULT_FULL", True)
    add_idf_sdkconfig_option("CONFIG_MBEDTLS_EXTERNAL_MEM_ALLOC", True)
    # PSRAM init — mandatory for the 2 MB audio ring buffer allocation.
    add_idf_sdkconfig_option("CONFIG_SPIRAM_BOOT_INIT", True)
    # Disable bell's internal Opus codec — snapclient-lib already provides the
    # espressif/opus IDF component; having two Opus builds causes symbol conflicts.
    add_idf_sdkconfig_option("CONFIG_BELL_CODEC_OPUS", False)
    # Emergency pool size for C++ exceptions (needed on xtensa with PSRAM).
    add_idf_sdkconfig_option("CONFIG_COMPILER_CXX_EXCEPTIONS_EMG_POOL_SIZE", 1024)
    # ── Network tuning (match IDF version for stable CDN streaming) ──
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
            raise cv.Invalid("cspot-lib directory not found! Expected at: " + str(cspot_path))
        add_idf_component(name="cspot-lib", path=str(cspot_path))
    else:
        lib_dir = lib_dir_check
        sp_libs = sorted(lib_dir.glob("libspotifyconnect-*.a"))
        cp_libs = sorted(lib_dir.glob("libcspot-*.a"))
        print(f"[SPOTIFY-PREBUILD] lib_dir={lib_dir} sp_libs={sp_libs} cp_libs={cp_libs}")
        if not sp_libs or not cp_libs:
            raise RuntimeError(
                f"Prebuild mode: lib/ exists but archives missing in {lib_dir}. "
                f"libspotifyconnect-*.a: {sp_libs}, libcspot-*.a: {cp_libs}. "
                "Check that the build-and-publish workflow completed successfully."
            )
        vtable = "_ZTVN7esphome15spotify_connect23SpotifyConnectComponentE"
        sp_lib = sp_libs[0].as_posix()
        cp_lib = cp_libs[0].as_posix()
        # The final link is done by PlatformIO's SCons ld, not CMake/ninja.
        # add_extra_script("post") modifies the SCons LINKFLAGS after the
        # bootloader env is cloned — the bootloader never sees these flags.
        script = "\n".join([
            'Import("env")',
            f'env.Prepend(LINKFLAGS=["-Wl,--undefined={vtable}"])',
            f'env.Append(LINKFLAGS=["-Wl,{sp_lib}", "-Wl,{cp_lib}"])',
            "",
        ])
        script_path = lib_dir / "spotify_connect_link.py"
        script_path.write_text(script)
        print(f"[SPOTIFY-PREBUILD] wrote {script_path}, calling add_extra_script")
        result = add_extra_script("post", "spotify_connect_link.py", script_path)
        print(f"[SPOTIFY-PREBUILD] add_extra_script done, result={result}")

    device_name = config.get(CONF_DEVICE_NAME) or CORE.name or "SnapSpot"

    var = await media_player.new_media_player(config)
    await cg.register_component(var, config)
    await register_i2s_audio_component(var, config)
    dout_pin = await cg.gpio_pin_expression(config[CONF_I2S_DOUT_PIN])
    cg.add(var.set_dout_pin(dout_pin))

    cg.add(var.set_device_name(device_name))
    cg.add(var.set_priority(config[CONF_PRIORITY]))
    cg.add(var.set_spotify_min_db(config[CONF_SPOTIFY_MIN_DB]))
    cg.add(var.set_spotify_max_db(config[CONF_SPOTIFY_MAX_DB]))
    if CONF_PRIORITY_LOCK in config:
        lock = await cg.get_variable(config[CONF_PRIORITY_LOCK])
        cg.add(var.set_priority_lock(lock))

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
