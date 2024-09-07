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

#ifndef _RELAY_H
#define _RELAY_H

/***********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
***********************************************************************************************************************/
#define RELAY_CONTROL_ENABLE
/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
#ifndef RELAY_ON
#define RELAY_ON    1
#endif

#ifndef RELAY_OFF
#define RELAY_OFF   0
#endif

#define RELAY_R_SET          (BIT_SELECT(P7, 6))  /* R */
#define RELAY_R_PRESET       (BIT_SELECT(P3, 2))  /* R */
#define RELAY_B_SET          (BIT_SELECT(P3, 3))  /* B */
#define RELAY_B_PRESET       (BIT_SELECT(P3, 0))  /* B */
#define RELAY_Y_SET          {;}//(BIT_SELECT(P5, 7))  /* Y */
#define RELAY_Y_PRESET       {;}//(BIT_SELECT(P5, 6))  /* Y */

#ifndef RELAY_CONNECTED
#define RELAY_CONNECTED 1
#endif

#ifndef RELAY_DISCONNECTED
#define RELAY_DISCONNECTED 0
#endif

/***********************************************************************************************************************
Variable Externs
***********************************************************************************************************************/

/***********************************************************************************************************************
Functions Prototypes
***********************************************************************************************************************/
extern uint8_t RELAY_SetStatusValue;
void RELAY_PollingProcessing(uint8_t init);
void RELAY_SwitchOn(void);
void RELAY_SwitchOff(void);
void RELAY_Initialize(void);
//uint8_t RELAY_GetStatus(void);
//void RELAY_SetStatus(uint8_t status);
//uint8_t RELAY_GetPreviousStatus(void);
void RELAY_SetPreviousStatus(uint8_t status);
void Switch_Delay(void);

void RELAY_SwitchOff_R(void);
void RELAY_SwitchOff_Y(void);
void RELAY_SwitchOff_B(void);
void RELAY_SwitchOn_R(void);
void RELAY_SwitchOn_Y(void);
void RELAY_SwitchOn_B(void);
void connect_relay_func(void);
//void relay_init(void);
//void latch_check(void);
#endif /* _RELAY_H */




