/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h> // Để dùng strlen, strcpy, strncpy
#include <stdlib.h> // Để dùng atoi
#include <stdbool.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
#define RX_COMMAND_BUFFER_SIZE 32 // Kích thước buffer nhận lệnh
uint8_t uart_rx_char_buffer[1];   // Buffer nhận từng ký tự
uint8_t command_line_buffer[RX_COMMAND_BUFFER_SIZE];
uint8_t command_line_idx = 0;

uint16_t car_motor_speed = 255; // Tốc độ xe (0-255)

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM4_Init(void);
/* USER CODE BEGIN PFP */
void moveForward(uint16_t speed);
void moveBackward(uint16_t speed);
void turnLeft(uint16_t speed);
void turnRight(uint16_t speed);
void stopMotors(void);
void process_command(char* cmd_str);
void set_servo_angle(uint8_t servo_num, uint8_t angle);
long map_value(long x, long in_min, long in_max, long out_min, long out_max); // Cần hàm map này
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_TIM1_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
  // Khởi động PWM cho L298N ENA/ENB
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1); // M_A_IN1_PWM (PA8)
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2); // M_A_IN2_PWM (PA9)
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3); // M_B_IN3_PWM (PA10)
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4); // M_B_IN4_PWM (PA11)

   stopMotors(); // Hàm này sẽ đặt ENA/ENB pulse về 0

   // Khởi động PWM cho 4 Servo
   HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1); // Servo 1 trên PB6
   HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2); // Servo 2 trên PB7
   HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3); // Servo 3 trên PB8
   HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4); // Servo 4 trên PB9

   // Đặt servo về vị trí giữa (90 độ) ban đầu
   set_servo_angle(1, 00);
   set_servo_angle(2, 30);
   set_servo_angle(3, 30);
   set_servo_angle(4, 00);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  // Nhận dữ liệu từ UART (HC-05) từng ký tự một
	   if (HAL_UART_Receive(&huart2, uart_rx_char_buffer, 1, 1) == HAL_OK) { // Timeout nhỏ
	     if (uart_rx_char_buffer[0] == '\n') { // Nếu là ký tự kết thúc lệnh
	       if (command_line_idx > 0) { // Nếu đã có dữ liệu trong buffer
	         command_line_buffer[command_line_idx] = '\0'; // Kết thúc chuỗi

	         // --- DEBUG trên bluetooth terminal  ---
	         char debug_received_line[RX_COMMAND_BUFFER_SIZE + 30];
	         sprintf(debug_received_line, "Car RX Line: [%s] (len:%d)\r\n", (char*)command_line_buffer, strlen((char*)command_line_buffer));
	         HAL_UART_Transmit(&huart2, (uint8_t*)debug_received_line, strlen(debug_received_line), 100);
	         // ------------------------

	         process_command((char*)command_line_buffer);
	         command_line_idx = 0; // Reset buffer cho lệnh tiếp theo
	       } else {
	         // Nhận được \n rỗng, có thể bỏ qua hoặc reset command_line_idx
	         command_line_idx = 0;
	       }
	     } else if (command_line_idx < RX_COMMAND_BUFFER_SIZE - 1) {
	       if(uart_rx_char_buffer[0] != '\r'){ // Bỏ qua ký tự \r nếu có
	            command_line_buffer[command_line_idx++] = uart_rx_char_buffer[0];
	       }
	     } else {
	       // Tràn buffer, reset
	       HAL_UART_Transmit(&huart2, (uint8_t*)"RX Buffer Overflow!\r\n", 20, 100);
	       command_line_idx = 0;
	     }
	     }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 71;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 999;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 359;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 1999;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 150;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.Pulse = 0;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */
  HAL_TIM_MspPostInit(&htim4);

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
#define MAX_MOTOR_PWM 999
void moveForward(uint16_t speed) {
    uint16_t pwm_value = map_value(speed, 0, 255, 0, MAX_MOTOR_PWM);

    // Motor A tiến: IN1 có PWM, IN2 là LOW (PWM=0)
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pwm_value); // IN1 (PA0)
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);         // IN2 (PA1)

    // Motor B tiến: IN3 có PWM, IN4 là LOW (PWM=0)
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, pwm_value); // IN3 (PA6)
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 0);         // IN4 (PA7)
}

void moveBackward(uint16_t speed) {
    uint16_t pwm_value = map_value(speed, 0, 255, 0, MAX_MOTOR_PWM);

    // Motor A lùi: IN1 là LOW, IN2 có PWM
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);         // IN1 (PA0)
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, pwm_value); // IN2 (PA1)

    // Motor B lùi: IN3 là LOW, IN4 có PWM
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 0);         // IN3 (PA6)
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, pwm_value); // IN4 (PA7)
}

void turnLeft(uint16_t speed) {
    uint16_t pwm_value = map_value(speed, 0, 255, 0, MAX_MOTOR_PWM);

    // Rẽ trái: Motor phải (A) tiến, Motor trái (B) dừng
    // Motor A tiến
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pwm_value);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);
    // Motor B lùi
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 0);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 0);
}

void turnRight(uint16_t speed) {
    uint16_t pwm_value = map_value(speed, 0, 255, 0, MAX_MOTOR_PWM);

    // Rẽ phải: Motor trái (B) tiến, Motor phải (A) dừng
    // Motor A lùi
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2,0);
    // Motor B tiến
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, pwm_value);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 0);
}

