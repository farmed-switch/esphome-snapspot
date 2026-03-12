from esphome import pins
import esphome.codegen as cg
from esphome.components import audio_dac, media_player, sensor, socket, text_sensor, wifi
from esphome.components.esp32 import add_extra_script, add_idf_component, add_idf_sdkconfig_option
from esphome.components.ethernet import SPI_ETHERNET_TYPES
from esphome.components.i2s_audio import (
    CONF_I2S_DOUT_PIN,
    CONF_STEREO,
    I2SAudioOut,
    i2s_audio_component_schema,
    register_i2s_audio_component,
)
import esphome.config_validation as cv
from esphome.const import CONF_AUDIO_DAC, CONF_NAME, CONF_PORT, CONF_TYPE
from esphome.core import CORE
from pathlib import Path

CODEOWNERS = ["@luar123"]

DEPENDENCIES = ["esp32", "i2s_audio"]
AUTO_LOAD = ["mdns", "socket"]

CONF_HOSTNAME = "hostname"
CONF_MUTE_PIN = "mute_pin"
CONF_PRIORITY = "priority"
CONF_PRIORITY_LOCK = "priority_lock"
# Snapcast dB window: server controls the curve, so range extends slightly above 0 dB.
# 0% volume maps to snapcast_min_db, 100% maps to snapcast_max_db.
# Override in your YAML to match the Snapserver slider range you use.
CONF_SNAPCAST_MIN_DB = "snapcast_min_db"
CONF_SNAPCAST_MAX_DB = "snapcast_max_db"

# Optional metadata text sensors: show title/artist/album of stream playing
# via Snapserver STREAM_TAGS. Works with any server-side source that calls
# Stream.SetMeta (spotifyd, mopidy, shairport-sync, etc.).
CONF_TRACK_NAME = "track_name"
CONF_ARTIST = "artist"
CONF_ALBUM = "album"
CONF_ALBUM_ART_URL = "album_art_url"
CONF_DURATION = "duration"
CONF_POSITION = "position"

# Pinned to the farmed-switch fork of luar123/snapclient.
# To update: change the commit hash AND update the submodule in snapclient-lib/:
#   cd snapclient-lib && git fetch && git checkout <new-commit> && cd ..
#   git add snapclient-lib && git commit -m "bump snapclient to <hash>"
#   Then update SNAPCLIENT_GIT_VERSION below to match.
# Fork of luar123/snapclient with PCM callback support (register_pcm_process_cb).
# After pushing player.c/player.h changes, run: git log --oneline -1  →  update hash.
SNAPCLIENT_GIT_VERSION = "ecaaf231180401db235fc56cf1ac268ef12a7cbb"
SNAPCLIENT_GIT_REPO = "https://github.com/farmed-switch/snapclient.git"

snapclient_ns = cg.esphome_ns.namespace("snapclient")
SnapClientComponent = snapclient_ns.class_(
    "SnapClientComponent", cg.Component, media_player.MediaPlayer, I2SAudioOut
)


def _consume_sockets(config):
    """Register socket needs for this component."""
    # upstream uses 10 sockets, but 7 are used for http server
    socket.consume_sockets(3, "snapclient")(config)
    return config


