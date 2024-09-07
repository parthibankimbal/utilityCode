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
* File Name    : startup.h
* Version      : 1.00
* Device(s)    : RL78/I1C
* Tool-Chain   : 
* H/W Platform : RL78/I1C Energy Meter Platform
* Description  : Start-up source File
***********************************************************************************************************************/

#ifndef _START_UP
#define _START_UP

/******************************************************************************
Includes   <System Includes> , "Project Includes"
******************************************************************************/

/******************************************************************************
Typedef definitions
******************************************************************************/
typedef enum
{
  e_LCD_contrast_lowest_level,
  e_LCD_contrast_midium_level,
  e_LCD_contrast_high_level,
  e_LCD_contrast_maximum_level,
}e_LCD_CONTRAST_LEVEL_TYPE;

typedef struct st_em_startup_diag {
    uint8_t em_init_status;             /* EM_Init status */
    uint8_t config_load_status;         /* Config storage reload from dataflash status (other than 0 means error) */
    uint8_t energy_load_status;         /* Energy storage reload from EEPROM status (other than 0 means error) */
    uint8_t em_start_status;            /* EM_Start status */
} st_em_startup_diag_t;
/******************************************************************************
Macro definitions
******************************************************************************/
/* Status */
#define STARTUP_OK                  0       /* OK */
#define STARTUP_ERROR               1       /* Error */

/***********************************************************************************************************************
Variable Externs
******************************************************************************/
extern uint8_t g_reset_flag;
extern uint8_t g_softreset_flag;
extern st_em_startup_diag_t g_em_startup_diag;
/******************************************************************************
Functions Prototypes
******************************************************************************/
uint8_t startup(void);
void start_all_LVDs(void);

#endif /* _START_UP */

