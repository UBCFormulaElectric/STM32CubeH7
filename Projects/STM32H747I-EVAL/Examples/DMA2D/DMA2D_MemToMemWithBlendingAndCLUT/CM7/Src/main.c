/**
  ******************************************************************************
  * @file    DMA2D/DMA2D_MemToMemWithBlendingAndCLUT/CM7/Src/main.c
  * @author  MCD Application Team
  * @brief   This example provides a description of how to configure
  *          DMA2D peripheral in Memory to Memory with Color Look Up Yable 
  *          and Blending transfer mode.
  *          This is the main program for Cortex-M7.  
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019 STMicroelectronics.
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

#include "L8_Image1.h"
#include "L8_Image0.h"

/** @addtogroup STM32H7xx_HAL_Examples
  * @{
  */

/** @addtogroup DMA2D_MemToMemWithBlendingAndCLUT
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
DMA2D_HandleTypeDef Dma2dHandle;
LTDC_HandleTypeDef  hltdc;

__IO uint32_t CLUT_LoadComplete;
__IO uint32_t Transfer_Complete;

static uint32_t LCD_X_Size = 0, LCD_Y_Size = 0; 

/* Private function prototypes -----------------------------------------------*/
static void DMA2D_Config(uint32_t configNb);
static void TransferError(DMA2D_HandleTypeDef* Dma2dHandle);
static void TransferComplete(DMA2D_HandleTypeDef* Dma2dHandle);
static void SystemClock_Config(void);
static void OnError_Handler(uint32_t condition);

static void CPU_CACHE_Enable(void);
static void MPU_Config(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief   Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  HAL_StatusTypeDef hal_status = HAL_OK;
  uint8_t  lcd_status = BSP_ERROR_NONE;

  /* System Init, System clock, voltage scaling and L1-Cache configuration are done by CPU1 (Cortex-M7) 
     in the meantime Domain D2 is put in STOP mode(Cortex-M4 in deep-sleep)
  */

  /* Configure the MPU attributes as Write Through for SDRAM*/
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

  /* When system initialization is finished, Cortex-M7 could wakeup (when needed) the Cortex-M4  by means of 
     HSEM notification or by any D2 wakeup source (SEV,EXTI..)   */  

  /* Configure LED4, LED2 and LED3 */
  BSP_LED_Init(LED4);
  BSP_LED_Init(LED2);
  BSP_LED_Init(LED3);

 /*##-1- Initialize the LCD ##################################################*/
  lcd_status = BSP_LCD_Init(0, LCD_ORIENTATION_LANDSCAPE);   
  OnError_Handler(lcd_status != BSP_ERROR_NONE);
	
  UTIL_LCD_SetFuncDriver(&LCD_Driver);
  UTIL_LCD_Clear(UTIL_LCD_COLOR_BLACK);

  HAL_Delay(100);

  /* Get the LCD width and height */
  BSP_LCD_GetXSize(0, &LCD_X_Size);
  BSP_LCD_GetYSize(0, &LCD_Y_Size);

  while (1)
  {
    BSP_LED_Off(LED4);    
      
    /*##-2- Configure DMA2D : Configure foreground To display L8_Image1 256 color Indexed  image ##############*/
    DMA2D_Config(0);
    
    /*##-3- Start DMA2D transfer in interrupt mode ###########################*/
    Transfer_Complete = 0;
    hal_status = HAL_DMA2D_Start_IT(&Dma2dHandle,
                                            (uint32_t)&L8_Image1,
                                            (uint32_t)LCD_FRAME_BUFFER,
                                            IMAGE_SIZE_X,
                                            IMAGE_SIZE_Y);
    OnError_Handler(hal_status != HAL_OK);
    
    while(Transfer_Complete == 0)
    {
      /* wait for Transfer complete*/
    }  
    
    HAL_Delay(2000);
    
    BSP_LED_Off(LED4); 
    
    /*##-4- Configure DMA2D : Configure foreground To display L8_Image0 256 color Indexed image ##############*/
    DMA2D_Config(1);
    
    /*##-5- Start DMA2D transfer in interrupt mode ###########################*/
    Transfer_Complete = 0;
    hal_status = HAL_DMA2D_Start_IT(&Dma2dHandle,
                                            (uint32_t)&L8_Image0,
                                            (uint32_t)LCD_FRAME_BUFFER,
                                            IMAGE_SIZE_X,
                                            IMAGE_SIZE_Y);
    OnError_Handler(hal_status != HAL_OK);
    
    while(Transfer_Complete == 0)
    {
      /* wait for Transfer complete*/
    }  
    
    HAL_Delay(2000);  
    
    BSP_LED_Off(LED4);
    
    /*##-6- Configure DMA2D : Configure foreground and background 
        To display blended L8_Image0 and L8_Image1 256 color Indexed images ##*/
    DMA2D_Config(2);
    
    /*##-7- Start DMA2D transfer in interrupt mode ###########################*/
    Transfer_Complete = 0;
    hal_status = HAL_DMA2D_BlendingStart_IT(&Dma2dHandle,
                                            (uint32_t)&L8_Image1,
                                            (uint32_t)&L8_Image0,
                                            (uint32_t)LCD_FRAME_BUFFER,
                                            IMAGE_SIZE_X,
                                            IMAGE_SIZE_Y);
    
    OnError_Handler(hal_status != HAL_OK);
    
    while(Transfer_Complete == 0)
    {
      /* wait for Transfer complete*/
    }
    
    HAL_Delay(2000);  
  }
}


