/***********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only 
* intended for use with Renesas products. No other uses are authorized. This 
* software is owned by Renesas Electronics Corporation and is protected under 
* all applicable laws, including copyright laws.
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING 
* THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT 
* LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE 
* AND NON-INFRINGEMENT.  ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.
* TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS 
* ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE 
* FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR 
* ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE 
* BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software 
* and to discontinue the availability of this software.  By using this software, 
* you agree to the additional terms and conditions found by accessing the 
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2011, 2012 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/

/***********************************************************************************************************************
* File Name    : rtc_compensation.h
* Version      : 
* Device(s)    : RL78/I1C
* Tool-Chain   : CA78K0R
* Description  : This file implements device driver for Internal Temperature Sensor.
* Creation Date: 
***********************************************************************************************************************/

#ifndef RTC_COMPENSATION_H
#define RTC_COMPENSATION_H

/***********************************************************************************************************************
Include
***********************************************************************************************************************/
#include "typedef.h"
#include "eeprom_storage.h"
/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
/*
 * Default crystal error offset. Must in the range of [-192.26ppm ; 192.26ppm]
 */
#define RTC_COMPENSATION_PPM_OFFSET             (54.5f)
#define CRYSTAL_CURVE_COFF                      (-0.033f) //ppm/C2
/* RTC period setting for doing compensation */
#define RTC_COMPENSATION_PERIOD                 (1)                     /* Period to do compensation, in minute */
#define RTC_CONST_PERIOD                        (500)                   /* Specify RTC const period, it must be 500ms or 1000ms */

/* Temperature sensor characteristics */
#define TMPS_MODE2_T1                           (25.0f)                 /* T1 deg. C */
#define TMPS_MODE2_V1                           (0.654229f)             /* Volt @ T1 degree */
#define TMPS_MODE2_SLOPE                        (-10.7f)                /* Slope at mode 2 */

/* RTC Compensation initial ppm offset */
#define RTC_COMPENSATION_PPM_ADDR                   CONFIG_STORAGE_CALIB_RTC_COMP_OFFSET_ADDR
#define RTC_COMPENSATION_PPM_SIZE                   CONFIG_STORAGE_CALIB_RTC_COMP_OFFSET_SIZE
#define RTC_COMPENSATION_MEM_Read(addr,buf,size)    read_page_eeprom(addr,buf,size)
#define RTC_COMPENSATION_MEM_Write(addr,buf,size)   write_page_eeprom(addr,buf,size)

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/
typedef struct tag_rtc_freq_error_t
{
    int16_t     temperature;
    float32_t   ppm;
} rtc_freq_error_t;

/***********************************************************************************************************************
Global functions
***********************************************************************************************************************/
void RTC_COMP_Init(void);                                               /* Initialize all necessary HW module for RTC temperature compensation */
void RTC_COMP_PollingProcessing(void);                                  /* Polling processing to do temperature compensation */
void RTC_COMP_calib(float value, uint8_t add);
float get_RTC_COMP_value(void);

/* API callback, need to be registered into driver layer */
void RTC_COMP_ConstInterruptCallback(void);                             /* Register into RTC driver layer, const period ISR */
float32_t RTC_COMP_GetTemperature(uint8_t *out_mode, uint16_t *out_counter, float32_t * out_vs);
#endif /* RTC_COMPENSATION_H */
