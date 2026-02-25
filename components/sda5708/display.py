import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components import display
from esphome.const import (
  CONF_ID, 
  CONF_LAMBDA,
  CONF_CLOCK_PIN,
  CONF_DATA_PIN,
  CONF_RESET_PIN,
)


CODEOWNERS = ["@shadow578"]
DEPENDENCIES = []

CONF_LOAD_PIN = "load_pin"

integration_ns = cg.esphome_ns.namespace("sda5708")
SDADisplayComponent = integration_ns.class_(
  "SDA5708Component", 
  display.DisplayBuffer, 
  cg.PollingComponent
)

CONFIG_SCHEMA = (
    display.BASIC_DISPLAY_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(SDADisplayComponent),

            cv.Required(CONF_DATA_PIN): pins.gpio_output_pin_schema,
            cv.Required(CONF_CLOCK_PIN): pins.gpio_output_pin_schema,
            cv.Required(CONF_LOAD_PIN): pins.gpio_output_pin_schema,
            cv.Required(CONF_RESET_PIN): pins.gpio_output_pin_schema,
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

    # rendering
    if CONF_LAMBDA in config:
        lambda_ = await cg.process_lambda(
            config[CONF_LAMBDA], 
            [(SDADisplayComponent.operator("ref"), "it")], 
            return_type=cg.void
        )
        cg.add(var.set_writer(lambda_))
