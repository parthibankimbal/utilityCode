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
* File Name    : config_storage.h
* Version      : 1.00
* Device(s)    : RL78/I1C
* Tool-Chain   : CCRL
* H/W Platform : RL78/I1C Energy Meter Platform
* Description  : Storage Source File
***********************************************************************************************************************/

#ifndef _CONFIG_STORAGE_H
#define _CONFIG_STORAGE_H

/***********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
***********************************************************************************************************************/
/* Driver */
#include "r_cg_macrodriver.h"   /* CG Macro Driver */
#include "r_cg_dsadc.h"         /* CG ADC Driver */
#include "r_cg_rtc.h"           /* CG RTC Driver */
#include "r_cg_wdt.h"           /* CG WDT Driver */
#include "r_cg_crc.h"           /* CG CRC Driver */

#include "config_format.h"     /* Storage Format Header */
/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/
typedef struct tag_em_setting
{
    float32_t degree_list_i1[EM_GAIN_PHASE_NUM_LEVEL_MAX];
    float32_t degree_list_i2[EM_GAIN_PHASE_NUM_LEVEL_MAX];
    float32_t gain_list_i1[EM_GAIN_PHASE_NUM_LEVEL_MAX];
    float32_t gain_list_i2[EM_GAIN_PHASE_NUM_LEVEL_MAX];
    dsad_reg_setting_t regs;
} st_em_setting_t;

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
/* Error Status */
#define CONFIG_OK                  0       /* OK */
#define CONFIG_ERROR               1       /* Error */
#define CONFIG_ERROR_DATA_CORRUPT  2       /* Data Storage in EEPROM is corrupted */

/* Warning Status */
#define CONFIG_NOT_FORMATTED       3       /* Storage is NOT formatted */

/* Bit field selection for Backup/Restore */
#define CONFIG_ITEM_NONE           0x00    /* None selection */
#define CONFIG_ITEM_CALIB          0x02    /* Select EM Calibration */

#define CONFIG_ITEM_ALL            (        CONFIG_ITEM_CALIB      |\
                                            (0)                     \
                                   )   /* Select all */

#ifdef __DEBUG
#define CFS_FUNC                    FAR_FUNC
#else
#define CFS_FUNC                    FAR_FUNC
#endif


/***********************************************************************************************************************
Variable Externs
***********************************************************************************************************************/
extern uint8_t g_DATAFLASH_FormatTrigger;
/***********************************************************************************************************************
Functions Prototypes
***********************************************************************************************************************/
/* STORAGE Module Controller */
CFS_FUNC uint8_t CONFIG_Init(uint8_t is_checking);                               /* Storage Initialization */
CFS_FUNC uint8_t CONFIG_Format(void);                                            /* Storage Format Device */
/* APIs used to check RAM area */
CFS_FUNC void CONFIG_PollingProcessing(void);                                    /* Storage Polling Processing */

/* STORAGE Module Write/Add/Backup/Restore */
/* Only call when the Storage device is formatted,
 * if not, error will occurred */
CFS_FUNC uint8_t CONFIG_Backup(uint8_t selection);                               /* Storage Backup */
CFS_FUNC uint8_t CONFIG_Restore(uint8_t selection);                              /* Storage Restore */
CFS_FUNC uint8_t CONFIG_LoadEMCalib(EM_CALIBRATION * p_calib, dsad_reg_setting_t * p_regs);               /* Load meter calibration configuration from storage */
/* Control the CRC checking:
 * Note: only avaliable in Single phase metrology
 * Reason: calibration in Single phase meter only set calib
 *       : after both calibration information finished.
 *       : In the middle, if CRC run, it will reload with
 *       : default value again
*/
CFS_FUNC void CONFIG_DisableCRCChecking(void);                                  /* Enable CRC Checking */
CFS_FUNC void CONFIG_EnableCRCChecking(void);                                   /* Enable CRC Checking */
/* Callback, need register to driver*/
CFS_FUNC void CONFIG_RtcCallback(void);                                          /* RTC Constand Interrupt Callback */

#endif /* _CONFIG_STORAGE_H */

