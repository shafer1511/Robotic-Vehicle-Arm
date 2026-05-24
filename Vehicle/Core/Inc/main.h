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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC
#define M_A_IN1_PWM_Pin GPIO_PIN_8
#define M_A_IN1_PWM_GPIO_Port GPIOA
#define M_A_IN2_PWM_Pin GPIO_PIN_9
#define M_A_IN2_PWM_GPIO_Port GPIOA
#define M_B_IN3_PWM_Pin GPIO_PIN_10
#define M_B_IN3_PWM_GPIO_Port GPIOA
#define M_B_IN4_PWM_Pin GPIO_PIN_11
#define M_B_IN4_PWM_GPIO_Port GPIOA
#define SERVO1_PWM_Pin GPIO_PIN_6
#define SERVO1_PWM_GPIO_Port GPIOB
#define SERVO2_PWM_Pin GPIO_PIN_7
#define SERVO2_PWM_GPIO_Port GPIOB
#define SERVO3_PWM_Pin GPIO_PIN_8
#define SERVO3_PWM_GPIO_Port GPIOB
#define SERVO4_PWM_Pin GPIO_PIN_9
#define SERVO4_PWM_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
