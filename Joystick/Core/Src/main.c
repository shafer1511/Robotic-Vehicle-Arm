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
#include <stdbool.h>
#include <string.h>
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
ADC_HandleTypeDef hadc1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
// Joystick 1 (điều khiển xe)
uint16_t adc_joy1_y_val;
uint16_t adc_joy1_x_val;
char car_command = 'S';
char last_car_command = ' ';

// Joystick 2 (điều khiển cánh tay)
uint16_t adc_joy2_y_val;
uint16_t adc_joy2_x_val;

// Mảng lưu góc của 4 servo (0-180 độ)
uint8_t servo_angles[4] = {00, 30, 30, 00}; // Khởi tạo góc giữa cho các servo
uint8_t last_servo_angles[4] = {0, 0, 0, 0}; // Để kiểm tra thay đổi

// Tốc độ tăng/giảm góc servo khi nhấn nút
uint8_t servo_increment_speed = 4;

char servo_command_buffer[30];
// Biến cho nút giữ tay gắp(Interuppt)
volatile uint8_t gripper_hold_btn_flag = 0; // Cờ báo được bật bởi hàm ngắt
uint8_t gripper_hold_active = 0;          // Biến trạng thái được quản lý bởi main loop
int16_t servo4_held_angle = 90;           // Lưu góc của servo 4 khi giữ