/**
  * @brief DMA2D configuration.
  * @note  This function Configure the DMA2D peripheral :
  *
  *        1) Config 0 :  the Transfer mode as memory to memory with PFC.
  *          1-1) Configure the output color mode as ARGB8888 pixel format.
  *          1-2) Configure the Foreground (layer 1)
  *          - Foreground image is loaded from FLASH memory (L8_Image1)
  *          - CLUT (256 color index table )loaded from FLASH memory (L8_CLUT_1)
  *          - color mode as L8 pixel format
  *          1-3) Load the L8_CLUT_1 CLUT (color index table for image L8_Image1) to DMA2D Forground CLUT
  *  
  *        2) Config 1 :  the Transfer mode as memory to memory with PFC.
  *          2-1) Configure the output color mode as ARGB8888 pixel format.
  *          2-2) Configure the Foreground (layer 1)
  *          - Foreground image is loaded from FLASH memory (L8_Image0)
  *          - CLUT (256 color index table )loaded from FLASH memory (L8_CLUT_0)
  *          - color mode as L8 pixel format
  *          2-3) Load the L8_CLUT_0 CLUT (color index table for image L8_Image0) to DMA2D Forground CLUT
  *  
  *        3) Config 2 :  the Transfer mode as memory to memory with Blending.
  *          3-1) Configure the output color mode as ARGB8888 pixel format.
  *          3-2) Configure the Foreground and Background layers (layer 1 and layer 0)
  *          - Foreground image is loaded from FLASH memory (L8_Image1)
  *          - Background image is loaded from FLASH memory (L8_Image0)
  *          - Forground CLUT (256 color index table )loaded from FLASH memory (L8_CLUT_1)
  *          - Background CLUT (256 color index table )loaded from FLASH memory (L8_CLUT_0)
  *          - color mode as L8 pixel format
  *          3-3) Load the L8_CLUT_1 foreground CLUT (color index table for image L8_Image1) to DMA2D Forground CLUT
  *          3-4) Load the L8_CLUT_0 background CLUT (color index table for image L8_Image0) to DMA2D Background CLUT 
  * @retval
  *  None
  */

