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
* File Name    : credit.c
* Version      : 1.00
* Device(s)    : RL78/I1C
* Tool-Chain   : CA78K0R
* H/W Platform : RL78/I1C Energy Meter Platform
* Description  : EM Display Application Layer APIs
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
***********************************************************************************************************************/
/* Driver layer */
#include "r_cg_macrodriver.h"
#include "typedef.h"            /* GCSE Standard definitions */
#include "relay.h"
#include "r_cg_tau.h"
//#include "event.h"
#include "r_cg_wdt.h"
//#include "r_cg_wdt.h"
//#include <string.h>
//#include <stdio.h>
//#include <stdarg.h>           /* Variant argument */
//#include "em_keypad.h"
//#include "storage.h"
//#include "r_dlms_event.h"
//#include "r_dlms_app.h"
//#include "crc16.h"
#include "main.h"
#include "Load_Control.h"
#include "factory_settings.h"
uint8_t RELAY_SetStatusValue = 0;
uint8_t reconnect_relay = 0;
uint8_t relay_connect_state = 0;
uint8_t relay_disconnect_state = 0;
/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/
/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
#ifdef SINGLE_PHASE_METER
  #define RELAY_PULSE_DURATION_COUNT  13000
#else
  #define RELAY_PULSE_DURATION_COUNT  15000
#endif
#define RELAY_IS_STATUS_VALID(status)   (status == RELAY_CONNECTED || status == RELAY_DISCONNECTED)
#define RELAY_VALIDATE_AND_CORRECT_READ_STATUS(status, set_func) {  \
    if (status != RELAY_CONNECTED &&                                \
        status != RELAY_DISCONNECTED) {                             \
        status = RELAY_CONNECTED;                                   \
        set_func(status);                                           \
    }                                                               \
}
/***********************************************************************************************************************
Imported global variables and functions (from other files)
***********************************************************************************************************************/

/******************************************************************************
Exported global variables and functions (to be accessed by other files)
******************************************************************************/


/******************************************************************************
Private global variables and functions
******************************************************************************/

/***********************************************************************************************************************
* Function Name    : void Switch_Delay(void)
* Description      :
* Arguments        : None
* Functions Called : None
* Return Value     : None
***********************************************************************************************************************/
void Switch_Delay(void)
{
    uint16_t i;
    for (i = 0; i < RELAY_PULSE_DURATION_COUNT; i++)
    {
        NOP();
    }
    R_WDT_Restart();
    for (i = 0; i < RELAY_PULSE_DURATION_COUNT; i++)
    {
        NOP();
    }
}
/***********************************************************************************************************************
* Function Name    : void RELAY_PollingProcessing(void)
* Description      :
* Arguments        : None
* Functions Called : None
* Return Value     : None
***********************************************************************************************************************/
/*
void RELAY_PollingProcessing(uint8_t init)
{
    if (relay_connect_state != 0)               //REVIEW: both relays operate on single pin only now
    {
        switch (relay_connect_state)
        {
        case 1:
            relay_connect_state = 2;
            RELAY_SwitchOn_R();
            break;
        case 2:
            Switch_Delay();
            relay_connect_state = 3;
            RELAY_SwitchOn_Y();
            break;
        case 3:
            Switch_Delay();
            RELAY_SwitchOn_B();
            relay_connect_state = 0;
            break;
        default:
            relay_connect_state = 1;
            break;
        }
    }
    else if (relay_disconnect_state != 0)
    {
        switch (relay_disconnect_state)
        {
        case 1:
            relay_disconnect_state = 2;
            RELAY_SwitchOff_R();
            break;
        case 2:
            Switch_Delay();
            relay_disconnect_state = 3;
            RELAY_SwitchOff_Y();
            break;
        case 3:
            Switch_Delay();
            RELAY_SwitchOff_B();
            relay_disconnect_state = 0;
            break;
        default:
            relay_disconnect_state = 0;
            relay_connect_state = 1;
            break;
        }
    }
}
*/
void RELAY_PollingProcessing(uint8_t init)
{
    if (relay_connect_state != 0)               //REVIEW: both relays operate on single pin only now
    {
        RELAY_SwitchOn_R();
        relay_connect_state = 0;
    }
    else if (relay_disconnect_state != 0)
    {
        RELAY_SwitchOff_R();
        relay_disconnect_state = 0;
    }
}
//Rakesh