void stopMotors() {
    // Dừng cả 2 motor bằng cách đặt tất cả PWM về 0 (phanh động)
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0); // IN1
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0); // IN2
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 0); // IN3
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 0); // IN4
}
long map_value(long x, long in_min, long in_max, long out_min, long out_max) {
  if (in_max == in_min) return out_min; // Tránh chia cho 0
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// servo_num: 1 đến 4
// angle: 0 đến 180
// Giả sử TIM3 được dùng cho cả 4 servo
// Kênh 1 cho Servo1, Kênh 2 cho Servo2, ...
void set_servo_angle(uint8_t servo_num, uint8_t angle) {
    if (angle > 180) angle = 180; // Giới hạn góc

    // Giả sử APB1 Timer Clock = 36MHz, PSC=359 (tick 10µs), ARR=1999
    // Pulse 1ms (0 deg) -> CCR = 100
    // Pulse 2ms (180 deg) -> CCR = 200
    uint16_t pulse_min = 70;
    uint16_t pulse_max = 420;
    uint16_t pulse_value = map_value(angle, 0, 180, pulse_min, pulse_max);

    uint32_t channel = 0;
    switch (servo_num) {
        case 1: channel = TIM_CHANNEL_1; break;
        case 2: channel = TIM_CHANNEL_2; break;
        case 3: channel = TIM_CHANNEL_3; break;
        case 4: channel = TIM_CHANNEL_4; break;
        default: return; // Servo không hợp lệ
    }
    __HAL_TIM_SET_COMPARE(&htim4, channel, pulse_value);
}
void process_command(char* cmd_str) {
    if (cmd_str == NULL || strlen(cmd_str) == 0) {
        return;
    }

    //HAL_UART_Transmit(&huart2, (uint8_t*)cmd_str, strlen(cmd_str), 100); // Echo lại để debug
    //HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n", 2, 100);


    // Xử lý lệnh điều khiển xe (ký tự đơn)
    if (strlen(cmd_str) == 1) {
        switch (cmd_str[0]) {
            case 'F':
            	HAL_GPIO_TogglePin(GPIOC, LED_Pin);
                moveForward(car_motor_speed); // Giả sử bạn có hàm này
                HAL_UART_Transmit(&huart2, (uint8_t*)"Car: FWD\r\n", 10, 100);
                break;
            case 'B':
            	HAL_GPIO_TogglePin(GPIOC, LED_Pin);
                moveBackward(car_motor_speed);
                HAL_UART_Transmit(&huart2, (uint8_t*)"Car: BWD\r\n", 10, 100);
                break;
            case 'L':
            	HAL_GPIO_TogglePin(GPIOC, LED_Pin);
                turnLeft(car_motor_speed);
                HAL_UART_Transmit(&huart2, (uint8_t*)"Car: LEFT\r\n", 11, 100);
                break;
            case 'R':
            	HAL_GPIO_TogglePin(GPIOC, LED_Pin);
                turnRight(car_motor_speed);
                HAL_UART_Transmit(&huart2, (uint8_t*)"Car: RIGHT\r\n", 12, 100);
                break;
            case 'S':
                stopMotors();
                HAL_UART_Transmit(&huart2, (uint8_t*)"Car: STOP\r\n", 11, 100);
                break;
            default:
                // Lệnh xe không xác định
                break;
        }
    }
    // Xử lý lệnh điều khiển servo (bắt đầu bằng 'V', theo sau là 12 chữ số)
    // Ví dụ: "V090120045180"
    else if (cmd_str[0] == 'V' && strlen(cmd_str) == 13) { // 'V' + 4 * 3 digits
        char angle_substr[4]; // 3 digits + null terminator
        uint8_t s_angles[4];
        bool parse_ok = true;

        for (int i = 0; i < 4; i++) {
            strncpy(angle_substr, &cmd_str[1 + i * 3], 3);
            angle_substr[3] = '\0';
            s_angles[i] = atoi(angle_substr);
            if (s_angles[i] > 180) { // Kiểm tra giá trị góc hợp lệ
                 // s_angles[i] = 180; // Hoặc báo lỗi
                 parse_ok = false; break;
                 HAL_GPIO_TogglePin(GPIOC,LED_Pin);
            }
        }

        if(parse_ok){
            set_servo_angle(1, s_angles[0]);
            set_servo_angle(2, s_angles[1]);
            set_servo_angle(3, s_angles[2]);
            set_servo_angle(4, s_angles[3]);

            // (Tùy chọn) Gửi lại xác nhận hoặc giá trị góc đã nhận để debug
            // char debug_msg[50];
            // sprintf(debug_msg, "Servo set: %03d %03d %03d %03d\r\n", s_angles[0],s_angles[1],s_angles[2],s_angles[3]);
            // HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), 100);
        } else {
            HAL_UART_Transmit(&huart2, (uint8_t*)"Servo Angle Error\r\n", 19, 100);
        }

    } else {
        // Lệnh không xác định
        // char unk_msg[30];
        // sprintf(unk_msg, "Unknown CMD: %s\r\n", cmd_str);
        // HAL_UART_Transmit(&huart2, (uint8_t*)unk_msg, strlen(unk_msg), 100);
    }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
