/**
  ******************************************************************************
  * @file    LPTIM/LPTIM_PWM_LSE/Src/main.c
  * @author  MCD Application Team
  * @brief   This example describes how to configure and use LPTIM to generate
  *          a PWM in stop mode using the LSE as a counter clock, through the
  *          STM32H7xx HAL API.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/** @addtogroup STM32H7xx_HAL_Examples
  * @{
  */

/** @addtogroup LPTIM_PWM_LSE
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Set the Maximum value of the counter (Auto-Reload) that defines the Period */
#define PeriodValue             (uint32_t) (100 - 1)

/* Set the Compare value that defines the duty cycle */
#define PulseValue              (uint32_t) (50 - 1)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* LPTIM handle declaration */
LPTIM_HandleTypeDef             LptimHandle;

/* Clocks structure declaration */
RCC_PeriphCLKInitTypeDef        RCC_PeriphCLKInitStruct;

/* Private function prototypes -----------------------------------------------*/
static void MPU_Config(void);
static void SystemClock_Config(void);
static HAL_StatusTypeDef LSE_ClockEnable(void);
static void Error_Handler(void);
static void CPU_CACHE_Enable(void);
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  /* Configure the MPU attributes */
  MPU_Config();

  /* Enable the CPU Cache */
  CPU_CACHE_Enable();
  
  /* STM32H7xx HAL library initialization:
       - Systick timer is configured by default as source of time base, but user 
         can eventually implement his proper time base source (a general purpose 
         timer for example or other time source), keeping in mind that Time base 
         duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and 
         handled in milliseconds basis.
       - Set NVIC Group Priority to 4
       - Low Level Initialization
     */
  HAL_Init();

  /* Configure the system clock to 400 MHz */
  SystemClock_Config();

  /* Configure LED2 */
  BSP_LED_Init(LED2);
  
  /* User push-button (EXTI_Line15_10) will be used to wakeup the system from Stop mode */
  BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);

  /* Enable the LSE Clock */
  if (LSE_ClockEnable() != HAL_OK)
  {
    Error_Handler();
  }
  
  /* ### - 1 - Re-target the LSE to Clock the LPTIM Counter ################# */
  /* Select the LSE clock as LPTIM peripheral clock */
  RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LPTIM1;
  RCC_PeriphCLKInitStruct.Lptim1ClockSelection = RCC_LPTIM1CLKSOURCE_LSE;  
  HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct);
  
  
  /* ### - 2 - Initialize the LPTIM peripheral ############################## */
  /*
   *  Instance        = LPTIM1
   *  Clock Source    = APB or LowPowerOSCillator (in this example LSE is
   *                    already selected from the RCC stage)
   *  Counter source  = External event.
   *  Clock prescaler = 1 (No division)
   *  Counter Trigger = Software trigger
   *  Output Polarity = High
   *  Update mode     = Immediate (Registers are immediately updated after any
   *                    write access) 
   */

  LptimHandle.Instance = LPTIM1;
  
  LptimHandle.Init.Clock.Source    = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
  LptimHandle.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV1;  
  LptimHandle.Init.CounterSource   = LPTIM_COUNTERSOURCE_INTERNAL;
  LptimHandle.Init.Trigger.Source  = LPTIM_TRIGSOURCE_SOFTWARE; 
  LptimHandle.Init.OutputPolarity  = LPTIM_OUTPUTPOLARITY_HIGH;
  LptimHandle.Init.UpdateMode      = LPTIM_UPDATE_IMMEDIATE;
  LptimHandle.Init.Input1Source    = LPTIM_INPUT1SOURCE_GPIO;
  LptimHandle.Init.Input2Source    = LPTIM_INPUT2SOURCE_GPIO;
  
  /* Initialize LPTIM peripheral according to the passed parameters */
  if (HAL_LPTIM_Init(&LptimHandle) != HAL_OK)
  {
    Error_Handler();
  }

  /* ### - 3 - Start counting in interrupt mode ############################# */
  /*
   *  Period = 99
   *  Pulse  = 49
   *  According to this configuration, the duty cycle will be equal to 50%
   */
  if (HAL_LPTIM_PWM_Start(&LptimHandle, PeriodValue, PulseValue) != HAL_OK)
  {
    Error_Handler();
  }
  
  /* ### - 4 - Enter in Stop mode ########################################### */
  HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI); 
  
  /* ### - 5 - Stop counting when leaving Stop mode ########################## */
  if (HAL_LPTIM_PWM_Stop(&LptimHandle) != HAL_OK)
  {
    Error_Handler();
  }

  /* Infinite Loop */
  while (1)
  {        
  }
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE BYPASS)
  *            SYSCLK(Hz)                     = 400000000 (CPU Clock)
  *            HCLK(Hz)                       = 200000000 (AXI and AHBs Clock)
  *            AHB Prescaler                  = 2
  *            D1 APB3 Prescaler              = 2 (APB3 Clock  100MHz)
  *            D2 APB1 Prescaler              = 2 (APB1 Clock  100MHz)
  *            D2 APB2 Prescaler              = 2 (APB2 Clock  100MHz)
  *            D3 APB4 Prescaler              = 2 (APB4 Clock  100MHz)
  *            HSE Frequency(Hz)              = 8000000
  *            PLL_M                          = 4
  *            PLL_N                          = 400
  *            PLL_P                          = 2
  *            PLL_Q                          = 4
  *            PLL_R                          = 2
  *            VDD(V)                         = 3.3
  *            Flash Latency(WS)              = 4
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  HAL_StatusTypeDef ret = HAL_OK;
  
  /*!< Supply configuration update enable */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /* The voltage scaling allows optimizing the power consumption when the device is
     clocked below the maximum system frequency, to update the voltage scaling value
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}
  
  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
  RCC_OscInitStruct.CSIState = RCC_CSI_OFF;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;

  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 400;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLQ = 4;

  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_1;
  ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
  if(ret != HAL_OK)
  {
    Error_Handler();
  }
  
/* Select PLL as system clock source and configure  bus clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_D1PCLK1 | RCC_CLOCKTYPE_PCLK1 | \
                                 RCC_CLOCKTYPE_PCLK2  | RCC_CLOCKTYPE_D3PCLK1);

  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;  
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2; 
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2; 
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2; 
  ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4);
  if(ret != HAL_OK)
  {
    Error_Handler();
  }

}
/**
  * @brief  Enable External Low Speed Clock (LSE)
  * @param  None
  * @retval Status
  */
static HAL_StatusTypeDef LSE_ClockEnable(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct;
  
  /* Enable LSE clock */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  return (HAL_RCC_OscConfig(&RCC_OscInitStruct));
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  /* Turn LED2 on */
  BSP_LED_On(LED2);
  while (1)
  {
  }
}

/**
  * @brief  CPU L1-Cache enable.
  * @param  None
  * @retval None
  */
static void CPU_CACHE_Enable(void)
{
  /* Enable I-Cache */
  SCB_EnableICache();

  /* Enable D-Cache */
  SCB_EnableDCache();
}


/**
  * @brief  Configure the MPU attributes
  * @param  None
  * @retval None
  */
static void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct;

  /* Disable the MPU */
  HAL_MPU_Disable();

  /* Configure the MPU as Strongly ordered for not defined regions */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = 0x00;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Enable the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
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
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}

#endif

/**
  * @}
  */

/**
  * @}
  */