/* Relay init on wake up */
//void relay_init(void)
//{
//    static uint8_t init_once = 0;
//    uint16_t crc, crc_cal;
//    if (init_once == 0)
//    {
//        if (g_event_state.over_current == EVENT_STATE_ENTER)
//        {
//            g_event_state.over_current = EVENT_STATE_RELEASE;
//            R_DLMS_Event_Detected_OverCurrent(EVENT_STATE_RELEASE);
//            EVENT_BackupState();
//        }
//        if (g_event_state.overload == EVENT_STATE_ENTER)
//        {
//            g_event_state.overload = EVENT_STATE_RELEASE;
//            R_DLMS_Event_Detected_OverLoad(EVENT_STATE_RELEASE);
//            EVENT_BackupState();
//        }

//        /*latch control state*/
//        crc = gen_crc16((const uint8_t*)&g_dlms_ctrl_control_state, 0, sizeof(uint8_t), Final);
//        EPR_Read(STORAGE_EEPROM_DLMS_RELAY_CONTROL_STATE_CRC_ADDR, (uint8_t *)&crc_cal, STORAGE_EEPROM_DLMS_RELAY_CONTROL_STATE_CRC_SIZE);
//        if (crc != crc_cal)
//        {
//            R_DLMS_Ctrl_SetState(CTRL_STATE_CONN);
//        }
//        init_once = 1;
//    }

//    if (g_dlms_ctrl_control_state != 0)
//    {
//        relay_connect_state = 1;
//    }
//    else
//    {
//        relay_disconnect_state = 1;
//    }
//}
/***********************************************************************************************************************
* Function Name    : void RELAY_Initialize(void)
* Description      :
* Arguments        : None
* Functions Called : None
* Return Value     : None
***********************************************************************************************************************/
void RELAY_Initialize(void)
{
    /*port seting*/
    //PFSEG3 &= 0xFE;
    PM8 &= 0xCF;
    P8 &= 0xCF;

    //PFSEG2 &= 0x1F;
    PM5 &= 0x0F;
    P5 &= 0x0F;

    /*clear the flag*/
}


/***********************************************************************************************************************
* Function Name    : void RELAY_SwitchOn(void)
* Description      :
* Arguments        : None
* Functions Called : None
* Return Value     : None
***********************************************************************************************************************/
void RELAY_SwitchOff(void)
{
    relay_connect_state = 0;
    relay_disconnect_state = 1;
}
//Rakesh
void RELAY_SwitchOff_R(void)
{
    SuperCapCharge_ticker_Callback = 0;
    S_CAP_CHARGE_DISABLE();

    RELAY_R_PRESET = RELAY_ON;       /* for P2 */
    RELAY_R_SET = RELAY_OFF;      /* for P2 */

    Switch_Delay();

    RELAY_R_PRESET = RELAY_OFF;      /* for P2 */
    RELAY_R_SET = RELAY_OFF;      /* for P2 */

    SuperCapCharge_ticker_Callback = SuperCapCharge_ticker;
}

void RELAY_SwitchOff_Y(void)
{
#ifndef SINGLE_PHASE_METER
    RELAY_Y_PRESET = RELAY_ON;       /* for P3 */
    RELAY_Y_SET = RELAY_OFF;      /* for P3 */

    Switch_Delay();

    RELAY_Y_PRESET = RELAY_OFF;      /* for P3 */
    RELAY_Y_SET = RELAY_OFF;      /* for P3 */
#endif
}

void RELAY_SwitchOff_B(void)
{
    RELAY_B_PRESET = RELAY_ON;       /* for P4 */
    RELAY_B_SET = RELAY_OFF;      /* for P4 */

    Switch_Delay();

    RELAY_B_PRESET = RELAY_OFF;      /* for P4 */
    RELAY_B_SET = RELAY_OFF;      /* for P4 */
}
//end
/***********************************************************************************************************************
* Function Name    : void RELAY_SwitchOff(void)
* Description      :
* Arguments        : None
* Functions Called : None
* Return Value     : None
***********************************************************************************************************************/
void RELAY_SwitchOn(void)
{
    relay_connect_state = 1;
    relay_disconnect_state = 0;
}
//Rakesh
void RELAY_SwitchOn_R(void)
{
    SuperCapCharge_ticker_Callback = 0;
    S_CAP_CHARGE_DISABLE();

    RELAY_R_PRESET = RELAY_OFF;      /* for P2 */
    RELAY_R_SET = RELAY_ON;       /* for P2 */

    Switch_Delay();

    RELAY_R_PRESET = RELAY_OFF;      /* for P2 */
    RELAY_R_SET = RELAY_OFF;      /* for P2 */

    SuperCapCharge_ticker_Callback = SuperCapCharge_ticker;
}

