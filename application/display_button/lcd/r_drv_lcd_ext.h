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
* File Name    : r_drv_lcd_ext.h
* Version      : 1.00
* Device(s)    : RL78/I1C
* Tool-Chain   : CA78K0R
* H/W Platform : RL78/I1C Energy Meter Platform
* Description  : MCU LCD module driver APIs
***********************************************************************************************************************/

#ifndef _R_DRV_LCD_EXT_H
#define _R_DRV_LCD_EXT_H

/***********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
***********************************************************************************************************************/
#include "typedef.h"
#include "r_lcd_config.h"

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/

/*
 * LCD RAM Definitions
 * The data area which need to be changed when changing MCU
 */
#define     LCD_RAM_START_ADDRESS   0xf0400 /* LCD Memory Start Address */
#define     LCD_RAM_END_ADDRESS     0xf0426 /* LCD Memory End Address */
#define     LCD_RAM_SIZE            (LCD_RAM_END_ADDRESS - LCD_RAM_START_ADDRESS+1)

 /* Return information on each function */
#define LCD_INPUT_OK                0x00
#define LCD_WRONG_INPUT_ARGUMENT    0x80

/***********************************************************************************************************************
Variable Externs
***********************************************************************************************************************/

/***********************************************************************************************************************
Functions Prototypes
***********************************************************************************************************************/
/* Controlling LCD RAM */
uint16_t LCD_GetRamSize(void);          /* LCD Get Ram Size */
void LCD_RamClearAll(void);             /* LCD Ram Clear All */
void LCD_RamSetAll(void);               /* LCD Ram Set All */

uint8_t LCD_GetNumInfo(uint8_t disp_value, uint8_t position, LCD_NUM_INFO* num_info);
uint8_t LCD_GetSignInfo(uint8_t sign, LCD_SIGN_INFO* sign_info);
void LCD_WriteRAMDigitInfo(uint32_t ram_addr, uint8_t disp_value);
void LCD_WriteRAMDigitInfo_8_Comm(uint32_t ram_addr, uint8_t disp_value);
void LCD_WriteRAMDigitInfo_8_Comm_Alfa(uint32_t ram_addr, uint8_t disp_value, uint8_t disp_value_2);
void LCD_ClearRAMDigitInfo(uint32_t ram_addr, uint8_t disp_value);
void LCD_ClearRAMDigitInfo_8_Comm(uint32_t ram_addr, uint8_t disp_value);
void LCD_ClearRAMDigitInfo_8_Comm_Alfa(uint32_t ram_addr, uint8_t disp_value, uint8_t disp_value_2);
void LCD_WriteRAMSignInfo(uint32_t ram_addr, uint8_t disp_value);
void LCD_ClearRAMSignInfo(uint32_t ram_addr, uint8_t disp_value);
void LCD_WriteRAMDigitInfo_8_Comm_TZ(uint32_t ram_addr, uint8_t disp_value);


#endif /* _R_DRV_LCD_EXT_H */
