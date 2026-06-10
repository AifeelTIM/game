/* USER CODE BEGIN Header */
/* TEST: auto push hook demo */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "hrtim.h"
#include "i2c.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include"oled.h"
#include"stdio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define IIR_ALPHA 0.0005f
#define ARR 34000
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint32_t adc_buffer[4];
int a = 0;
int n = 0;
float u1 = 0.0;
float value = 0.0;
float value2 = 0.0;
float value3 = 0.0;
float value4 = 0.0;
float err = 0.0;
float err2 = 0.0;
float CCR = 17000;
float CCR2 = 17000;
float tarvoltage = 15000;
float tarcurrent = 1980;
float kp = 0.0001;
float kp2 = 0.0005;
char message[50];
char message2[50];
char message3[50];
char message4[50];
char message5[50];
char message6[50];
char message7[50];
float disp_u1 = 0.0f;
float disp_i1 = 0.0f;
float disp_u0 = 0.0f;
float disp_i0 = 0.0f;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

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
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_TIM1_Init();
  MX_HRTIM1_Init();
  /* USER CODE BEGIN 2 */
  HAL_Delay(100);
  OLED_Init();

  //HAL_Delay(200);
  HAL_TIM_Base_Start(&htim1);
  // HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  // HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
  // HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
  // HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);

  /* Enable outputs BEFORE starting counters (ST recommended order) */
  HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TC1 | HRTIM_OUTPUT_TC2);
  HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TD1 | HRTIM_OUTPUT_TD2);
  HAL_HRTIM_WaveformCountStart(&hhrtim1 , HRTIM_TIMERID_TIMER_C);
  HAL_HRTIM_WaveformCountStart(&hhrtim1 , HRTIM_TIMERID_TIMER_D);

  /* Set initial fixed duty before enabling ADC ISR feedback */
  HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].CMP1xR = 17000;
  HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_D].CMP1xR = 17000;

  HAL_ADC_Start_DMA(&hadc1, (uint32_t *)(adc_buffer), 4);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    disp_u1 += IIR_ALPHA * (value - disp_u1);
    disp_i1 += IIR_ALPHA * (value2 - disp_i1);
    disp_u0 += IIR_ALPHA * (value3 - disp_u0);
    disp_i0 += IIR_ALPHA * (value4 - disp_i0);
    a++;
    if (a >= 500) {
      OLED_NewFrame();
      sprintf(message,"U1: %.2f",disp_u1 / 1000) ;
      sprintf(message2,"I1: %.2f",(disp_i1 / 1000 + 2.02) / 2 );
      sprintf(message3,"U0: %.2f",disp_u0 / 1000) ;
      sprintf(message4,"I0: %.2f",disp_i0 / 1000 + 0.01) ;
      sprintf(message5,"P1: %.2f", disp_u1 /1000 * (disp_i1 / 1000 + 2.02) / 2) ;
      OLED_PrintString(1, 1, message, &font16x16, OLED_COLOR_NORMAL);
      OLED_PrintString(1, 15, message2, &font16x16, OLED_COLOR_NORMAL);
      OLED_PrintString(1, 29, message3, &font16x16, OLED_COLOR_NORMAL);
      OLED_PrintString(1, 45, message4, &font16x16, OLED_COLOR_NORMAL);
      OLED_PrintString(65, 1, message5, &font16x16, OLED_COLOR_NORMAL);
      OLED_ShowFrame();
      a = 0;
    }


     // float_data[0] = a;     // 电压读数
     // float_data[1] = CCR;       // PI 输出
     // float_data[2] = value2;       // 误差
     // JustFloat_Send(float_data, 3, JUSTFLOAT_TYPE_FLOAT);
     if(HAL_GPIO_ReadPin(KEY_3_GPIO_Port, KEY_3_Pin) == GPIO_PIN_RESET)
     {
       HAL_Delay(20);
       while(HAL_GPIO_ReadPin(KEY_3_GPIO_Port, KEY_3_Pin) == GPIO_PIN_RESET)
       {

       }
       HAL_Delay(20);
       tarcurrent += 10;
     }
     if(HAL_GPIO_ReadPin(KEY_4_GPIO_Port, KEY_4_Pin) == GPIO_PIN_RESET)
     {
       HAL_Delay(20);
       while(HAL_GPIO_ReadPin(KEY_4_GPIO_Port, KEY_4_Pin) == GPIO_PIN_RESET)
       {
       }
       HAL_Delay(20);
       tarcurrent -= 10;
     }

     if(HAL_GPIO_ReadPin(KEY_1_GPIO_Port, KEY_1_Pin) == GPIO_PIN_RESET)
     {
       HAL_Delay(20);
       while(HAL_GPIO_ReadPin(KEY_1_GPIO_Port, KEY_1_Pin) == GPIO_PIN_RESET)
       {

       }
       HAL_Delay(20);
       tarvoltage += 1000;
     }
     if(HAL_GPIO_ReadPin(KEY_2_GPIO_Port, KEY_2_Pin) == GPIO_PIN_RESET)
     {
       HAL_Delay(20);
       while(HAL_GPIO_ReadPin(KEY_2_GPIO_Port, KEY_2_Pin) == GPIO_PIN_RESET)
       {
       }
       HAL_Delay(20);
       tarvoltage -= 1000;
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

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV2;
  RCC_OscInitStruct.PLL.PLLN = 85;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc){

  //tarvoltage = 20000- k*value2;

  if (hadc->Instance != ADC1) return;
n++;
  value = adc_buffer[0] * 3300.0 /4095.0 * 10.0 * 1.024;
  err = tarvoltage - value;
  CCR += kp*err;
  if(CCR >= 33000)CCR = 33000;
  if(CCR <= 100)CCR = 100;

 // __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, CCR);
  HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].CMP1xR = (uint32_t)CCR;
  value2 = adc_buffer[1] * 3300.0 /4095.0 * 4.667 * 1.145;
  err2 = tarcurrent - value2;
  CCR2 -= kp2 * err2;
  if(CCR2 >= 30000)CCR2 = 30000;
  if(CCR2 <= ARR / 2)CCR2 = ARR / 2;
  //__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, CCR2);
HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_D].CMP1xR = CCR2;
  value3 = adc_buffer[2] * 3300.0 /4095.0 * 10.0 / 0.988;
  value4 = adc_buffer[3] * 3300.0 /4095.0 * 5.0 / 0.7818;

  // __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_D, HRTIM_COMPAREUNIT_1, CCR2);
  //HAL_HRTIM_SoftwareUpdate(&hhrtim1, HRTIM_TIMERINDEX_TIMER_D);
  // value2 = adc_buffer[1] * 3300.0 /4095.0 * 2 *0.9954;
  // err2 = tarcurrent - value2;
  // CCR2 += kp2*err2;
  // if(CCR2 >= 8200)CCR2 = 8200;
  // if(CCR2 <= 100)CCR2 = 100;

  // if(a == 0){
  //
  // }
  // if(a == 1){
  //   __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,CCR2);
  // }

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
#ifdef USE_FULL_ASSERT
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