static void DMA2D_Config(uint32_t configNb)
{
  HAL_StatusTypeDef hal_status = HAL_OK;
  DMA2D_CLUTCfgTypeDef CLUTCfg;

  if(configNb == 0)
  {
    /* Configure the DMA2D Mode, Color Mode and output offset */
    Dma2dHandle.Init.Mode         = DMA2D_M2M_PFC;         /* DMA2D mode Memory to Memory with Pixel Format Conversion */
    Dma2dHandle.Init.ColorMode    = DMA2D_OUTPUT_ARGB8888; /* output format of DMA2D */
    Dma2dHandle.Init.OutputOffset = (LCD_X_Size - IMAGE_SIZE_X); /* output in pixels */
    Dma2dHandle.Init.AlphaInverted = DMA2D_REGULAR_ALPHA;  /* No Output Alpha Inversion*/  
    Dma2dHandle.Init.RedBlueSwap   = DMA2D_RB_REGULAR;     /* No Output Red & Blue swap */       
    
    /* DMA2D Callbacks Configuration */
    Dma2dHandle.XferCpltCallback  = TransferComplete;
    Dma2dHandle.XferErrorCallback = TransferError;
    
    /* Foreground layer Configuration */
    Dma2dHandle.LayerCfg[1].AlphaMode = DMA2D_REPLACE_ALPHA;
    Dma2dHandle.LayerCfg[1].InputAlpha = 0xFF;                   /* Fully opaque */
    Dma2dHandle.LayerCfg[1].InputColorMode = DMA2D_INPUT_L8;     /* Input Color mode is L8 : indexed 256 color image */
    Dma2dHandle.LayerCfg[1].InputOffset = 0x0;                   /* No offset in input */
    Dma2dHandle.LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA; /* No ForeGround Alpha inversion*/
    Dma2dHandle.LayerCfg[1].RedBlueSwap   = DMA2D_RB_REGULAR;    /* No ForeGround Red/Blue swap*/
      
    Dma2dHandle.Instance = DMA2D;
    
    /* DMA2D Initialization */
    hal_status = HAL_DMA2D_DeInit(&Dma2dHandle);
    hal_status |= HAL_DMA2D_Init(&Dma2dHandle);
    OnError_Handler(hal_status != HAL_OK);
    
    /* Apply DMA2D Foreground configuration */
    HAL_DMA2D_ConfigLayer(&Dma2dHandle, 1);
    
    /* Load DMA2D Foreground CLUT */
    CLUTCfg.CLUTColorMode = DMA2D_CCM_ARGB8888;     
    CLUTCfg.pCLUT = (uint32_t *)L8_CLUT_1;
    CLUTCfg.Size = 255;

    CLUT_LoadComplete = 0;
    HAL_DMA2D_CLUTLoad_IT(&Dma2dHandle, CLUTCfg, 1);
  
    while(CLUT_LoadComplete == 0)
    {
      /* wait for the end of the CLUT loading*/
    } 
  }
  else if(configNb == 1)
  {
    /* Configure the DMA2D Mode, Color Mode and output offset */
    Dma2dHandle.Init.Mode         = DMA2D_M2M_PFC;         /* DMA2D mode Memory to Memory with Pixel Format Conversion */
    Dma2dHandle.Init.ColorMode    = DMA2D_OUTPUT_ARGB8888; /* output format of DMA2D */
    Dma2dHandle.Init.OutputOffset = (LCD_X_Size - IMAGE_SIZE_X); /* output in pixels */
    Dma2dHandle.Init.AlphaInverted = DMA2D_REGULAR_ALPHA;  /* No Output Alpha Inversion*/  
    Dma2dHandle.Init.RedBlueSwap   = DMA2D_RB_REGULAR;     /* No Output Red & Blue swap */      
    
    /* DMA2D Callbacks Configuration */
    Dma2dHandle.XferCpltCallback  = TransferComplete;
    Dma2dHandle.XferErrorCallback = TransferError;
    
    /* Foreground layer Configuration */
    Dma2dHandle.LayerCfg[1].AlphaMode = DMA2D_REPLACE_ALPHA;
    Dma2dHandle.LayerCfg[1].InputAlpha = 0xFF;                   /* Fully opaque */
    Dma2dHandle.LayerCfg[1].InputColorMode = DMA2D_INPUT_L8;     /* Input Color mode is L8 : indexed 256 color image */
    Dma2dHandle.LayerCfg[1].InputOffset = 0x0;                   /* No offset in input */
    Dma2dHandle.LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA; /* No ForeGround Alpha inversion*/
    Dma2dHandle.LayerCfg[1].RedBlueSwap   = DMA2D_RB_REGULAR;    /* No ForeGround Red/Blue swap*/
    
    Dma2dHandle.Instance = DMA2D;
    
    /* DMA2D Initialization */
    hal_status = HAL_DMA2D_DeInit(&Dma2dHandle);
    hal_status |= HAL_DMA2D_Init(&Dma2dHandle);
    OnError_Handler(hal_status != HAL_OK);
    
    /* Apply DMA2D Foreground configuration */
    HAL_DMA2D_ConfigLayer(&Dma2dHandle, 1);
    
    /* Load DMA2D Foreground CLUT */
    CLUTCfg.CLUTColorMode = DMA2D_CCM_ARGB8888;     
    CLUTCfg.pCLUT = (uint32_t *)L8_CLUT_0;
    CLUTCfg.Size = 255;

    CLUT_LoadComplete = 0;
    HAL_DMA2D_CLUTLoad_IT(&Dma2dHandle, CLUTCfg, 1);
  
    while(CLUT_LoadComplete == 0)
    {
      /* wait for the end of the CLUT loading*/
    }    
  }
  else /* configNb == 2 */
  {
    /* Configure the DMA2D Mode, Color Mode and output offset */
    Dma2dHandle.Init.Mode          = DMA2D_M2M_BLEND;       /* DMA2D mode Memory to Memory with Blending */    
    Dma2dHandle.Init.ColorMode     = DMA2D_OUTPUT_ARGB8888; /* output format of DMA2D */
    Dma2dHandle.Init.OutputOffset  = (LCD_X_Size - IMAGE_SIZE_X); /* output in pixels */
    Dma2dHandle.Init.AlphaInverted = DMA2D_REGULAR_ALPHA;   /* No Output Alpha Inversion*/  
    Dma2dHandle.Init.RedBlueSwap   = DMA2D_RB_REGULAR;      /* No Output Red & Blue swap */      
    
    /* DMA2D Callbacks Configuration */
    Dma2dHandle.XferCpltCallback  = TransferComplete;
    Dma2dHandle.XferErrorCallback = TransferError;
    
    /* Foreground layer Configuration */
    Dma2dHandle.LayerCfg[1].AlphaMode = DMA2D_REPLACE_ALPHA;
    Dma2dHandle.LayerCfg[1].InputAlpha = 0x7F;                   /* 127 : semi-transparent */
    Dma2dHandle.LayerCfg[1].InputColorMode = DMA2D_INPUT_L8;     /* Input Color mode is L8 : indexed 256 color image */
    Dma2dHandle.LayerCfg[1].InputOffset = 0x0;                   /* No offset in input */
    Dma2dHandle.LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA; /* No ForeGround Alpha inversion*/
    Dma2dHandle.LayerCfg[1].RedBlueSwap   = DMA2D_RB_REGULAR;    /* No ForeGround Red/Blue swap*/    
    
    /* Background layer Configuration */
    Dma2dHandle.LayerCfg[0].AlphaMode = DMA2D_REPLACE_ALPHA;
    Dma2dHandle.LayerCfg[0].InputAlpha = 0xFF;                   /* Fully opaque */
    Dma2dHandle.LayerCfg[0].InputColorMode = DMA2D_INPUT_L8;     /* Input Color mode is L8 : indexed 256 color image */
    Dma2dHandle.LayerCfg[0].InputOffset = 0x0;                   /* No offset in input */
    Dma2dHandle.LayerCfg[0].AlphaInverted = DMA2D_REGULAR_ALPHA; /* No Background Alpha inversion*/
    Dma2dHandle.LayerCfg[0].RedBlueSwap   = DMA2D_RB_REGULAR;    /* No Background Red a Blue swap*/
    
    Dma2dHandle.Instance = DMA2D;
    
    /* DMA2D Initialization */
    hal_status = HAL_DMA2D_DeInit(&Dma2dHandle);
    hal_status |= HAL_DMA2D_Init(&Dma2dHandle);
    OnError_Handler(hal_status != HAL_OK);
    
    /* Apply DMA2D Foreground configuration */
    HAL_DMA2D_ConfigLayer(&Dma2dHandle, 1);
    
    /* Apply DMA2D Background configuration */
    HAL_DMA2D_ConfigLayer(&Dma2dHandle, 0);
    
    /* Load DMA2D Foreground CLUT */
    CLUTCfg.CLUTColorMode = DMA2D_CCM_ARGB8888;     
    CLUTCfg.pCLUT = (uint32_t *)L8_CLUT_1;
    CLUTCfg.Size = 255;

    CLUT_LoadComplete = 0;
    HAL_DMA2D_CLUTLoad_IT(&Dma2dHandle, CLUTCfg, 1);
  
    while(CLUT_LoadComplete == 0)
    {
      /* wait for the end of the CLUT loading*/
    }

    /* Load DMA2D Background CLUT */
    CLUTCfg.CLUTColorMode = DMA2D_CCM_ARGB8888;     
    CLUTCfg.pCLUT = (uint32_t *)L8_CLUT_0;
    CLUTCfg.Size = 255;

    CLUT_LoadComplete = 0;
    HAL_DMA2D_CLUTLoad_IT(&Dma2dHandle, CLUTCfg, 0);
  
    while(CLUT_LoadComplete == 0)
    {
      /* wait for the end of the CLUT loading*/
    }    
  }  
}

