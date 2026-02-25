import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation, pins
from esphome.components import display
from esphome.const import (
  CONF_ID, 
  CONF_LAMBDA,
  CONF_CLOCK_PIN,
  CONF_DATA_PIN,
  CONF_RESET_PIN,
  CONF_BRIGHTNESS
)
from esphome.core import ID
from esphome.cpp_generator import MockObj, TemplateArgsType
from esphome.types import ConfigType

CODEOWNERS = ["@shadow578"]
DEPENDENCIES = []

CONF_LOAD_PIN = "load_pin"
CONF_LOW_PEAK_CURRENT = "reduce_peak_current"
CONF_CUSTOM_GLYPHS = "custom_glyphs"

CONF_GLYPH_CHAR = "char"
CONF_GLYPH_GLYPH = "glyph"

sda5708_ns = cg.esphome_ns.namespace("sda5708")
SDADisplayComponent = sda5708_ns.class_(
  "SDA5708Component", 
  display.DisplayBuffer, 
  cg.PollingComponent
)

SetBrightnessAction = sda5708_ns.class_("SetBrightnessAction", automation.Action)


def validate_custom_glyph(value):
    # glyphs require a list of strings, each representing a row of the glyph.
    # each string must be exactly 5 characters long, and can only contain ' ' or '#'.
    # the list must contain exactly 7 rows.
    if not isinstance(value, list):
        raise cv.Invalid("Glyph must be a list of strings.")
    if len(value) != 7:
        raise cv.Invalid("Glyph must have exactly 7 rows.")
    
    for row in value:
        if not isinstance(row, str):
            raise cv.Invalid("Each row of the glyph must be a string.")
        if len(row) != 5:
            raise cv.Invalid("Each row of the glyph must be exactly 5 characters long.")
        for char in row:
            if char not in [' ', '#']:
                raise cv.Invalid("Each character in the glyph must be either ' ' or '#'.")

    return value


CUSTOM_GLYPH_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_GLYPH_CHAR): cv.string,
        cv.Required(CONF_GLYPH_GLYPH): cv.All(
            cv.ensure_list(cv.string), 
            validate_custom_glyph
        ),
    }
)

CONFIG_SCHEMA = (
    display.BASIC_DISPLAY_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(SDADisplayComponent),

            cv.Required(CONF_DATA_PIN): pins.gpio_output_pin_schema,
            cv.Required(CONF_CLOCK_PIN): pins.gpio_output_pin_schema,
            cv.Required(CONF_LOAD_PIN): pins.gpio_output_pin_schema,
            cv.Required(CONF_RESET_PIN): pins.gpio_output_pin_schema,

            cv.Optional(CONF_BRIGHTNESS): cv.int_range(min=0, max=7),
            cv.Optional(CONF_LOW_PEAK_CURRENT): cv.boolean,

            cv.Optional(CONF_CUSTOM_GLYPHS): cv.ensure_list(CUSTOM_GLYPH_SCHEMA),
        }
    )
    .extend(cv.polling_component_schema("1s"))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await display.register_display(var, config)

    # pins
    pin_data = await cg.gpio_pin_expression(config[CONF_DATA_PIN])
    pin_clock = await cg.gpio_pin_expression(config[CONF_CLOCK_PIN])
    pin_load = await cg.gpio_pin_expression(config[CONF_LOAD_PIN])
    pin_reset = await cg.gpio_pin_expression(config[CONF_RESET_PIN])

    cg.add(var.set_data_pin(pin_data))
    cg.add(var.set_clock_pin(pin_clock))
    cg.add(var.set_load_pin(pin_load))
    cg.add(var.set_reset_pin(pin_reset))

    # initial control register
    if CONF_LOW_PEAK_CURRENT in config:
        cg.add(var.set_init_peak_current(config[CONF_LOW_PEAK_CURRENT]))
    if CONF_BRIGHTNESS in config:
        cg.add(var.set_init_brightness(config[CONF_BRIGHTNESS]))

    # rendering
    if CONF_LAMBDA in config:
        lambda_ = await cg.process_lambda(
            config[CONF_LAMBDA], 
            [(SDADisplayComponent.operator("ref"), "it")], 
            return_type=cg.void
        )
        cg.add(var.set_writer(lambda_))

    # custom glyphs
    if CONF_CUSTOM_GLYPHS in config:
        for glyph_config in config[CONF_CUSTOM_GLYPHS]:
            char_expr, glyph_expr = await custom_glyph_to_code(glyph_config)
            cg.add(var.get_font().set_glyph(char_expr, glyph_expr))


async def custom_glyph_to_code(config):
    char = config[CONF_GLYPH_CHAR]
    glyph = config[CONF_GLYPH_GLYPH]

    glyph_data = []
    for row in glyph:
        row_data = 0
        for i, c in enumerate(row):
            if c == '#':
                row_data |= (1 << (4 - i))
        glyph_data.append(row_data)
    
    char_expr = cg.RawExpression(f"'{char}'")
    glyph_expr = cg.RawExpression(f"{{{', '.join(f'0b{row:05b}' for row in glyph_data)}}}")
    return char_expr, glyph_expr


@automation.register_action(
    "sda5708.set_brightness",
    SetBrightnessAction,
    cv.maybe_simple_value(
        {
            cv.GenerateID(): cv.use_id(SDADisplayComponent),
            cv.Required(CONF_BRIGHTNESS): cv.templatable(cv.int_range(min=0, max=7)),
        },
        key=CONF_BRIGHTNESS,
    ),
)
async def sda5708_set_brightness_to_code(
    config: ConfigType,
    action_id: ID,
    template_arg: cg.TemplateArguments,
    args: TemplateArgsType,
) -> MockObj:
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    template_ = await cg.templatable(config[CONF_BRIGHTNESS], args, cg.uint8)
    cg.add(var.set_brightness(template_))
    return var
