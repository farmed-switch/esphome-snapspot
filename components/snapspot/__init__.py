from pathlib import Path

from esphome import pins
import esphome.codegen as cg
from esphome.components import audio_dac
from esphome.components.esp32 import (
    add_extra_script,
    add_idf_component,
    add_idf_sdkconfig_option,
    get_esp32_variant,
)
from esphome.components.i2s_audio import (
    CONF_I2S_DOUT_PIN,
    CONF_STEREO,
    I2SAudioOut,
    i2s_audio_component_schema,
    register_i2s_audio_component,
)
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_NAME, CONF_PORT
from esphome.core import CORE

CODEOWNERS = ["@farmed-switch"]

DEPENDENCIES = ["esp32", "i2s_audio", "web_server_base"]

# CarlosDerSeher/snapclient commit used for Snapcast components
SNAPCLIENT_GIT_VERSION = "1adc5245012160c3c4eb312c962c7dc18b17231e"

CONF_AUDIO_DAC = "audio_dac"
CONF_MUTE_PIN = "mute_pin"
CONF_SPOTIFY_NAME = "spotify_name"
CONF_SNAPCAST_HOST = "snapcast_host"
CONF_SPOTIFY_VOLUME_MAX = "spotify_volume_max"
CONF_SNAPCAST_VOLUME_MAX = "snapcast_volume_max"
CONF_SPOTIFY_MIN_DB = "spotify_min_db"
CONF_SPOTIFY_MAX_DB = "spotify_max_db"
CONF_SNAPCAST_MIN_DB = "snapcast_min_db"
CONF_SNAPCAST_MAX_DB = "snapcast_max_db"
CONF_DAC_MIN_DB = "dac_min_db"
CONF_DAC_MAX_DB = "dac_max_db"

snapspot_ns = cg.esphome_ns.namespace("snapspot")
SnapSpotComponent = snapspot_ns.class_(
    "SnapSpotComponent", cg.Component, I2SAudioOut
)

def _validate_psram(config):
    """snapspot requires PSRAM - Spotify ring buffer (2MB) and Snapcast PCM buffers won't fit in DRAM."""
    if CORE.config is not None and "psram" not in CORE.config:
        raise cv.Invalid(
            "snapspot requires PSRAM. Add 'psram:' to your YAML. "
            "Spotify ring buffer (2MB) and Snapcast PCM buffers require external SRAM."
        )
    return config


CONFIG_SCHEMA = cv.All(
    i2s_audio_component_schema(
        SnapSpotComponent,
        default_sample_rate=44100,
        default_channel=CONF_STEREO,
        default_bits_per_sample="16bit",
    )
    .extend(
        {
            cv.GenerateID(): cv.declare_id(SnapSpotComponent),
            cv.Required(CONF_I2S_DOUT_PIN): pins.internal_gpio_output_pin_number,
            cv.Optional(CONF_MUTE_PIN): pins.gpio_output_pin_schema,
            cv.Optional(CONF_AUDIO_DAC): cv.use_id(audio_dac.AudioDac),
            cv.Optional(CONF_SPOTIFY_NAME): cv.string,
            cv.Optional(CONF_SNAPCAST_HOST, default=""): cv.string,
            cv.Optional(CONF_PORT, default=1704): cv.port,
            cv.Optional(CONF_SPOTIFY_VOLUME_MAX, default=1.0): cv.float_range(min=0.0, max=1.0),
            cv.Optional(CONF_SNAPCAST_VOLUME_MAX, default=1.0): cv.float_range(min=0.0, max=1.0),
            # Separate dB ranges for each source. Must match TAS5805M volume_min/volume_max.
            # 0% volume maps to <source>_min_db, 100% maps to <source>_max_db.
            # dac_min_db / dac_max_db must equal TAS5805M volume_min / volume_max in your YAML.
            cv.Optional(CONF_DAC_MIN_DB, default=-25.0): cv.float_,
            cv.Optional(CONF_DAC_MAX_DB, default=15.0): cv.float_,
            cv.Optional(CONF_SPOTIFY_MIN_DB, default=-25.0): cv.float_,
            cv.Optional(CONF_SPOTIFY_MAX_DB, default=15.0): cv.float_,
            cv.Optional(CONF_SNAPCAST_MIN_DB, default=-25.0): cv.float_,
            cv.Optional(CONF_SNAPCAST_MAX_DB, default=15.0): cv.float_,
        }
    )
    .extend(cv.COMPONENT_SCHEMA),
    cv.require_framework_version(esp_idf=cv.Version(5, 1, 1)),
    _validate_psram,
)