/**
  * @brief  On Error Handler on condition TRUE.
  * @param  condition : Can be TRUE or FALSE
  * @retval None
  */
static void OnError_Handler(uint32_t condition)
{
  if(condition)
  {
    BSP_LED_On(LED3);
    while(1) { ; } /* Blocking on error */
  }
}

/**
  * @brief  DMA2D Transfer completed callback
  * @param  hdma2d: DMA2D handle.
  * @note   This example shows a simple way to report end of DMA2D transfer, and
  *         you can add your own implementation.
  * @retval None
  */
static void TransferComplete(DMA2D_HandleTypeDef *hdma2d)
{
  /* Turn LED4 On */
  BSP_LED_On(LED4);
  
  Transfer_Complete = 1;
}

/**
  * @brief  DMA2D error callbacks
  * @param  hdma2d: DMA2D handle
  * @note   This example shows a simple way to report DMA2D transfer error, and you can
  *         add your own implementation.
  * @retval None
  */
static void TransferError(DMA2D_HandleTypeDef *hdma2d)
{
  /* Turn LED2 On */
  BSP_LED_On(LED2);
}

void HAL_DMA2D_CLUTLoadingCpltCallback(DMA2D_HandleTypeDef *hdma2d)
{
  CLUT_LoadComplete = 1;
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 400000000 (CM7 CPU Clock)
  *            HCLK(Hz)                       = 200000000 (CM4 CPU, AXI and AHBs Clock)
  *            AHB Prescaler                  = 2
  *            D1 APB3 Prescaler              = 2 (APB3 Clock  100MHz)
  *            D2 APB1 Prescaler              = 2 (APB1 Clock  100MHz)
  *            D2 APB2 Prescaler              = 2 (APB2 Clock  100MHz)
  *            D3 APB4 Prescaler              = 2 (APB4 Clock  100MHz)
  *            HSE Frequency(Hz)              = 25000000
  *            PLL_M                          = 5
  *            PLL_N                          = 160
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
  HAL_PWREx_ConfigSupply(PWR_DIRECT_SMPS_SUPPLY);

  /* The voltage scaling allows optimizing the power consumption when the device is 
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}
 
  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
  RCC_OscInitStruct.CSIState = RCC_CSI_OFF;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;

  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 160;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
 
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
  if(ret != HAL_OK)
  {
    while(1) { ; }
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
    while(1) { ; }
  }

 /*
  Note : The activation of the I/O Compensation Cell is recommended with communication  interfaces
          (GPIO, SPI, FMC, QSPI ...)  when  operating at  high frequencies(please refer to product datasheet)       
          The I/O Compensation Cell activation  procedure requires :
        - The activation of the CSI clock
        - The activation of the SYSCFG clock
        - Enabling the I/O Compensation Cell : setting bit[0] of register SYSCFG_CCCSR
 */
 
  /*activate CSI clock mondatory for I/O Compensation Cell*/  
  __HAL_RCC_CSI_ENABLE() ;
    
  /* Enable SYSCFG clock mondatory for I/O Compensation Cell */
  __HAL_RCC_SYSCFG_CLK_ENABLE() ;
  
  /* Enables the I/O Compensation Cell */    
  HAL_EnableCompensationCell();
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
  * @brief  Configure the MPU attributes as Write Through for External SDRAM.
  * @note   The Base Address is SDRAM_DEVICE_ADDR .
  *         The Configured Region Size is 32MB because same as SDRAM size.
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

  /* Configure the MPU attributes as WT for SDRAM */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = SDRAM_DEVICE_ADDR;
  MPU_InitStruct.Size = MPU_REGION_SIZE_32MB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER1;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

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
void assert_failed(uint8_t* file, uint32_t line)
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