CONFIG_SCHEMA = cv.All(
    media_player.media_player_schema(SnapClientComponent)
    .extend(
        i2s_audio_component_schema(
            SnapClientComponent,
            default_sample_rate=44100,
            default_channel=CONF_STEREO,
            default_bits_per_sample="16bit",
        )
    )
    .extend(
        {
            cv.GenerateID(): cv.declare_id(SnapClientComponent),
            cv.Optional(CONF_NAME): cv.string,
            # Empty hostname means "discover via mDNS".
            cv.Optional(CONF_HOSTNAME): cv.domain,
            cv.Optional(CONF_PORT, default=1704): cv.port,
            cv.Required(CONF_I2S_DOUT_PIN): pins.internal_gpio_output_pin_schema,
            cv.Optional(CONF_MUTE_PIN): pins.gpio_output_pin_schema,
            cv.Optional(CONF_AUDIO_DAC): cv.use_id(audio_dac.AudioDac),
            # dB window for volume scaling (0.0-1.0 HA slider → dB to audio_dac).
            # Snapcast server controls its own curve; these values define the
            # translation from the HA media_player volume slider to the DAC.
            cv.Optional(CONF_SNAPCAST_MIN_DB, default=-45.0): cv.float_,
            cv.Optional(CONF_SNAPCAST_MAX_DB, default=15.0): cv.float_,
            # Lock priority. Higher = preferred when two sources compete.
            # Default 50 matches spotify_connect default so neither preempts.
            # Set snapclient lower (e.g. 30) to let Spotify win automatically in v2.1.
            cv.Optional(CONF_PRIORITY, default=50): cv.int_range(min=0, max=255),
            # Optional: reference to a shared priority_lock component (id: my_lock).
            # When omitted the component falls back to i2s_audio try_lock() directly.
            cv.Optional(CONF_PRIORITY_LOCK): cv.use_id(
                cg.MockObjClass(
                    "priority_lock::PriorityLockManager",
                    parents=[cg.Component],
                )
            ),
            # Optional metadata text sensors updated from Snapserver STREAM_TAGS.
            # Works with any source that calls Stream.SetMeta on the Snapserver
            # (spotifyd onevent hook, mopidy-snapcast, shairport-sync, etc.).
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
    _consume_sockets,  # Register socket usage during validation
)


async def to_code(config):
    lib_dir_check = Path(__file__).parent / "lib"
    print(f"[SNAPCLIENT-PREBUILD] to_code() called -- __file__={__file__} lib_exists={lib_dir_check.exists()}")

    if lib_dir_check.exists():
        lib_dir = lib_dir_check
        sc_libs = sorted(lib_dir.glob("libsnapclient-*.a"))
        print(f"[SNAPCLIENT-PREBUILD] lib_dir={lib_dir} sc_libs={sc_libs}")
        if not sc_libs:
            raise RuntimeError(
                f"Prebuild mode: lib/ exists but no libsnapclient-*.a in {lib_dir}. "
                "Check that the build-and-publish workflow completed successfully."
            )
        vtable = "_ZTVN7esphome10snapclient19SnapClientComponentE"
        sc_lib = sc_libs[0].as_posix()
        script = "\n".join([
            'Import("env")',
            'import glob, os',
            f'env.Prepend(LINKFLAGS=["-Wl,--undefined={vtable}"])',
            f'env.Append(LINKFLAGS=["-Wl,{sc_lib}"])',
            '# dsp_processor.c.o (baked in libsnapclient.a) references dsps_biquad_* from',
            '# esp-dsp.  The IDF link group closes before libsnapclient.a is processed,',
            '# so we append esp-dsp explicitly after the prebuilt .a (link-order fix).',
            '_bd = env.subst("$BUILD_DIR")',
            '_dsp_libs = (',
            '    glob.glob(os.path.join(_bd, "esp-idf", "espressif__esp-dsp", "*.a")) or',
            '    glob.glob(os.path.join(_bd, "esp-idf", "esp-dsp", "*.a"))',
            ')',
            'print("[SNAPCLIENT-LINK] BUILD_DIR esp-idf dsp candidates:", glob.glob(os.path.join(_bd, "esp-idf", "*dsp*", "*.a")))' ,
            'if _dsp_libs:',
            '    print("[SNAPCLIENT-LINK] appending esp-dsp:", _dsp_libs[0])',
            '    env.Append(LINKFLAGS=["-Wl," + _dsp_libs[0]])',
            'else:',
            '    print("[SNAPCLIENT-LINK] WARNING: esp-dsp .a not found in BUILD_DIR -- dsps_biquad_* may be unresolved")',
            "",
        ])
        script_path = lib_dir / "snapclient_link.py"
        script_path.write_text(script)
        print(f"[SNAPCLIENT-PREBUILD] wrote {script_path}, calling add_extra_script")
        add_extra_script("post", "snapclient_link.py", script_path)
        # esp-dsp must be declared as an IDF component so it gets compiled and
        # linked -- dsp_processor.c.o (baked in libsnapclient.a) calls dsps_biquad_*.
        add_idf_component(name="espressif/esp-dsp", ref=">1.5.0")
    else:
        add_idf_component(name="espressif/esp-dsp", ref=">1.5.0")
        for component in [
            "dsp_processor",
            "flac",
            "libbuffer",
            "libmedian",
            "lightsnapcast",
            "opus",
            "snapclient",
            "timefilter",
        ]:
            add_idf_component(
                name=component,
                ref=SNAPCLIENT_GIT_VERSION,
                repo=SNAPCLIENT_GIT_REPO,
                path=f"components/{component}",
            )

    if CONF_AUDIO_DAC not in config:
        add_idf_sdkconfig_option("CONFIG_USE_DSP_PROCESSOR", True)
        add_idf_sdkconfig_option("CONFIG_SNAPCLIENT_USE_SOFT_VOL", True)
    if CONF_NAME not in config:
        config[CONF_NAME] = CORE.name or ""

    use_mdns = config.get(CONF_HOSTNAME) is None
    if not use_mdns:
        add_idf_sdkconfig_option("CONFIG_SNAPSERVER_HOST", str(config[CONF_HOSTNAME]))
    add_idf_sdkconfig_option("CONFIG_SNAPSERVER_PORT", int(config[CONF_PORT]))
    add_idf_sdkconfig_option("CONFIG_SNAPSERVER_USE_MDNS", use_mdns)
    add_idf_sdkconfig_option("CONFIG_SNAPCLIENT_NAME", config[CONF_NAME])
    add_idf_sdkconfig_option("CONFIG_FREERTOS_TASK_NOTIFICATION_ARRAY_ENTRIES", 2)
    ethernet = CORE.config.get("ethernet")
    if ethernet:
        if ethernet.get(CONF_TYPE) in SPI_ETHERNET_TYPES:
            cg.add_build_flag("-DCONFIG_SNAPCLIENT_USE_SPI_ETHERNET=1")
        else:
            cg.add_build_flag("-DCONFIG_SNAPCLIENT_USE_INTERNAL_ETHERNET=1")
    wifi.enable_runtime_power_save_control()

    var = await media_player.new_media_player(config)
    await cg.register_component(var, config)
    await register_i2s_audio_component(var, config)
    dout_pin = await cg.gpio_pin_expression(config[CONF_I2S_DOUT_PIN])
    cg.add(var.set_dout_pin(dout_pin))
    cg.add(var.set_snapserver_hostname(config.get(CONF_HOSTNAME, "")))
    cg.add(var.set_snapserver_port(config[CONF_PORT]))
    cg.add(var.set_snapserver_use_mdns(use_mdns))
    if CONF_MUTE_PIN in config:
        pin = await cg.gpio_pin_expression(config[CONF_MUTE_PIN])
        cg.add(var.set_mute_pin(pin))
    if audio_dac_config := config.get(CONF_AUDIO_DAC):
        aud_dac = await cg.get_variable(audio_dac_config)
        cg.add(var.set_audio_dac(aud_dac))
    cg.add(var.set_snapcast_min_db(config[CONF_SNAPCAST_MIN_DB]))
    cg.add(var.set_snapcast_max_db(config[CONF_SNAPCAST_MAX_DB]))
    cg.add(var.set_priority(config[CONF_PRIORITY]))
    if CONF_PRIORITY_LOCK in config:
        lock = await cg.get_variable(config[CONF_PRIORITY_LOCK])
        cg.add(var.set_priority_lock(lock))
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
