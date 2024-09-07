/***********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products.
* No other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
* applicable laws, including copyright laws.
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIESREGARDING THIS SOFTWARE, WHETHER EXPRESS, IMPLIED
* OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NON-INFRINGEMENT.  ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY
* LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE FOR ANY DIRECT,
* INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR
* ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability
* of this software. By using this software, you agree to the additional terms and conditions found by accessing the
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2013, 2015 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/

/***********************************************************************************************************************
* File Name    : em_display.h
* Version      : 1.00
* Device(s)    : RL78/I1C
* Tool-Chain   : CA78K0R
* H/W Platform : RL78/I1C Energy Meter Platform
* Description  : EM Display Application Layer APIs
***********************************************************************************************************************/

#ifndef _EM_DISPLAY_H
#define _EM_DISPLAY_H

/***********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
***********************************************************************************************************************/

/* Wrapper layer */
#include "compiler.h"
#include "r_drv_lcd_ext.h"
#include "wrp_user_ext.h"
#include "rtc_user.h"
#include "factory_settings.h"
#include "string.h"
/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/
typedef enum
{
  e_METERING_MODE,
  e_PAYMENT_MODE,
  e_READ_EEPROM,
}e_Display_Config;

typedef struct tagTimeDataInfo
{
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
} TIME_DATA_INFO;

typedef struct tagLCD_icon_status
{
  uint32_t logo:1;
  uint32_t magnet:1;
  uint32_t warning:1;
  uint32_t cover_open:1;
  uint32_t reverse:1;
  uint32_t prepaid:1;
  uint32_t battery:1;
  uint32_t clock:1;
  uint32_t r_phase:1;
  uint32_t y_phase:1;
  uint32_t b_phase:1;
  uint32_t latch_stat:2;
  uint32_t mains:1;
  uint32_t DG:1;
  uint32_t DG_Cover:1;
  uint32_t bypass:1;
  uint32_t rupee:1;
  uint32_t rx:1;
  uint32_t tx:1;
  uint32_t signal:3;
  uint32_t earth_loading:1;
  uint32_t single_wire:1;
}LCD_icon_status;
extern LCD_icon_status e_LCD_icon_status;
/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
#ifdef __DEBUG
#define LCD_FUNC                    FAR_FUNC
#else
#define LCD_FUNC                    FAR_FUNC
#endif

#define LCD_FIRST_DISPLAY_POS       1       /* default is paramerter 0 */
#define LCD_DELAY_TIME              20      /* Create delay time 10s */
#ifdef UTILITY_JNK
  #define LCD_DELAY_TIME_MODE_CHANGE      20      /* Create delay time 10s */
#elif (UTILITY_INTELLI || UTILITY_APRAAVA || UTILITY_PURBANCHAL)
  #define LCD_DELAY_TIME_MODE_CHANGE      60      /* Create delay time 30s */
#endif
#define METER_VERSION_TIME          4       /*2 seconds*/
#define DISPLAY_MODE_ENTER_TIME     10      /*5 seconds*/
/***********************************************************************************************************************
Variable Externs
***********************************************************************************************************************/

/***********************************************************************************************************************
Functions Prototypes
***********************************************************************************************************************/

/* Prototype APIs which are used to display EM information */
LCD_FUNC void EM_DisplaySequence(void);
LCD_FUNC void EM_DisplaySequenceReset(void);
LCD_FUNC void EM_RTC_DisplayInterruptCallback(void);

/* APIs which are used to control LCD */
LCD_FUNC void LCD_SwitchAutoRoll(void);
LCD_FUNC void LCD_ChangeNext(void);
LCD_FUNC void EM_LCD_DisplayPOR(void);

LCD_FUNC void EM_LCD_DisplayNVMFail(void);

/* APIs which are used to display some extra information on LCD */
LCD_FUNC uint8_t LCD_DisplayTime(TIME_DATA_INFO time_info, uint8_t is_used_BCD);

/* APIs which are used to display number on LCD */
LCD_FUNC uint8_t LCD_DisplayIntWithPos(long lNum, int8_t position);
LCD_FUNC uint8_t LCD_DisplayFloat(float32_t fnum);
LCD_FUNC uint8_t LCD_DisplayFloat3Digit(float32_t fnum);
LCD_FUNC void LCD_Print_string(char *data, uint8_t len);
void EM_LCD_Toggle_Logo(uint8_t toggle_val);
void EM_LCD_Display_Signal_bar(void);
#endif /* _EM_DISPLAY_H */
