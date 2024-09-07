/******************************************************************************
* DISCLAIMER

* This software is supplied by Renesas Electronics Corporation and is only
* intended for use with Renesas products. No other uses are authorized.

* This software is owned by Renesas Electronics Corporation and is protected under
* all applicable laws, including copyright laws.

* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES
* REGARDING THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY,
* INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
* PARTICULAR PURPOSE AND NON-INFRINGEMENT.  ALL SUCH WARRANTIES ARE EXPRESSLY
* DISCLAIMED.

* TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS
* ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE
* FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES
* FOR ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR ITS
* AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

* Renesas reserves the right, without notice, to make changes to this
* software and to discontinue the availability of this software.
* By using this software, you agree to the additional terms and
* conditions found by accessing the following link:
* http://www.renesas.com/disclaimer
******************************************************************************/
/* Copyright (C) 2011 Renesas Electronics Corporation. All rights reserved.  */
/******************************************************************************
* File Name    : key.c
* Version      : 1.00
* Device(s)    : RL78/I1C
* Tool-Chain   : CubeSuite Version 1.5d
* H/W Platform : RL78/I1C Energy Meter Platform
* Description  : KEY processing source File
******************************************************************************
* History : DD.MM.YYYY Version Description
******************************************************************************/

/******************************************************************************
Includes   <System Includes> , "Project Includes"
******************************************************************************/
/* Driver */
#include "r_cg_macrodriver.h"   /* CG Macro Driver */

/* MW/Core */
#include "em_core.h"        /* EM Core APIs */

/* Application */
#include "key.h"            /* KEY Interface Header File */
#include "em_display.h"     /* LCD Display Header File */

uint8_t Key_Press_Counter = KEY_PRESS_COUNTER_TIMEOUT_RESET;
/******************************************************************************
Typedef definitions
******************************************************************************/
/* Key flag */
typedef struct tagEventFlag
{
    uint8_t is_key_down_pressed : 1;    /* Key down flag */
    uint8_t is_key_mid_pressed : 1;    /* Key mid flag */
    uint8_t is_key_up_pressed : 1;    /* Key up flag */
    uint8_t reserved : 5;    /* (NO USE) Reserved */

} KEY_FLAG;

/******************************************************************************
Macro definitions
******************************************************************************/

/******************************************************************************
Imported global variables and functions (from other files)
******************************************************************************/
//extern 
uint16_t g_wSleepCounter;
/******************************************************************************
Exported global variables and functions (to be accessed by other files)
******************************************************************************/

/******************************************************************************
Private global variables and functions
******************************************************************************/
static KEY_FLAG g_key_flag = { 0, 0, 0 };

/******************************************************************************
* Function Name    : void KEY_DownPressed(void)
* Description      : Event Callback for Key Down Pressed
* Arguments        : None
* Functions Called : None
* Return Value     : None
******************************************************************************/
void KEY_DownPressed(void)
{
    g_key_flag.is_key_down_pressed = 1;
}

/******************************************************************************
* Function Name    : void KEY_MidPressed(void)
* Description      : Event Callback for Key Mid Pressed
* Arguments        : None
* Functions Called : None
* Return Value     : None
******************************************************************************/
void KEY_MidPressed(void)
{
    g_key_flag.is_key_mid_pressed = 1;
}

/******************************************************************************
* Function Name    : void EVENT_KeyUpPressed(void)
* Description      : Event Callback for Key Up Pressed
* Arguments        : None
* Functions Called : None
* Return Value     : None
******************************************************************************/
void KEY_UpPressed(void)
{
    g_key_flag.is_key_up_pressed = 1;
}

void keys_scan(void)
{
    /* Detection on port here */
    static int8_t key_down_last = (!KEY_PRESSED);
    static int8_t key_mid_last = (!KEY_PRESSED);
    static int8_t key_up_last = (!KEY_PRESSED);

#if 0
    if (KEY_DOWN == (!KEY_PRESSED) &&
        key_down_last == KEY_PRESSED)
    {
        KEY_DownPressed();
    }
    key_down_last = KEY_DOWN;
#endif

#if 0
    if (KEY_MID == (!KEY_PRESSED) &&
        key_mid_last == KEY_PRESSED)
    {
        KEY_MidPressed();
    }
    key_mid_last = KEY_MID;
#endif

#if 1
    if (KEY_UP == (!KEY_PRESSED) &&
        key_up_last == KEY_PRESSED)
    {
        KEY_UpPressed();
    }
    key_up_last = KEY_UP;
    //KEY_PollingProcessing();
#endif
}
/******************************************************************************
* Function Name    : void KEY_PollingProcessing(void)
* Description      : KEY Polling Processing
* Arguments        : None
* Functions Called : None
* Return Value     : None
******************************************************************************/
void KEY_PollingProcessing(void)
{
    //keys_scan();
#if 0
    /* When key DOWN is pressed */
    if (g_key_flag.is_key_down_pressed != 0)    //REVIEW: Disable with MACROS... DONE
    {
        /* Switch the auto scroll of LCD */
        Key_Press_Counter = KEY_PRESS_COUNTER_TIMEOUT_RESET;
        LCD_SwitchAutoRoll();

        /* ACK */
        g_key_flag.is_key_down_pressed = 0;
    }
#endif

#if 0
    /* When key MID is pressed */
    if (g_key_flag.is_key_mid_pressed != 0)
    {
        /* ACK */
        Key_Press_Counter = KEY_PRESS_COUNTER_TIMEOUT_RESET;
        g_key_flag.is_key_mid_pressed = 0;
    }
#endif

#if 1
    /* When key UP is pressed */
    if (g_key_flag.is_key_up_pressed != 0)
    {
        /* Change to the next display item of LCD */
        Key_Press_Counter = (IS_MAINS_PRESENT()) ? KEY_PRESS_COUNTER_TIMEOUT_RESET : KEY_PRESS_COUNTER_TIMEOUT_BATTERY_MODE;
        LCD_ChangeNext();
        g_wSleepCounter = 57;

        /* ACK */
        g_key_flag.is_key_up_pressed = 0;
    }
#endif
}
