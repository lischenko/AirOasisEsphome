import esphome.codegen as cg
from esphome.components import fan, uart
import esphome.config_validation as cv
from esphome.const import CONF_UART_ID

CODEOWNERS = []
DEPENDENCIES = ["uart"]

AOIAPurifier = cg.esphome_ns.namespace("aoia_purifier").class_(
    "AOIAPurifier", cg.Component, fan.Fan
)

CONFIG_SCHEMA = (
    fan.fan_schema(AOIAPurifier, default_restore_mode="NO_RESTORE")
    .extend(
        {
            cv.Required(CONF_UART_ID): cv.use_id(uart.UARTComponent),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    var = await fan.new_fan(config)
    await cg.register_component(var, config)

    uart_component = await cg.get_variable(config[CONF_UART_ID])
    cg.add(var.set_uart(uart_component))
