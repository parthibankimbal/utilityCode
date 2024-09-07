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
* File Name    : r_meter_cmd.h
* Version      : 1.00
* Device(s)    : RL78/I1C
* Tool-Chain   : CCRL
* H/W Platform : RL78/I1C Energy Meter Platform
* Description  : CMD Prompt MW Layer APIs
***********************************************************************************************************************/

#ifndef __R_METER_CMD_H
#define __R_METER_CMD_H

/***********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
***********************************************************************************************************************/
#include "typedef.h"        /* GSCE Standard Typedef */
#include "em_type.h"
#include "platform.h"
#include "communication.h"
#include "em_energies.h"

#include "HardwareTest.h"
#if ((defined METER_ENABLE_PROPRIETARY_METER_COMMAND) && (METER_ENABLE_PROPRIETARY_METER_COMMAND == TRUE))

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

typedef void(*meter_calibration_init_type_callback)(void);
extern meter_calibration_init_type_callback meter_calibration_init_callback;
/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
#define SUPPORT_COMMAND_CALIBRATION                 (TRUE)
#define SUPPORT_COMMAND_MEMORY_FORMAT               (TRUE)
//#define SUPPORT_COMMAND_SIGNAL_LOGGING              (TRUE)
#define SUPPORT_COMMAND_ONCHIP_MEMORY_ACCESS        (TRUE)
//#define SUPPORT_COMMAND_PLATFORM_ACCESS             (TRUE)

/* Define the key used for authentication */
#define R_METER_CMD_AUTHENTICATION_KEY              "k_meter_umd_auth"//"r_meter_access_r"//"r_meter_cmd_auth"
#define R_METER_CMD_AUTHENTICATION_PASS             "$kimbal.com"
#define R_METER_CMD_UNLOCK_ACCESS_KEY               "unlock_the_meter"

/* User configurable */
/* Define the RL78 UART channel: 0, 1, 2 or 3 only
* Note:
*   - UART driver for that channel must already exist on driver layer
*   - No parenthese
*/
#define METER_CMD_SIGLOG_UART_CHANNEL                   2
/* Define the SPS value to achieve 6MHz clock:
* 0: CPU clock 6MHz
* 1: CPU clock 12MHz
* 2: CPU clock 24MHz
*/
#define METER_CMD_SIGLOG_SPS_DIVIDER_SPS_CLK_6M         0

/*
* Define the maximum number of signals can acquire from meter
*/
#define METER_CMD_SIGLOG_SUPPORTED_NUM_OF_SIGNALS       9

/*
* Define the variable to put into the signal buffer
* Note:
*   + The p_samples is avaiable in the ADC call back
*   + The macro below need to be used only in that function
*   + No parenthese
*/
#define METER_CMD_SIGLOG_VALUE_SIGNAL1                  p_samples->v
#define METER_CMD_SIGLOG_VALUE_SIGNAL2                  p_samples->i1
#define METER_CMD_SIGLOG_VALUE_SIGNAL3                  p_samples->i2
#define METER_CMD_SIGLOG_VALUE_SIGNAL4                  p_samples->v_fund
#define METER_CMD_SIGLOG_VALUE_SIGNAL5                  p_samples->v
#define METER_CMD_SIGLOG_VALUE_SIGNAL6                  p_samples->v
#define METER_CMD_SIGLOG_VALUE_SIGNAL7                  p_samples->v
#define METER_CMD_SIGLOG_VALUE_SIGNAL8                  p_samples->v
#define METER_CMD_SIGLOG_VALUE_SIGNAL9                  p_samples->v

/***********************************************************************************************************************
Variable Externs
***********************************************************************************************************************/
/***********************************************************************************************************************
Functions Prototypes
***********************************************************************************************************************/
/* Core function, put to meter initialization and polling thread */
void R_METER_CMD_Init(void);
uint8_t R_METER_CMD_PollingProcessing(uint8_t u8Port);

/* Assign to UART Send and Receive End interrupt callback function */
void R_METER_CMD_UART_SendEndCallback(void);
void R_METER_CMD_UART_ReceiveEndCallback(uint8_t receive_port, uint8_t received_byte);

/* Export function for other command to other meter application */
uint8_t R_METER_CMD_ADC_IsRequested(void);
void R_METER_CMD_ADC_InterruptCallBack(NEAR_PTR EM_SAMPLES * p_samples);

#else

#define R_METER_CMD_Init()                          {;}
#define R_METER_CMD_PollingProcessing()             {;}
#define R_METER_CMD_UART_SendEndCallback()          {;}
#define R_METER_CMD_UART_ReceiveEndCallback         {;}
#define R_METER_CMD_ADC_IsRequested()               (FALSE)
#define R_METER_CMD_ADC_InterruptCallBack           {;}

#endif

#endif /* __R_METER_CMD_H */
