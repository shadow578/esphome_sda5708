# ESPHome Component for Siemens SDA5708-24 LED Matrix Display

this component adds support for driving a Siemens SDA5708-24 LED matrix display using ESPHome.
implementation is based on the details found on [SB-Projects' SDA5708 page](https://www.sbprojects.net/knowledge/footprints/sda5708/index.php) ([mirror](https://web.archive.org/web/20260225174854/https://www.sbprojects.net/knowledge/footprints/sda5708/index.php)).

## Wiring

| ![SDA5708 Pinout](./_img/connector.png) |
| --------------------------------------- |
| SDA5708 Pinout, by SB-Projects          |

The SDA5708-24 comes with a connector (Molex type 51065-0600) with six pins. The pinout is as follows:

| Pin # | Name    | Notes                        |
| ----- | ------- | ---------------------------- |
| 1     | Vcc     | +5V Power Supply             |
| 2     | #LOAD   | Data load/latch (active low) |
| 3     | DATA    | Serial data                  |
| 4     | SDCLOCK | Serial clock                 |
| 5     | #RESET  | display reset (active low)   |
| 6     | GND     |                              |


Connect `Vcc` and `GND` to the +5V and GND of your mcu's power supply.
The `#LOAD`, `DATA`, `SDCLOCK`, and `#RESET` pins can be connected to any available GPIOs on your mcu.
3.3V GPIOs work fine, but 5V is recommended.


## Usage

To use this component, add the following to your ESPHome configuration:

```yaml
display:
  - platform: sda5708
    data_pin: GPIO3
    clock_pin: GPIO13
    load_pin: GPIO12
    reset_pin: GPIO14
    lambda: |-
      it.print("Hello!");
```

### Configuration Variables

- __data_pin__ (__Required__, pin): The GPIO pin connected to the SDA5708's `DATA` pin.
- __clock_pin__ (__Required__, pin): The GPIO pin connected to the SDA5708's `SDCLOCK` pin.
- __load_pin__ (__Required__, pin): The GPIO pin connected to the SDA5708's `#LOAD` pin.
- __reset_pin__ (__Required__, pin): The GPIO pin connected to the SDA5708's `#RESET` pin.
- __brightness__ (Optional, int): (Initial) brightness level (0-7). Can be overwritten at runtime using the `set_brightness` method or the `sda5708.set_brightness` action.
- __peak_current__ (Optional, bool): Whether to limit the peak current (to ~12.5%). It is recommended to leave this at default, as limiting peak current may cause flickering.
- __custom_glyphs__ (Optional, list of glyphs): A list of custom glyphs. Each entry is defined as follows:
  - __char__ (Required, string): The character to associate with the glyph. Must be a single character _or_ a C-style escaped character (e.g. `\1`,).
  - __glyph__ (Required, list of strings): A list of 7 strings, each representing a row of the glyph. Each string must be exactly 5 characters long.`#` represents an on pixel and ` ` (space) represents an off pixel.
- __update_interval__ (Optional, time): The interval to re-draw the screen. Defaults to `1s`.
- __lambda__ (Optional, lambda): A lambda to use for rendering the content on the display. Similar to [LCD Display](https://esphome.io/components/display/lcd_display).


#### Actions

##### `sda5708.set_brightness` Action

Set the brightness of the display. The brightness level can be an integer from 0 (off) to 7 (max brightness).

- __brightness__ (Required, int): The brightness level to set (0-7).
