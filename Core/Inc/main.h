/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define STEPPER_STEPS_Pin GPIO_PIN_0
#define STEPPER_STEPS_GPIO_Port GPIOA
#define STEPPER_DIR_Pin GPIO_PIN_1
#define STEPPER_DIR_GPIO_Port GPIOA
#define LOCK_Pin GPIO_PIN_2
#define LOCK_GPIO_Port GPIOA
#define MAX_SPEED_Pin GPIO_PIN_3
#define MAX_SPEED_GPIO_Port GPIOA
#define SPI_RST_Pin GPIO_PIN_4
#define SPI_RST_GPIO_Port GPIOA
#define SPI_DC_Pin GPIO_PIN_6
#define SPI_DC_GPIO_Port GPIOA
#define EXT_ADC_Pin GPIO_PIN_0
#define EXT_ADC_GPIO_Port GPIOB
#define BUZZER_Pin GPIO_PIN_1
#define BUZZER_GPIO_Port GPIOB
#define EXT_DIR_Pin GPIO_PIN_12
#define EXT_DIR_GPIO_Port GPIOA
#define EXT_START_Pin GPIO_PIN_15
#define EXT_START_GPIO_Port GPIOA
#define DIR_Pin GPIO_PIN_3
#define DIR_GPIO_Port GPIOB
#define START_Pin GPIO_PIN_4
#define START_GPIO_Port GPIOB
#define SELECT_Pin GPIO_PIN_5
#define SELECT_GPIO_Port GPIOB
#define UP_Pin GPIO_PIN_6
#define UP_GPIO_Port GPIOB
#define DOWN_Pin GPIO_PIN_7
#define DOWN_GPIO_Port GPIOB
#define MENU_Pin GPIO_PIN_8
#define MENU_GPIO_Port GPIOB
#define MODE_Pin GPIO_PIN_9
#define MODE_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
