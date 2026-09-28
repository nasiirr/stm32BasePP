#include "STM32_u8g2_hal.h"
#include "main.h"
#include "u8g2.h"

extern SPI_HandleTypeDef hspi1;

static uint8_t u8g2_spi_byte_callback(u8x8_t *u8x8, uint8_t message,
                                       uint8_t argument_int, void *argument_ptr)
{
    (void)u8x8;

    switch (message)
    {
    case U8X8_MSG_BYTE_INIT:
        return 1;

    case U8X8_MSG_BYTE_SEND:
        return HAL_SPI_Transmit(&hspi1, (uint8_t *)argument_ptr,
                                argument_int, HAL_MAX_DELAY) == HAL_OK;

    case U8X8_MSG_BYTE_SET_DC:
        HAL_GPIO_WritePin(SPI_DC_GPIO_Port, SPI_DC_Pin,
                          argument_int ? GPIO_PIN_SET : GPIO_PIN_RESET);
        return 1;

    case U8X8_MSG_BYTE_START_TRANSFER:
        return 1;

    case U8X8_MSG_BYTE_END_TRANSFER:
        return 1;

    default:
        return 1;
    }
}

static uint8_t u8g2_gpio_and_delay_callback(u8x8_t *u8x8, uint8_t message,
                                             uint8_t argument_int, void *argument_ptr)
{
    (void)u8x8;
    (void)argument_ptr;

    switch (message)
    {
    case U8X8_MSG_GPIO_AND_DELAY_INIT:
        HAL_GPIO_WritePin(SPI_RST_GPIO_Port, SPI_RST_Pin, GPIO_PIN_SET);
        return 1;

    case U8X8_MSG_GPIO_RESET:
        HAL_GPIO_WritePin(SPI_RST_GPIO_Port, SPI_RST_Pin,
                          argument_int ? GPIO_PIN_SET : GPIO_PIN_RESET);
        return 1;

    case U8X8_MSG_GPIO_DC:
        HAL_GPIO_WritePin(SPI_DC_GPIO_Port, SPI_DC_Pin,
                          argument_int ? GPIO_PIN_SET : GPIO_PIN_RESET);
        return 1;

    case U8X8_MSG_DELAY_MILLI:
        HAL_Delay(argument_int);
        return 1;

    default:
        return 1;
    }
}

void STM32_u8g2_Init(u8g2_t *u8g2)
{
    HAL_GPIO_WritePin(SPI_RST_GPIO_Port, SPI_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(20);
    HAL_GPIO_WritePin(SPI_RST_GPIO_Port, SPI_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(20);

    u8g2_Setup_ssd1309_128x64_noname0_f(u8g2, U8G2_R2,
                                        u8g2_spi_byte_callback,
                                        u8g2_gpio_and_delay_callback);
    u8g2_InitDisplay(u8g2);
    u8g2_SetPowerSave(u8g2, 0);
}

// void STM32_u8g2_Test(void)
// {
//     u8g2_t u8g2;

//     STM32_u8g2_Init(&u8g2);
//     STM32_u8g2_DrawTest(&u8g2, GPIO_PIN_RESET, GPIO_PIN_RESET);
// }
//  int x = 0;
// void STM32_u8g2_DrawTest(u8g2_t *u8g2, GPIO_PinState button_1,
//                          GPIO_PinState button_2)
// {
//     u8g2_ClearBuffer(u8g2);
//     u8g2_SetFont(u8g2, u8g2_font_6x10_tf);
//     u8g2_DrawStr(u8g2, x, 10, "SPI1 u8g2 OK");
//     u8g2_DrawStr(u8g2, x, 26, button_1 == GPIO_PIN_SET ? "B10: PRESSED" : "B10: RELEASED");
//     u8g2_DrawStr(u8g2, x, 42, button_2 == GPIO_PIN_SET ? "B11: PRESSED" : "B11: RELEASED");
//     u8g2_DrawStr(u8g2, x, 58, "LEDs follow buttons");
//     u8g2_SendBuffer(u8g2);

// }