async def to_code(config):
    # ── prebuilt .a: link via PlatformIO extra_script ────────────────────────
    # In the public repo the C++ source is replaced by prebuilt .a files in lib/.
    #
    # PROBLEM: pioarduino places positional .a args BEFORE .o files in the link command.
    # The linker scans positional .a files when no symbols are undefined yet → nothing
    # is extracted. cg.add_build_flag() ended up in the same positional-before-.o block.
    #
    # SOLUTION: PlatformIO extra_script that modifies env["LINKFLAGS"].
    #   - extra_scripts run ONLY for pioarduino's SCons firmware.elf link step.
    #   - The bootloader is a CMake sub-project built BEFORE the SCons step; it does
    #     NOT receive extra_script LINKFLAGS changes — it only sees platformio.ini
    #     build_flags (which we no longer use for the .a paths).
    #   - We Prepend --undefined=vtable and Append .a paths to LINKFLAGS:
    #       LINKFLAGS order: ... --undefined=vtable ... /path/libsnapspot.a ...
    #       Full link:   [LINKFLAGS] [SOURCES/.o] [--start-group idf libs --end-group]
    #     The linker sees vtable as undefined BEFORE scanning the positional .a → pulls
    #     the vtable object → firmware links completely. ✅
    #   - Bootloader: no --undefined, no .a → links cleanly ✅
    lib_dir = Path(__file__).parent / "lib"
    if lib_dir.exists():
        # Determine IDF target string from ESP32 variant.
        # get_esp32_variant() returns a plain string e.g. "ESP32S3", "ESP32", "ESP32C3".
        _VARIANT_TO_TARGET = {
            "ESP32":   "esp32",
            "ESP32S2": "esp32s2",
            "ESP32S3": "esp32s3",
            "ESP32C3": "esp32c3",
            "ESP32C6": "esp32c6",
            "ESP32H2": "esp32h2",
            "ESP32P4": "esp32p4",
        }
        idf_target = _VARIANT_TO_TARGET.get(get_esp32_variant(), "esp32s3")

        # Find the prebuilt .a files matching target (e.g. libsnapspot-idf*-esp32s3.a)
        snapspot_libs = sorted(lib_dir.glob(f"libsnapspot-idf*-{idf_target}.a"))
        cspot_libs    = sorted(lib_dir.glob(f"libcspot-idf*-{idf_target}.a"))
        if snapspot_libs and cspot_libs:
            vtable_symbol = "_ZTVN7esphome8snapspot17SnapSpotComponentE"
            snapspot_lib  = snapspot_libs[0].as_posix()
            cspot_lib     = cspot_libs[0].as_posix()

            # Write a PlatformIO extra_script that will be copied to the build dir
            # and run AFTER espidf.py — so that bootloader_env = env.Clone() has
            # already happened before we touch env.LINKFLAGS.
            #
            # pioarduino execution order:
            #   1. main.py  (sets PROGNAME=firmware, etc.)
            #   2. pre: extra_scripts
            #   3. espidf.py  ← does bootloader_env = env.Clone()
            #   4. post: extra_scripts  ← WE RUN HERE
            #
            # Appending to env in a post: script is safe: bootloader_env is
            # already an independent clone and won't be affected by our changes.
            link_script_content = "\n".join([
                'Import("env")',
                '# Generated by snapspot __init__.py — do not edit by hand.',
                '# post: script — runs AFTER espidf.py clones the bootloader env.',
                '# Changes to env here do NOT propagate to bootloader_env.',
                f'env.Prepend(LINKFLAGS=["-Wl,--undefined={vtable_symbol}"])',
                f'env.Append(LINKFLAGS=["-Wl,{snapspot_lib}", "-Wl,{cspot_lib}"])',
                '',
            ])
            link_script_path = lib_dir / "snapspot_link.py"
            link_script_path.write_text(link_script_content)
            add_extra_script("post", "snapspot_link.py", link_script_path)

    # ── cspot: local source (dev repo only) ──────────────────────────────────
    # In the public repo cspot is already baked into libcspot-idf5.5.2-esp32s3.a
    cspot_path = Path(__file__).parent.parent.parent / "idf_components" / "cspot"
    if cspot_path.exists():
        add_idf_component(name="cspot", path=str(cspot_path))

    # esp_http_client is needed by cspot (bell/HTTPClient.cpp)
    # Added to CMakeLists.txt REQUIRES instead of using include_builtin_idf_component
    # (that function was removed in ESPHome 2026.x)

    # ── steg 11: flac + opus (from CarlosDerSeher/snapclient) ─────────────────
    # Used by audio_decoder.cpp for FLAC and Opus codec support.
    # lightsnapcast / libbuffer / libmedian / dsp_processor stay commented (not used).
    add_idf_component(
        name="opus",
        ref=SNAPCLIENT_GIT_VERSION,
        repo="https://github.com/CarlosDerSeher/snapclient.git",
        path="components/opus",
    )
    add_idf_component(
        name="flac",
        ref=SNAPCLIENT_GIT_VERSION,
        repo="https://github.com/CarlosDerSeher/snapclient.git",
        path="components/flac",
    )

    # ── CarlosDerSeher components NOT used (kept as reference) ─────────────────
    # add_idf_component(name="espressif/esp-dsp", ref=">1.5.0")
    # add_idf_component(name="espressif/mdns", ref=">1.2.3")
    # add_idf_component(
    #     name="lightsnapcast",
    #     ref=SNAPCLIENT_GIT_VERSION,
    #     repo="https://github.com/CarlosDerSeher/snapclient.git",
    #     path="components/lightsnapcast",
    # )
    # add_idf_component(
    #     name="libbuffer",
    #     ref=SNAPCLIENT_GIT_VERSION,
    #     repo="https://github.com/CarlosDerSeher/snapclient.git",
    #     path="components/libbuffer",
    # )
    # add_idf_component(
    #     name="libmedian",
    #     ref=SNAPCLIENT_GIT_VERSION,
    #     repo="https://github.com/CarlosDerSeher/snapclient.git",
    #     path="components/libmedian",
    # )
    # add_idf_component(
    #     name="dsp_processor",
    #     ref=SNAPCLIENT_GIT_VERSION,
    #     repo="https://github.com/CarlosDerSeher/snapclient.git",
    #     path="components/dsp_processor",
    # )

    # ── sdkconfig: CRITICAL for cspot ────────────────────────────────────────
    # C++ exceptions: cspot throws std::runtime_error on disconnect
    add_idf_sdkconfig_option("CONFIG_COMPILER_CXX_EXCEPTIONS", True)
    add_idf_sdkconfig_option("CONFIG_COMPILER_CXX_EXCEPTIONS_EMG_POOL_SIZE", 0)

    # mbedTLS: Spotify CDN uses large TLS records; default buffers cause crash
    add_idf_sdkconfig_option("CONFIG_MBEDTLS_SSL_IN_CONTENT_LEN", 8192)
    add_idf_sdkconfig_option("CONFIG_MBEDTLS_SSL_OUT_CONTENT_LEN", 4096)
    add_idf_sdkconfig_option("CONFIG_MBEDTLS_ASYMMETRIC_CONTENT_LEN", True)
    add_idf_sdkconfig_option("CONFIG_MBEDTLS_CERTIFICATE_BUNDLE_DEFAULT_FULL", True)
    add_idf_sdkconfig_option("CONFIG_MBEDTLS_EXTERNAL_MEM_ALLOC", True)
    add_idf_sdkconfig_option("CONFIG_MBEDTLS_PSK_MODES", True)

    # HTTP server: default max URI handlers (8) is too low with ESPHome + ZeroConf
    add_idf_sdkconfig_option("CONFIG_HTTPD_MAX_URI_HANDLERS", 24)

    # Main task stack: slightly larger to accommodate ESPHome + component setup
    add_idf_sdkconfig_option("CONFIG_ESP_MAIN_TASK_STACK_SIZE", 3584)

    # ── fix for esp-idf 5.4 ──────────────────────────────────────────────────
    cg.add_build_flag("-Wno-error=incompatible-pointer-types")
    cg.add_build_flag("-DCONFIG_USE_SAMPLE_INSERTION=1")

    # ── component wiring ─────────────────────────────────────────────────────
    spotify_name = config.get(CONF_SPOTIFY_NAME) or CORE.name or "SnapSpot"

    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await register_i2s_audio_component(var, config)

    cg.add(var.set_dout_pin(config[CONF_I2S_DOUT_PIN]))
    cg.add(var.set_spotify_name(spotify_name))
    cg.add(var.set_snapcast_host(config[CONF_SNAPCAST_HOST]))
    cg.add(var.set_snapcast_port(config[CONF_PORT]))
    cg.add(var.set_spotify_volume_max(config[CONF_SPOTIFY_VOLUME_MAX]))
    cg.add(var.set_snapcast_volume_max(config[CONF_SNAPCAST_VOLUME_MAX]))
    cg.add(var.set_dac_db_range(config[CONF_DAC_MIN_DB], config[CONF_DAC_MAX_DB]))
    cg.add(var.set_spotify_db_range(config[CONF_SPOTIFY_MIN_DB], config[CONF_SPOTIFY_MAX_DB]))
    cg.add(var.set_snapcast_db_range(config[CONF_SNAPCAST_MIN_DB], config[CONF_SNAPCAST_MAX_DB]))

    if CONF_MUTE_PIN in config:
        pin = await cg.gpio_pin_expression(config[CONF_MUTE_PIN])
        cg.add(var.set_mute_pin(pin))

    if audio_dac_config := config.get(CONF_AUDIO_DAC):
        aud_dac = await cg.get_variable(audio_dac_config)
        cg.add(var.set_audio_dac(aud_dac))
