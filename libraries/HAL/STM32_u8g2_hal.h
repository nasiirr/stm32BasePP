#ifndef STM32_U8G2_HAL_H
#define STM32_U8G2_HAL_H

#include "u8g2.h"
#include "stm32f1xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

void STM32_u8g2_Init(u8g2_t *u8g2);
void STM32_u8g2_Test(void);
void STM32_u8g2_DrawTest(u8g2_t *u8g2, GPIO_PinState button_1,
						 GPIO_PinState button_2);
extern int x ;
#ifdef __cplusplus
}
#endif

#endif
