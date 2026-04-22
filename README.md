<p align="center">
  <img src="assets/snapspot-logo.jpg" alt="SnapSpot logo" width="160">
</p>

# SnapSpot

**Spotify Connect + Snapcast on ESP32 — one YAML, one board, all rooms.**

[![ESPHome](https://img.shields.io/badge/ESPHome-2026.x-brightgreen)](https://esphome.io)
[![License: GPL-3.0](https://img.shields.io/badge/License-GPL--3.0-blue.svg)](LICENSE)
[![ESP32](https://img.shields.io/badge/ESP32-classic%20%7C%20S3-blue)](https://www.espressif.com)

---

SnapSpot turns an ESP32 board into two media players that share the same speaker output:
a **Spotify Connect** device and a **Snapcast** synchronized client. Both show up in
Home Assistant as full media players with volume, metadata, and playback controls.

When Spotify starts playing, it takes over. When it stops, Snapcast resumes automatically.

---

## Heads up — this is early days

I'll be honest: this project is still pretty young. It works well on my own boards
and I use it every day at home, but there are absolutely still **bugs and rough edges**
that I'm slowly working through as I find them. Audio glitches, the occasional reconnect,
volume curves that don't always feel right — I'm aware of a lot of this and chipping
away at it bit by bit.

If you're the kind of person who likes tinkering and doesn't mind that things might
not be perfect on day one, **please go ahead and try it.** I'd love to hear from you:

- Does it work on your board? Which one?
- Did something break? Which YAML, which firmware version, what did the log say?
- Got an idea or a feature wish? Open an issue.
- Got a fix? Even better — pull requests welcome.

Every bit of testing on hardware I don't own helps me make this better for everyone.
This isn't a polished commercial product, it's a hobby project shared in the open,
and your feedback is what shapes where it goes next.

If you'd rather wait until things are more stable, that's totally fine too — just star
the repo and check back in a few months.

---

## What you need

- **ESP32 or ESP32-S3** with **PSRAM** (mandatory — cspot needs ~2 MB for its audio buffer)
- An **I2S DAC or amplifier** (MAX98357, PCM5102, TAS5805M, etc.)
- A **Snapcast server** running somewhere on your network (for the Snapcast side)
- **ESPHome 2026.4+**

ESP32-S3 with 8 MB Octal PSRAM is recommended. Classic ESP32 works but has a 4 MB PSRAM
hardware limit that leaves less headroom.

---

## Quick start

```yaml
external_components:
  - source: github://farmed-switch/esphome-snapspot@main
    components: [snapspot, snapclient, spotify_connect]
    refresh: 1d

psram:
  mode: octal
  speed: 80MHz

wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password

i2s_audio:
  - id: i2s_bus
    i2s_lrclk_pin: GPIO9
    i2s_bclk_pin: GPIO8

speaker:
  - platform: i2s_audio
    id: i2s_output
    i2s_audio_id: i2s_bus
    i2s_dout_pin: GPIO10
    dac_type: external
    channel: stereo
  - platform: mixer
    id: audio_mixer
    output_speaker: i2s_output
    queue_mode: true
    source_speakers:
      - id: cspot_source
      - id: snapclient_source

snapspot:
  mixer_id: audio_mixer
  software_eq: true

media_player:
  - platform: spotify_connect
    name: "Living Room Spotify"

  - platform: snapclient
    name: "Living Room Snapcast"
```

Adjust pins for your board. Flash via the ESPHome add-on in Home Assistant.

---

## Tested server settings

Tested with Snapserver 0.28 on Raspberry Pi. Use FLAC codec for best sync results.

```ini
[stream]
codec = flac
chunk_ms = 20
buffer = 3000
```

---

## Tested hardware

All testing done on [Sonocotta](https://github.com/sonocotta) boards. Other ESP32 boards
with PSRAM and I2S should work — let me know if you try one.

**ESP32-S3 with octal PSRAM is the recommended and well-tested platform today.**
Classic ESP32 boards are still **experimental** — they boot and play, but have
much less PSRAM/SRAM headroom and may need tuning per board.

| Board | SoC | Status |
|---|---|---|
| Amped ESP32-S3 Plus | ESP32-S3 | Works well |
| Amped ESP32-S3 | ESP32-S3 | Works well |
| Louder ESP32-S3 | ESP32-S3 | Works well |
| Louder ESP32-S3 Plus | ESP32-S3 | Works well |
| Amped ESP32 | ESP32 | Experimental — less headroom |
| Louder ESP32 Audio Brick | ESP32 | Experimental — less headroom |

---

## Notes

- FLAC is recommended — PCM may cause sync issues with some Snapserver versions
- ESP32-S3 with octal PSRAM gives the best experience and is the recommended platform
- Classic ESP32 is **experimental** — works but has tight PSRAM/SRAM headroom and is
  more sensitive to Wi-Fi conditions and sdkconfig tuning

---

## Thanks

Built on top of work by people who shared their code:

- [cspot](https://github.com/feelfreelinux/cspot) by feelfreelinux & alufers — Spotify Connect
- [snapclient](https://github.com/CarlosDerSeher/snapclient) by CarlosDerSeher — Snapcast for ESP32
- [snapclient](https://github.com/luar123/snapclient) by luar123 — original ESPHome wrapper
- [ESPHome](https://esphome.io) — the framework that ties it all together
- **Andy** at [Sonocotta](https://github.com/sonocotta) — hardware and testing / mentor support

---

## License

GPL-3.0 — see [LICENSE](LICENSE) and [NOTICE](NOTICE) for details and upstream attributions.
