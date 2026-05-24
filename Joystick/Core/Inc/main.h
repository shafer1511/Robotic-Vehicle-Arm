/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC
#define JOYSTICK_Y_Pin GPIO_PIN_0
#define JOYSTICK_Y_GPIO_Port GPIOA
#define JOYSTICK_X_Pin GPIO_PIN_1
#define JOYSTICK_X_GPIO_Port GPIOA
#define JOY2_Y_Pin GPIO_PIN_4
#define JOY2_Y_GPIO_Port GPIOA
#define JOY2_X_Pin GPIO_PIN_5
#define JOY2_X_GPIO_Port GPIOA
#define S1_LEFT_BTN_Pin GPIO_PIN_3
#define S1_LEFT_BTN_GPIO_Port GPIOB
#define S1_RIGHT_BTN_Pin GPIO_PIN_4
#define S1_RIGHT_BTN_GPIO_Port GPIOB
#define S2_UP_BTN_Pin GPIO_PIN_5
#define S2_UP_BTN_GPIO_Port GPIOB
#define S2_DOWN_BTN_Pin GPIO_PIN_6
#define S2_DOWN_BTN_GPIO_Port GPIOB
#define GRIPPER_HOLD_BTN_Pin GPIO_PIN_7
#define GRIPPER_HOLD_BTN_GPIO_Port GPIOB
#define GRIPPER_HOLD_BTN_EXTI_IRQn EXTI9_5_IRQn

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