void RELAY_SwitchOn_Y(void)
{
#ifndef SINGLE_PHASE_METER
    RELAY_Y_PRESET = RELAY_OFF;      /* for P3 */
    RELAY_Y_SET = RELAY_ON;       /* for P3 */

    Switch_Delay();

    RELAY_Y_PRESET = RELAY_OFF;      /* for P3 */
    RELAY_Y_SET = RELAY_OFF;      /* for P3 */
#endif
}

void RELAY_SwitchOn_B(void)
{
    RELAY_B_PRESET = RELAY_OFF;      /* for P4 */
    RELAY_B_SET = RELAY_ON;       /* for P4 */

    Switch_Delay();

    RELAY_B_PRESET = RELAY_OFF;      /* for P4 */
    RELAY_B_SET = RELAY_OFF;      /* for P4 */
}
//end
/***********************************************************************************************************************
* Function Name    : void RELAY_GetStatus(void)
* Description      :
* Arguments        : None
* Functions Called : None
* Return Value     : uint8_t RELAY_OFF/RELAY_ON
***********************************************************************************************************************/
//uint8_t RELAY_GetStatus(void)
//{
//    uint8_t status;
//    uint16_t crc, crc_bak;

//    EPR_Read(
//        STORAGE_EEPROM_MISC_RELAY_STATUS_ADDR,
//        (uint8_t *)&status,
//        STORAGE_EEPROM_MISC_RELAY_STATUS_SIZE
//    );

//    RELAY_VALIDATE_AND_CORRECT_READ_STATUS(
//        status,
//        RELAY_SetStatus
//    );

//    crc = gen_crc16((const uint8_t*)&status, 0, sizeof(uint8_t), Final);
//    EPR_Read(
//        STORAGE_EEPROM_DLMS_RELAY_STATE_CRC_ADDR,
//        (uint8_t *)&crc_bak,
//        STORAGE_EEPROM_DLMS_RELAY_STATE_CRC_SIZE
//    );

//    if (crc != crc_bak)
//    {
//        RELAY_SetStatus(RELAY_CONNECTED);
//        status = RELAY_CONNECTED;
//    }
//    RELAY_SetStatusValue = status;
//    return status;
//}

//void RELAY_SetStatus(uint8_t status)
//{
//    uint16_t crc;

//    if (!RELAY_IS_STATUS_VALID(status))
//    {
//        status = RELAY_CONNECTED;
//    }

//    {
//        EPR_Write(
//            STORAGE_EEPROM_MISC_RELAY_STATUS_ADDR,
//            (uint8_t *)&status,
//            STORAGE_EEPROM_MISC_RELAY_STATUS_SIZE
//        );

//        crc = gen_crc16((const uint8_t*)&status, 0, sizeof(uint8_t), Final);
//        EPR_Write(
//            STORAGE_EEPROM_DLMS_RELAY_STATE_CRC_ADDR,
//            (uint8_t *)&crc,
//            STORAGE_EEPROM_DLMS_RELAY_STATE_CRC_SIZE
//        );

//        RELAY_SetStatusValue = status;
//    }
//}

//uint8_t RELAY_GetPreviousStatus(void)
//{
//    uint8_t status;
//
//    EPR_Read(
//        STORAGE_EEPROM_MISC_RELAY_STATUS_OLD_ADDR,
//        (uint8_t *)&status,
//        STORAGE_EEPROM_MISC_RELAY_STATUS_OLD_SIZE
//    );
//
//    RELAY_VALIDATE_AND_CORRECT_READ_STATUS(
//        status,
//        RELAY_SetPreviousStatus
//    );
//
//    return status;
//}
//
//
//void RELAY_SetPreviousStatus(uint8_t status)
//{
//    if (RELAY_IS_STATUS_VALID(status))
//    {
//        EPR_Write(
//            STORAGE_EEPROM_MISC_RELAY_STATUS_OLD_ADDR,
//            (uint8_t *)&status,
//            STORAGE_EEPROM_MISC_RELAY_STATUS_OLD_SIZE
//        );
//    }
//}

//void latch_check(void)
//{
//    static uint8_t timer = 0;
//    if ((g_dlms_ctrl_control_state == CTRL_STATE_CONN) && (g_event_state.over_current == EVENT_STATE_RELEASE) && (g_event_state.overload == EVENT_STATE_RELEASE) && (RELAY_SetStatusValue != 1))
//    {
//        if (timer++ > 20)
//        {
//            RELAY_SetStatus(RELAY_CONNECTED);
//            relay_connect_state = 1;
//        }
//    }
//    else
//    {
//        timer = 0;
//    }
//}