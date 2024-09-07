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
* File Name    : relay.h
* Version      : 1.00
* Device(s)    : RL78/I1C
* Tool-Chain   : CA78K0R
* H/W Platform : RL78/I1C Energy Meter Platform
* Description  : RELAY processing source File
***********************************************************************************************************************/

#ifndef _1_WIRE_H
#define _1_WIRE_H

/***********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
***********************************************************************************************************************/
#include "r_cg_macrodriver.h"
#include "r_cg_cgc.h"
#include <string.h>
#include <stdlib.h>
#include "typedef.h"            /* GCSE Standard definitions */
#include "r_cg_userdefine.h"
#include "r_cg_it8bit.h"
#include "key.h"            /* KEY Interface Header File */
#include "r_cg_dsadc.h"
#include "math.h"
#include "main.h"
#include "em_operation.h"
#include "eeprom.h"
#include "eeprom_storage.h"
#include "rtc_user.h"
#include "startup.h"
#include "wrp_em_pulse.h"
#include "r_cg_lcd.h"
#include "em_display.h"

/* Filter middleware */
/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
#define SINGLE_WIRE_SKIP_TIME
/***********************************************************************************************************************
Variable Externs
***********************************************************************************************************************/

/***********************************************************************************************************************
Functions Prototypes
***********************************************************************************************************************/
typedef struct 
{
  uint64_t accumulated;
  uint16_t count;
}st_sq_accumulate;

#define SINGLE_WIRE_FAST_SCAN_MAX_TIME			28800//8hrs X 60 X 60

extern uint8_t g_u8SingleWireTimeUpdate;
extern uint32_t g_u32SleepTime;
extern int16_t i16PhBufOffset[];
extern int16_t i16NeuBufOffset[];
extern uint8_t g_uSingleWirePulsesEnable;

void SingleWireCheck(void);
void R_DSADC_Create2(void);
void R_DSADC_Start2(void);
void R_DSADC_Stop2(void);
void R_DSADC_Set_OperationOn2(void);
void R_DSADC_Set_OperationOff2(void);
void accumulate_square(st_sq_accumulate* accumulate, int64_t sample);
void set_SingleWire_Offset(void);
uint8_t verify_SingleWire_Offset(void);
void pushButton_Callback(void);
void singleWireEnablePulses(void);
void singleWireDisablePulses(void);
void singleWireEnableDisplay(void);
void singleWireDisableDisplay(void);
void keyPressEnableDisplayAndPulses(void);
#endif /* _1_WIRE_H */




