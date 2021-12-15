/**
  ******************************************************************************
  * @file    DMA2D/DMA2D_MemToMemWithBlendingAndCLUT/Inc/main.h 
  * @author  MCD Application Team
  * @brief   Header for main.c module
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
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stm32h7b3i_eval.h"
#include "stm32h7b3i_eval_lcd.h"
#include "stm32h7b3i_eval_sdram.h"
#include "stm32h7b3i_eval_io.h"


#define LCD_FRAME_BUFFER  0xD0000000

#define COLOR_LUT_SIZE_Y  256 /* size of the color Look Up Table*/

#define IMAGE_SIZE_X      320
#define IMAGE_SIZE_Y      240

#define ARGB8888_BYTE_PER_PIXEL  4
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#endif /* __MAIN_H */

