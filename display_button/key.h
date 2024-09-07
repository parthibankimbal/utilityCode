/******************************************************************************
  Copyright (C) 2011 Renesas Electronics Corporation, All Rights Reserved.
*******************************************************************************
* File Name    : key.h
* Version      : 1.00
* Description  : KEY Interface Header file
******************************************************************************
* History : DD.MM.YYYY Version Description
******************************************************************************/

#ifndef _KEY_H
#define _KEY_H

/******************************************************************************
Includes   <System Includes> , "Project Includes"
******************************************************************************/
#include "r_cg_userdefine.h"    /* CG User Define */ 
/******************************************************************************
Typedef definitions
******************************************************************************/

/******************************************************************************
Macro definitions
******************************************************************************/
#ifndef KEY_PRESSED
#define KEY_PRESSED     0
#endif

#define KEY_PRESS_COUNTER_TIMEOUT_RESET 10
#define KEY_PRESS_COUNTER_TIMEOUT_BATTERY_MODE	30

#define     KEY_UP              PUSH_BUTTON_KEY
#define     COVER_OPEN          (BIT_SELECT(P4,2))
#define     COMM_MODULE_LINK    (BIT_SELECT(P12,5))

extern uint8_t Key_Press_Counter;
/******************************************************************************
Variable Externs
******************************************************************************/

/******************************************************************************
Functions Prototypes
******************************************************************************/
/* Key Polling Processing */
void keys_scan(void);
void KEY_PollingProcessing(void);

/* Callback */
void KEY_DownPressed(void);
void KEY_MidPressed(void);
void KEY_UpPressed(void);

#endif /* _KEY_H */

