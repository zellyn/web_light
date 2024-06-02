# Web Light

This is an attempt to recreate the simple and cheap XIAO ESP32-C3 /
NeoPixel light seen in [bleeptrack](https://www.bleeptrack.de/)'s
[paper stars video](https://www.youtube.com/watch?v=MRfAjHKRlBU).

They were kind enough to
[reply on mastodon](https://hachyderm.io/@zellyn/111621948922719858) with
hardware details, thus kick-starting my first ESP32 programming attempt.
Apparently I can still solder a bit. Who knew?

# Reference information

## Neopixels

![Through-hole LED form-factor pinout](./img/neopixel-8mm-pins.jpg)

## XIAO ESP32-C3

![pinout](./img/pinout-xiao-esp32-c3.png)

## Sources

- `led_strip` code copied from [led_strip/examples/.../led_strip_rmt_ws2812_main.c](https://github.com/espressif/idf-extra-components/blob/master/led_strip/examples/led_strip_rmt_ws2812/main/led_strip_rmt_ws2812_main.c)
- HSV conversion copied from [examples/peripherals/rmt/led_strip/main/led_strip_example_main.c](https://github.com/espressif/esp-idf/blob/master/examples/peripherals/rmt/led_strip/main/led_strip_example_main.c)
- wifi code copied from [examples/wifi/getting_started/station/main/station_example_main.c](https://github.com/espressif/esp-idf/blob/v5.2.1/examples/wifi/getting_started/station/main/station_example_main.c)