// Biến dùng để chống dội bằng thời gian trong hàm ngắt
uint32_t last_interrupt_time = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */
uint16_t read_adc_channel(uint32_t channel);
long map_value(long x, long in_min, long in_max, long out_min, long out_max);
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
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
  MX_ADC1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  char* msg_controller_ready = "Controller V3 Ready (2 Joysticks)\r\n";
  HAL_StatusTypeDef tx_status; // Khai báo biến để lưu trạng thái
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET); // LED ON
  tx_status =  HAL_UART_Transmit(&huart2, (uint8_t*)msg_controller_ready, strlen(msg_controller_ready), HAL_MAX_DELAY);
  if (tx_status == HAL_OK) {
      // Gửi thành công(Tắt LED
       HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET); // LED OFF
  } else {
      // Gửi thất bại(Giữ LED sáng hoặc cho nháy nhanh để báo lỗi)
      while(1) {
          HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
          HAL_Delay(100);
      }
  }
    // Gửi trạng thái servo ban đầu một lần
    sprintf(servo_command_buffer, "V%03d%03d%03d%03d\n", servo_angles[0], servo_angles[1], servo_angles[2], servo_angles[3]);
    HAL_UART_Transmit(&huart2, (uint8_t*)servo_command_buffer, strlen(servo_command_buffer), 100);
    memcpy(last_servo_angles, servo_angles, sizeof(servo_angles)); // Cập nhật trạng thái đã gửi
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  // 0. KIỂM TRA VÀ XỬ LÝ CỜ BÁO TỪ NGẮT NÚT NHẤN
	     if (gripper_hold_btn_flag) {
	         gripper_hold_btn_flag = 0; // Hạ cờ ngay để tránh xử lý nhiều lần
	         HAL_UART_Transmit(&huart2, (uint8_t*)"Gripper Hold: OFF\r\n", 19, 100);

	         // Đảo trạng thái giữ của tay gắp
	         gripper_hold_active = !gripper_hold_active;

	         if (gripper_hold_active) {
	        	 HAL_GPIO_TogglePin(GPIOC,LED_Pin);
	        	 HAL_UART_Transmit(&huart2, (uint8_t*)"Gripper Hold: ON\r\n", 18, 100);
	             // Khi BẬT chế độ giữ, lưu lại góc hiện tại của Servo 4
	             servo4_held_angle = servo_angles[3];
	         }
	         // Khi TẮT, servo 4 sẽ tự động nhận giá trị từ joystick ở logic bên dưới
	     }

	  // 1. Đọc Joystick 1 và gửi lệnh điều khiển xe
	  adc_joy1_y_val = read_adc_channel(ADC_CHANNEL_0); // JOY1_Y (PA0)
	  adc_joy1_x_val = read_adc_channel(ADC_CHANNEL_1); // JOY1_X (PA1)

	  uint16_t dead_zone_low = 800;
	  uint16_t dead_zone_high = 3200;
	  char current_car_cmd_temp = 'S';
	  if (adc_joy1_y_val > dead_zone_high)
		  {
		  	  current_car_cmd_temp = 'F';
		  	  HAL_GPIO_TogglePin(GPIOC, LED_Pin);
		  }
	  else if (adc_joy1_y_val < dead_zone_low)
		  {
		  current_car_cmd_temp = 'B';
		  HAL_GPIO_TogglePin(GPIOC, LED_Pin);
		  }
	  else if (adc_joy1_x_val > dead_zone_high)
		  {
		  current_car_cmd_temp = 'R';
		  HAL_GPIO_TogglePin(GPIOC, LED_Pin);
		  }
	  else if (adc_joy1_x_val < dead_zone_low)
		  {
		  current_car_cmd_temp = 'L';
		  HAL_GPIO_TogglePin(GPIOC, LED_Pin);
		  }

	  if (current_car_cmd_temp != car_command) {
	       car_command = current_car_cmd_temp;
	  }

	  if (car_command != last_car_command) {
	    char car_cmd_str_to_send[3]; // Đủ cho 'F', '\n', '\0'
	    sprintf(car_cmd_str_to_send, "%c\n", car_command);
	    HAL_UART_Transmit(&huart2, (uint8_t*)car_cmd_str_to_send, strlen(car_cmd_str_to_send), HAL_MAX_DELAY);
	    last_car_command = car_command;
	    HAL_Delay(100);
	  }

	  // 2. XỬ LÝ NÚT NHẤN CHO SERVO 1 & 2
	    // Servo 1 (Đế xoay)
	    if (HAL_GPIO_ReadPin(S1_LEFT_BTN_GPIO_Port, S1_LEFT_BTN_Pin) == GPIO_PIN_RESET) { // Nhấn nút Quay Trái
	        if (servo_angles[0] > 0) { // Nếu chưa phải góc nhỏ nhất
	            servo_angles[0] -= servo_increment_speed;
	        }
	    }
	    if (HAL_GPIO_ReadPin(S1_RIGHT_BTN_GPIO_Port, S1_RIGHT_BTN_Pin) == GPIO_PIN_RESET) { // Nhấn nút Quay Phải
	        if (servo_angles[0] < 180) { // Nếu chưa phải góc lớn nhất
	            servo_angles[0] += servo_increment_speed;
	        }
	    }

	    // Servo 2 (Khớp vai)
	    if (HAL_GPIO_ReadPin(S2_UP_BTN_GPIO_Port, S2_UP_BTN_Pin) == GPIO_PIN_RESET) { // Nhấn nút Lên
	        if (servo_angles[1] < 180) {
	            servo_angles[1] += servo_increment_speed;
	        }
	    }
	    if (HAL_GPIO_ReadPin(S2_DOWN_BTN_GPIO_Port, S2_DOWN_BTN_Pin) == GPIO_PIN_RESET) { // Nhấn nút Xuống
	        if (servo_angles[1] > 0) {
	            servo_angles[1] -= servo_increment_speed;
	        }
	    }

	    // Giới hạn giá trị servo_angles[0] và [1] trong khoảng 0-180 (đề phòng)
	    if(servo_angles[0] < 0) servo_angles[0] = 0;
	    if(servo_angles[0] > 180) servo_angles[0] = 180;
	    if(servo_angles[1] < 0) servo_angles[1] = 0;
	    if(servo_angles[1] > 180) servo_angles[1] = 180;

	    // 3. Đọc Joystick 2 để điều khiển Servo 3 & 4
	    adc_joy2_y_val = read_adc_channel(ADC_CHANNEL_4); // JS2_Y (PA4)
	    adc_joy2_x_val = read_adc_channel(ADC_CHANNEL_5); // JS2_X (PA5)

	    // Servo 3 (Khớp khuỷu) được điều khiển bởi trục Y của JS2
	    servo_angles[2] = map_value(adc_joy2_y_val, 0, 4095, 0, 180);

	    // Servo 4 (Tay gắp) được điều khiển bởi trục X của JS2
	    servo_angles[3] = map_value(adc_joy2_x_val, 0, 4095, 30, 120); // Ví dụ dải góc cho tay gắp
	    if(servo_angles[3] > 40) servo_angles[3] = 40;
	    if(servo_angles[3] < 0) servo_angles[3] = 00;

	       // CẬP NHẬT SERVO 4 (TAY GẮP) DỰA TRÊN TRẠNG THÁI GIỮ
	       if (gripper_hold_active) {
	           servo_angles[3] = servo4_held_angle; // Sử dụng góc đã lưu
	       } else {
	           servo_angles[3] = map_value(adc_joy2_x_val, 0, 4095, 30, 120); // Điều khiển bằng joystick
	       }
	       if(servo_angles[3] > 40) servo_angles[3] = 40;
	       if(servo_angles[3] < 0) servo_angles[3] = 00;

	    // 4. Gửi lệnh servo nếu có bất kỳ góc nào thay đổi (logic này giữ nguyên)
	    bool servo_data_changed = false;
	    for (int i = 0; i < 4; i++) {
	        // Ép kiểu về uint8_t để so sánh với last_servo_angles
	        if ((uint8_t)servo_angles[i] != last_servo_angles[i]) {
	            servo_data_changed = true;
	            break;
	        }
	    }

	    if (servo_data_changed) {
	        // Ép kiểu về uint8_t khi gửi đi để đảm bảo đúng định dạng %03d
	        sprintf(servo_command_buffer, "V%03u%03u%03u%03u\n",
	                (uint8_t)servo_angles[0], (uint8_t)servo_angles[1],
	                (uint8_t)servo_angles[2], (uint8_t)servo_angles[3]);
	        HAL_UART_Transmit(&huart2, (uint8_t*)servo_command_buffer, strlen(servo_command_buffer), 100);

	        // Cập nhật mảng last_servo_angles
	        for (int i=0; i<4; i++) {
	            last_servo_angles[i] = (uint8_t)servo_angles[i];
	        }
	    }
	  // Logic gửi lệnh 'S' cho xe nếu không có input (giữ nguyên hoặc tùy chỉnh)
	  if (car_command == 'S' && last_car_command != 'S' && !servo_data_changed) {
	       HAL_UART_Transmit(&huart2, (uint8_t*)&car_command, 3, HAL_MAX_DELAY);
	       last_car_command = car_command;
	  }

	  HAL_Delay(50);

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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

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

  /*Configure GPIO pins : S1_LEFT_BTN_Pin S1_RIGHT_BTN_Pin S2_UP_BTN_Pin S2_DOWN_BTN_Pin */
  GPIO_InitStruct.Pin = S1_LEFT_BTN_Pin|S1_RIGHT_BTN_Pin|S2_UP_BTN_Pin|S2_DOWN_BTN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : GRIPPER_HOLD_BTN_Pin */
  GPIO_InitStruct.Pin = GRIPPER_HOLD_BTN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GRIPPER_HOLD_BTN_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
long map_value(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Hàm read_adc_channel vẫn giữ nguyên như trước
uint16_t read_adc_channel(uint32_t channel) {
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = channel;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_71CYCLES_5; // Hoặc thời gian lấy mẫu khác

    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
        Error_Handler();
    }

    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 100);
    uint16_t adc_value = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);

    return adc_value;
}
// Hàm callback sẽ được gọi mỗi khi có ngắt ngoài từ một chân GPIO
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  // Kiểm tra xem ngắt có phải từ chân nút giữ tay gắp không
  if (GPIO_Pin == GRIPPER_HOLD_BTN_Pin) // GRIPPER_HOLD_BTN_Pin được CubeMX tự định nghĩa trong main.h
  {
    // Kỹ thuật chống dội bằng thời gian (non-blocking)
    uint32_t current_time = HAL_GetTick();
    if (current_time - last_interrupt_time > 250) // Chỉ chấp nhận ngắt mới sau 250ms
    {
      // Đây là một lần nhấn hợp lệ, chỉ cần đặt cờ báo cho vòng lặp main xử lý
      gripper_hold_btn_flag = 1;

      last_interrupt_time = current_time; // Cập nhật thời gian của lần nhấn hợp lệ
    }
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
