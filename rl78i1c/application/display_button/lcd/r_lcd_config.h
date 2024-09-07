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
* File Name    : r_lcd_config.h
* Version      : 1.00
* Device(s)    : RL78/I1C
* Tool-Chain   : CA78K0R
* H/W Platform : RL78/I1C Energy Meter Platform
* Description  : Declare const array for LCD configuration
***********************************************************************************************************************/

#ifndef _R_LCD_CONFIG_H
#define _R_LCD_CONFIG_H

/***********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
***********************************************************************************************************************/
#include "typedef.h"

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/
/*
 * LCD information definition
 * The data area which need to be changed when changing LCD
 */

#define _BV(x)  (1<<x)
//Numeric map
#define a7 _BV(0)
#define b7 _BV(2)
#define c7 _BV(5)
#define d7 _BV(6)
#define e7 _BV(4)
#define g7 _BV(3)
#define f7 _BV(1)
#define dot7 _BV(7)


#define     LCD_NUM_DIGIT_CHAR      47      /* number of displayable char for a digit (0..9, A..F, -) */
#define     LCD_NUM_DIGIT           8       /* number of digits in LCD panel */
#define     LCD_NUM_SIGN            60      /* number of sign in LCD panel */

#define     LCD_NUM_DIGIT_TZ        2       /* number of digits in LCD panel */

#define     LCD_FIRST_POS_DIGIT     1       /* Specify the first position of digit on LCD screen */
#define     LCD_LAST_POS_DIGIT      8       /* Specify the last position of digit on LCD screen */

 /* Some special characters can be displayed on LCD screen */
#define     LCD_CHAR_o              '.'      /* To display minus on LCD digit */
#define     LCD_MINUS_SIGN          '-'      /* To display minus on LCD digit */
#define     LCD_CLEAR_DIGIT         15      /* Value which is used to clear digit on LCD */
#define     LCD_CHAR_A              'A'      /* Display character A on LCD */
#define     LCD_CHAR_B              'B'      /* Display character b on LCD */
#define     LCD_CHAR_C              'C'      /* Display character C on LCD */
#define     LCD_CHAR_D              'D'      /* Display character d on LCD */
#define     LCD_CHAR_E              'E'      /* Display character E on LCD */
#define     LCD_CHAR_F              'F'      /* Display character F on LCD */
#define     LCD_CHAR_P              'P'      /* Display character P on LCD */
#define     LCD_CHAR_L              'L'      /* Display character L on LCD */
#define     LCD_CHAR_N              'N'      /* Display character N on LCD */
#define     LCD_CHAR_U              'U'      /* Display character U on LCD */
#define     LCD_CHAR_T              'T'      /* Display character T on LCD */
#define     LCD_CHAR_R              'R'      /* Display character r on LCD */

/* LCD number type */
typedef enum tagLCDNumType
{
    NUM_TYPE_A = 0,         /* Bit No.:  7 6 5 4 3 2 1 0
                             * Seg No.:- C B A D E G F
                             */
    NUM_TYPE_AN = 1,

} LCD_NUM_TYPE;

/* LCD special sign enummeration */
typedef enum tagLCDEMSpecSign
{
    S_OK = 0,
    S_DATE,
    S_TIME,
    S_EARTH,
    S_REV,
    S_MAINS,
    S_MAG,
    S_BILL,
    S_PF,
    S_T10,
    S_T11,
    S_T12,
    S_T13,
    S_R, //  R
    S_Y, //  Y
    S_B, //  B
    S_T8,
    S_h, //  h
    S_r, //  r
    S_A, //  A
    S_FWSL, //  / fwd slash
    S_BWSL, //  backward slash
    S_V, //  V
    S_k, //  k
    S_D6,
    S_D5,
    S_D4,
    S_D3,
    S_D2,
    S_D1,
    S_T9,
    S_TOD,
    S_MD,
    S_SEP,
    S_IMPO,
    S_EXPO,
    S_QPOS,
    S_QNEG,
    S_LOGO,
    S_COPN,
    S_NMIS,
    S_WARN,
    S_CLK,
    S_BAT,
    S_CONT,
    S_DCONT,
    S_PP,
    S_DGREE,
    S_RX,
    S_TX,
    S_SIG1,
    S_SIG2,
    S_SIG3,
    S_SIG4,
    S_SIG5,
    S_RUPEE,
    S_SNO,
    S_BYPAS,
    S_SLOAD,
} LCD_EM_SPEC_SIGN;

/* LCD number and sign mapping on data RAM */
typedef struct tagLCDNumMap
{
    uint8_t         pos;        /* number no.   */
    uint32_t        addr;       /* ram address  */
    LCD_NUM_TYPE    type;       /* num map type in ram of this number */
} LCD_NUM_MAP;

typedef struct tagLCDNumTypeValue
{
    uint8_t     typeA;
} LCD_NUM_TYPE_VALUE;

typedef struct tagLCDAlfaNumTypeValue
{
    uint8_t     typeA;
    uint8_t     typeB;
} LCD_ALFANUM_TYPE_VALUE;

typedef struct tagLCDSignMap
{
    LCD_EM_SPEC_SIGN    sign;       /* special sign */
    uint32_t            addr;       /* ram address  */
    uint8_t             pos;        /* bit number of this sign in pwRAM */
} LCD_SIGN_MAP;

/* Information for displaying data */
typedef struct tagLCDNumInfo
{
    uint32_t    addr;
    uint8_t     value;
    uint8_t     value_2;
} LCD_NUM_INFO;

typedef struct tagLCDSignInfo
{
    uint32_t    addr;
    uint8_t     pos;
} LCD_SIGN_INFO;

typedef struct tagLCDDecInfo
{
    uint8_t pos;
    LCD_EM_SPEC_SIGN sign;
} LCD_DECIMAL_INFO;

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Variable Externs
***********************************************************************************************************************/
extern const LCD_NUM_MAP        g_LCD_NumMap[];
extern const LCD_NUM_TYPE_VALUE g_LCD_NumType[];
extern const LCD_ALFANUM_TYPE_VALUE g_LCD_AlfaNumType[];
extern const LCD_SIGN_MAP       g_LCD_SignMap[];
extern const LCD_DECIMAL_INFO g_DecInfo5;
extern const LCD_DECIMAL_INFO g_DecInfo3;
extern const LCD_DECIMAL_INFO g_DecInfo2;
extern const LCD_DECIMAL_INFO g_DecInfo1;

extern const LCD_NUM_MAP        g_LCD_NumMap_TZ[];

/***********************************************************************************************************************
Functions Prototypes
***********************************************************************************************************************/

#endif /* _R_LCD_CONFIG_H */
