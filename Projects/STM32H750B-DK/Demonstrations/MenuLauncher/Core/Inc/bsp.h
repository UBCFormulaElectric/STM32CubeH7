/**
  ******************************************************************************
  * @file    bsp.h
  * @author  MCD Application Team
  * @brief   Header for bsp.c file
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BSP_H
#define __BSP_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define BKP_REG_PWR_CFG           (RTC->BKP28R)
#define BKP_REG_SW_CFG            (RTC->BKP27R)
#define BKP_REG_SUBDEMO_ADDRESS   (RTC->BKP26R)
#define BKP_REG_CALIB_DR0         (RTC->BKP25R)
#define BKP_REG_CALIB_DR1         (RTC->BKP24R)

/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
uint8_t BSP_Config(const uint8_t minimal);
uint8_t BSP_TouchUpdate(void);
void BSP_JumpToSubDemo(uint32_t SubDemoAddress);

#ifdef __cplusplus
}
#endif

#endif /*__BSP_H */

