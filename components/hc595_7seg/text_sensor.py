import esphome.codegen as cg
from esphome.components import spi, text_sensor
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_LAMBDA, CONF_UPDATE_INTERVAL
from esphome.core import CORE

DEPENDENCIES = ["spi"]

hc595_7seg_ns = cg.esphome_ns.namespace("hc595_7seg")
HC5957Segment = hc595_7seg_ns.class_(
    "HC5957Segment",
    text_sensor.TextSensor,
    cg.PollingComponent,
    spi.SPIDevice,
)
CONF_SEGMENT_ACTIVE_LOW = "segment_active_low"
CONF_DIGIT_ACTIVE_HIGH = "digit_active_high"

CONFIG_SCHEMA = (
    text_sensor.text_sensor_schema()
    .extend(
        {
            cv.GenerateID(): cv.declare_id(HC5957Segment),
            cv.Required(CONF_LAMBDA): cv.returning_lambda,
            cv.Optional(CONF_SEGMENT_ACTIVE_LOW, default=True): cv.boolean,
            cv.Optional(CONF_DIGIT_ACTIVE_HIGH, default=True): cv.boolean,
        }
    )
    .extend(cv.polling_component_schema("1s"))
    .extend(spi.spi_device_schema(cs_pin_required=True))
)


async def to_code(config):
    var = await text_sensor.new_text_sensor(config)
    await cg.register_component(var, config)
    await spi.register_spi_device(var, config, write_only=True)

    text_lambda = await cg.process_lambda(
        config[CONF_LAMBDA],
        [],
        return_type=cg.optional.template(cg.std_string),
    )
    cg.add(var.set_text_lambda(text_lambda))
    cg.add(var.set_segment_active_low(config[CONF_SEGMENT_ACTIVE_LOW]))
    cg.add(var.set_digit_active_high(config[CONF_DIGIT_ACTIVE_HIGH]))
