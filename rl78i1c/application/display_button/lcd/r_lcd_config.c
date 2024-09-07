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
* File Name    : r_lcd_config.c
* Version      : 1.00
* Device(s)    : RL78/I1C
* Tool-Chain   : CA78K0R
* H/W Platform : RL78/I1C Energy Meter Platform
* Description  : Declare const array for LCD configuration
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
***********************************************************************************************************************/
#include "r_lcd_config.h"
#include "r_drv_lcd_ext.h"

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/
//Alfa numeric map
#define a _BV(0)
#define j _BV(1)
#define b _BV(2)
#define g2 _BV(3)
#define l _BV(4)
#define k _BV(5)
#define c _BV(6)

#define f _BV(0)
#define h _BV(1)
#define i _BV(2)
#define g1 _BV(3)
#define e _BV(4)
#define m _BV(5)
#define d _BV(6)

/***********_BV(6)******************************************************************************************************
Macro definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Imported global variables and functions (from other files)
***********************************************************************************************************************/

/***********************************************************************************************************************
Exported global variables and functions (to be accessed by other files)
***********************************************************************************************************************/
/*
 * Data mapping:
 * This data must be changed when there is any difference on PIN
 * (COM and SEGMENT) connection setting between LCD and MCU
 */
const LCD_NUM_MAP g_LCD_NumMap[LCD_NUM_DIGIT] =
{
    /* Pos   Address Number Type */
    {1,    LCD_RAM_START_ADDRESS + 15,   NUM_TYPE_AN},
    {2,    LCD_RAM_START_ADDRESS + 13,   NUM_TYPE_AN},
    {3,    LCD_RAM_START_ADDRESS + 11,   NUM_TYPE_A},
    {4,    LCD_RAM_START_ADDRESS + 10,   NUM_TYPE_A},
    {5,    LCD_RAM_START_ADDRESS + 9,    NUM_TYPE_A},
    {6,    LCD_RAM_START_ADDRESS + 8,    NUM_TYPE_A},
    {7,    LCD_RAM_START_ADDRESS + 7,    NUM_TYPE_A},
    {8,    LCD_RAM_START_ADDRESS + 6,    NUM_TYPE_A},

#ifdef CRYSTAL_SMALL_LCD
    // Pos   Address Number Type
    {1,    LCD_RAM_START_ADDRESS + 12,   NUM_TYPE_A},
    {2,    LCD_RAM_START_ADDRESS + 11,   NUM_TYPE_A},
    {3,    LCD_RAM_START_ADDRESS + 10,   NUM_TYPE_A},
    {4,    LCD_RAM_START_ADDRESS + 9,    NUM_TYPE_A},
    {5,    LCD_RAM_START_ADDRESS + 8,    NUM_TYPE_A},
    {6,    LCD_RAM_START_ADDRESS + 7,    NUM_TYPE_A},
    {7,    LCD_RAM_START_ADDRESS + 14,   NUM_TYPE_A},
#endif
};

const LCD_NUM_MAP g_LCD_NumMap_TZ[LCD_NUM_DIGIT_TZ] =
{
    /* Pos   Address Number Type */
    {1,    LCD_RAM_START_ADDRESS + 19,   NUM_TYPE_A},
    {2,    LCD_RAM_START_ADDRESS + 17,   NUM_TYPE_A},
};

const LCD_NUM_TYPE_VALUE g_LCD_NumType[LCD_NUM_DIGIT_CHAR] =
{
    /*porting*/

    g7, 				            // [0] = -
    dot7,                           // [1] = Null
    c7 + d7 + e7 + g7,                // o
    //0,                              // [2] = Null
    a7 + b7 + c7 + d7 + e7 + f7,		        // [3] = 0
    b7 + c7,				            // [4] = 1
    b7 + a7 + d7 + e7 + g7,			        // [5] = 2
    c7 + b7 + a7 + d7 + g7,			        // [6] = 3
    c7 + b7 + g7 + f7,			        // [7] = 4
    c7 + a7 + d7 + g7 + f7,			        // [8] = 5
    c7 + a7 + d7 + e7 + g7 + f7,		        // [9] = 6
    c7 + b7 + a7,			            // [10] = 7
    a7 + b7 + c7 + d7 + e7 + f7 + g7,	        // [11] = 8
    c7 + d7 + b7 + a7 + g7 + f7,		        // [12] = 9
    0,                              // [13] = Null
    0,                              // [14] = Null
    0,                              // [15] = Null
    0,                              // [16] = Null
    0,                              // [17] = Null
    0,                              // [18] = Null
    0,                              // [19] = Null
    a7 + b7 + c7 + e7 + f7 + g7,		        // [20] = A
    c7 + d7 + e7 + f7 + g7,			        // [21] = b
    a7 + d7 + e7 + f7,			        // [22] = C
    c7 + b7 + d7 + e7 + g7,			        // [23] = d
    a7 + d7 + e7 + f7 + g7,		            // [24] = E
    a7 + e7 + f7 + g7,			        // [25] = F
    c7 + a7 + d7 + e7 + f7,			        // [26] = G
    b7 + c7 + e7 + f7 + g7,			        // [27] = H
    e7 + f7,				            // [28] = 1
    b7 + c7 + d7 + e7,			        // [29] = J
    0,                              // [30] = Null
    d7 + e7 + f7,			            // [31] = L
    a7 + b7 + c7 + e7 + f7,			        // [32] = M
    c7 + e7 + g7,			            // [33] = N
    a7 + b7 + c7 + d7 + e7 + f7,		        // [34] = 0
    b7 + a7 + e7 + g7 + f7,			        // [35] = P
    0,                              // [36] = Null
    e7 + g7,				            // [37] = r
    c7 + a7 + d7 + g7 + f7,			        // [37] = 5
    d7 + e7 + g7 + f7,		            // [38] = t
    c7 + d7 + e7,			            // [39] = u
    b7 + c7 + d7 + e7 + f7,			        // [40] = V0
    0,                              // [41] = y
    0,                              // [42] = Null
    b7 + c7 + d7 + f7 + g7,			        // [43] = y
    0,				                // [44] = Null
    0				                // [45] = Null 
    /*porting*/
#ifdef CRYSTAL_SMALL_LCD
    //Pos   Address Number Type
    {0x77},//1011 1110//    { 0xD7 }    , // '0' - 0*1101 0111 /  
    {0x12},//0000 0110//    { 0x06 }    , // '1' - 1*0000 0110 /
    {0x5D},//0111 1100//    { 0xE3 }    , // '2' - 2*1110 0011 /
    {0x5B},//0101 1110//    { 0xA7 }    , // '3' - 3*1010 0111 /
    {0x3A},//1100 0110//    { 0x36 }    , // '4' - 4*0011 0110 /
    {0x6B},//1101 1010//    { 0xB5 }    , // '5' - 5*1011 0101 /
    {0x6F},//1111 1010//    { 0xF5 }    , // '6' - 6*1111 0101 /
    {0x52},//0000 1110//    { 0x07 }    , // '7' - 7*0000 0111 /
    {0x7F},//1111 1110//    { 0xF7 }    , // '8' - 8*1111 0111 /
    {0x7B},//1101 1110//    { 0xB7 }    , // '9' - 9*1011 0111 /
    {0x7E},//1110 1110//    { 0x77 }    , // 'A' - 10 0111 0111 /
    {0x2F},//1111 0010//    { 0xF4 }    , // 'b' - 11 1111 0100 /
    {0x65},//1011 1000//    { 0xD1 }    , // 'C' - 12 1101 0001 /
    {0x1F},//0111 0110//    { 0xE6 }    , // 'd' - 13 1110 0110 /
    {0x6D},//1111 1000//    { 0xF1 }    , // 'E' - 14 1111 0001 /
    {0x6C},//1110 1000//    { 0x71 }    , // 'F' - 15 0111 0001 /
    {0x08},//0100 0000//    { 0x20 }    , // '-' - 16 0010 0000 /
    {0x7C},//1110 1100//    { 0x73 }    , // 'P' - 17 0111 0011 /
    {0x25},//1011 0000//    { 0xD0 }    , // 'L' - 18 1101 0000 /
    {0x0E},//1010 1110//    { 0x57 }    , // 'n' - 19 0101 0111 /
    {0x80},//0000 0001//    { 0x08 }    , // na - 20 0000 1000 /
    {0x37},//1011 0110//    { 0xD6 }    , // 'U' - 21 1101 0110 /
    {0x2d},                               // 't' - 22/
    {0x0C},                               // 'r' - 23/
#endif
#ifdef RENESAS_LCD	
    /*porting*/
    {0xBE},//1011 1110//    { 0xD7 }    , // '0' - 0* 1101 0111 /  
    {0x06},//0000 0110//    { 0x06 }    , // '1' - 1* 0000 0110 /
    {0x7C},//0111 1100//    { 0xE3 }    , // '2' - 2* 1110 0011 /
    {0x5E},//0101 1110//    { 0xA7 }    , // '3' - 3* 1010 0111 /
    {0xC6},//1100 0110//    { 0x36 }    , // '4' - 4* 0011 0110 /
    {0xDA},//1101 1010//    { 0xB5 }    , // '5' - 5* 1011 0101 /
    {0xFA},//1111 1010//    { 0xF5 }    , // '6' - 6* 1111 0101 /
    {0x0E},//0000 1110//    { 0x07 }    , // '7' - 7* 0000 0111 /
    {0xFE},//1111 1110//    { 0xF7 }    , // '8' - 8* 1111 0111 /
    {0xDE},//1101 1110//    { 0xB7 }    , // '9' - 9* 1011 0111 /
    {0xEE},//1110 1110//    { 0x77 }    , // 'A' - 10* 0111 0111 /
    {0xF2},//1111 0010//    { 0xF4 }    , // 'b' - 11* 1111 0100 /
    {0xB8},//1011 1000//    { 0xD1 }    , // 'C' - 12* 1101 0001 /
    {0x76},//0111 0110//    { 0xE6 }    , // 'd' - 13* 1110 0110 /
    {0xF8},//1111 1000//    { 0xF1 }    , // 'E' - 14* 1111 0001 /
    {0xE8},//1110 1000//    { 0x71 }    , // 'F' - 15* 0111 0001 /
    {0x40},//0100 0000//    { 0x20 }    , // '-' - 16* 0010 0000 /
    {0xEC},//1110 1100//    { 0x73 }    , // 'P' - 17* 0111 0011 /
    {0xB0},//1011 0000//    { 0xD0 }    , // 'L' - 18* 1101 0000 /
    {0xAE},//1010 1110//    { 0x57 }    , // 'N' - 19* 0101 0111 /
    {0x01},//0000 0001//    { 0x08 }    , // na - 20* 0000 1000 /
    {0xB6},//1011 0110//    { 0xD6 }    , // 'U' - 21* 1101 0110 /
    {0xF0},                               // 't' - 22*/
    {0x60},                               // 'r' - 23*/
    /*porting*/
#endif
};

const LCD_ALFANUM_TYPE_VALUE g_LCD_AlfaNumType[LCD_NUM_DIGIT_CHAR] =
{
    //{a-j-b-g2-l-k-c},{f,h,i,g1,e,m,d}
      {g2 , g1},							//'-'
      {0 , 0},							//'.'
      {j , m},							//'/'
      {a + b + c , d + e + f},					//'0'
      {b + c , 0},							//'1'
      {a + b + g2 , g1 + e + d},					//'2'
      {a + b + g2 + c , g1 + d},					//'3'
      {b + c + g2 , g1 + f},					//'4'
      {a + c + g2 , g1 + f + d},					//'5'
      {a + c + g2 , g1 + e + f + d},				//'6'
      {a + b + c , 0},						//'7'
      {a + b + c + g2 , d + e + f + g1},				//'8'
      {a + b + c + g2 , d + f + g1},				//'9'
      {0 , 0},							//Null
      {0 , 0},							//Null
      {0 , 0},							//Null
      {0 , 0},							//Null
      {0 , 0},							//Null
      {0 , 0},							//Null
      {0 , 0},							//Null
      {a + b + c + g2 , e + f + g1},				//'A'
      {a + b + c + g2 + l , d + i},					//'B'
      {a , d + e + f},						//'C'
      {a + b + c + l , d + i},					//'D'
      {a , d + e + f + g1},						//'E'
      {a , e + f + g1},						//'F'
      {a + g2 + c , d + e + f},					//'G'
      {b + c + g2 , e + f + g1},					//'H'
      {a + l , i + d},						//'I'
      {b + c , e + d},						//'J'
      {j + k , g1 + e + f},						//'K'
      {0 , d + e + f},						//'L'
      {j + c + b , h + f + e},					//'M'
      {c + b + k , h + f + e},					//'N'
      {a + b + c , d + e + f},					//'O'
      {a + b + g2 , g1 + e + f},					//'P'
      {a + b + c + k , d + e + f},					//'Q'
      {a + b + k , e + f + g1},					//'R'
      {a + k , h + d},						//'S'
      {a + l , i},							//'T'
      {b + c , d + e + f},						//'U'
      {b + c + k , h},						//'V'
      {b + c + k , e + f + m},					//'W'
      {j + k , h + m},						//'X'
      {j + l , h},							//'Y'
      {a + j , m + d},						//'Z'

};
/* Mapping all special sign of LCD to LCDRAM */
const LCD_SIGN_MAP g_LCD_SignMap[LCD_NUM_SIGN] =
{
    /* Porting*/
    /* Number No.   Address   Bit No. */
    {S_OK   , LCD_RAM_START_ADDRESS + 23, 3}, /* 00 */              //Not Use
    {S_DATE , LCD_RAM_START_ADDRESS + 15, 7}, /* 01 *///
    {S_TIME , LCD_RAM_START_ADDRESS + 15, 7}, /* 02 *///
    //{S_TIME , LCD_RAM_START_ADDRESS + 18, 3}, /* 02 *///
    {S_EARTH, LCD_RAM_START_ADDRESS + 5,  4}, /* 03 *///
    {S_REV  , LCD_RAM_START_ADDRESS + 17, 2}, /* 04 *///
    {S_MAINS   , LCD_RAM_START_ADDRESS + 4,  0}, /* 05 */          //Not Use
    {S_MAG  , LCD_RAM_START_ADDRESS + 5,  3}, /* 06 *///
    {S_BILL , LCD_RAM_START_ADDRESS + 5,  5}, /* 07 *///
    {S_PF   , LCD_RAM_START_ADDRESS + 5,  5}, /* 08 */             //Not Use
    {S_T10  , LCD_RAM_START_ADDRESS + 2 , 3}, /* 09 */             //Not Use
    {S_T11  , LCD_RAM_START_ADDRESS + 2 , 2}, /* 10 */             //Not Use
    {S_T12  , LCD_RAM_START_ADDRESS + 2 , 1}, /* 11 */             //Not Use
    {S_T13  , LCD_RAM_START_ADDRESS + 2 , 0}, /* 12 */             //Not Use
    {S_R    , LCD_RAM_START_ADDRESS + 1 , 3}, /* 13 */             //Not Use
    {S_Y    , LCD_RAM_START_ADDRESS + 1 , 2}, /* 14 */             //Not Use
    {S_B    , LCD_RAM_START_ADDRESS + 1 , 1}, /* 15 */             //Not Use
    {S_T8   , LCD_RAM_START_ADDRESS + 0 , 3}, /* 16 */             //Not Use
    {S_h    , LCD_RAM_START_ADDRESS + 5 , 2}, /* 17 */
    {S_r    , LCD_RAM_START_ADDRESS + 5 , 1}, /* 18 */
    {S_A    , LCD_RAM_START_ADDRESS + 5 , 0}, /* 19 */
    {S_FWSL , LCD_RAM_START_ADDRESS + 20, 2}, /* 20 */
    {S_BWSL , LCD_RAM_START_ADDRESS + 20, 1}, /* 21 */
    {S_V    , LCD_RAM_START_ADDRESS + 19, 0}, /* 22 */
    {S_k    , LCD_RAM_START_ADDRESS + 18, 0}, /* 23 */
    {S_D6   , LCD_RAM_START_ADDRESS + 21, 0}, /* 24 */             //Not Use
    {S_D5   , LCD_RAM_START_ADDRESS + 6,  7}, /* 25 */// P4
    {S_D4   , LCD_RAM_START_ADDRESS + 7,  7}, /* 26 */ //P3
    {S_D3   , LCD_RAM_START_ADDRESS + 8,  7}, /* 27 */ //P2
    {S_D2   , LCD_RAM_START_ADDRESS + 17, 0}, /* 28 */             //Not Use
    {S_D1   , LCD_RAM_START_ADDRESS + 11, 7}, /* 29 */ //P1
    {S_T9   , LCD_RAM_START_ADDRESS + 23, 0}, /* 30 */             //Not Use
    {S_TOD  , LCD_RAM_START_ADDRESS + 6,  1}, /* 31 *///             //Not Use
    {S_MD   , LCD_RAM_START_ADDRESS + 5,  6}, /* 32 *///
    {S_SEP  , LCD_RAM_START_ADDRESS + 10, 7}, /* 33 *///
    {S_IMPO , LCD_RAM_START_ADDRESS + 21, 1}, /* 34 *///             //Not Use
    {S_EXPO , LCD_RAM_START_ADDRESS + 21, 4}, /* 35 *///             //Not Use
    {S_QPOS , LCD_RAM_START_ADDRESS + 21, 2}, /* 34 *///             //Not Use
    {S_QNEG , LCD_RAM_START_ADDRESS + 21, 3}, /* 35 *///             //Not Use
    {S_LOGO , LCD_RAM_START_ADDRESS + 21, 5}, /* 36 *///             //Not Use
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    {S_COPN , LCD_RAM_START_ADDRESS + 13, 7}, /* 37 *///
    {S_NMIS , LCD_RAM_START_ADDRESS + 9 , 7}, /* 38 *///
    {S_WARN , LCD_RAM_START_ADDRESS + 19, 3}, /* 39 *///
    {S_CLK  , LCD_RAM_START_ADDRESS + 18, 3}, /* 40 *///
    {S_BAT  , LCD_RAM_START_ADDRESS + 17, 3}, /* 40 *///
    {S_CONT , LCD_RAM_START_ADDRESS + 16, 2}, /* 40 *///
    {S_DCONT, LCD_RAM_START_ADDRESS + 16, 3}, /* 40 *///
    {S_PP   , LCD_RAM_START_ADDRESS + 18, 2}, /* 40 *///
    {S_DGREE, LCD_RAM_START_ADDRESS + 20, 0}, /* 40 *///
    {S_RX   , LCD_RAM_START_ADDRESS + 19, 2}, /* 40 *///
    {S_TX   , LCD_RAM_START_ADDRESS + 19, 1}, /* 40 *///
    {S_SIG1 , LCD_RAM_START_ADDRESS + 18, 1}, /* 40 *///
    {S_SIG2 , LCD_RAM_START_ADDRESS + 17, 1}, /* 40 *///
    {S_SIG3 , LCD_RAM_START_ADDRESS + 16, 1}, /* 40 *///
    {S_SIG4 , LCD_RAM_START_ADDRESS + 16, 0}, /* 40 *///
    {S_SIG5 , LCD_RAM_START_ADDRESS + 17, 0}, /* 40 *///
    {S_RUPEE, LCD_RAM_START_ADDRESS + 18, 7}, /* 40 *///
    {S_SNO  , LCD_RAM_START_ADDRESS + 16, 7}, /* 40 *///
    {S_BYPAS, LCD_RAM_START_ADDRESS + 9 , 7}, /* 40 *///
    {S_SLOAD, LCD_RAM_START_ADDRESS + 14, 7}, /* 40 *///

    /* Porting*/
};
#ifdef CRYSTAL_SMALL_LCD
const LCD_SIGN_MAP g_LCD_SignMap[LCD_NUM_SIGN] =
{
    /* Porting*/
    /* Number No.   Address   Bit No. */
    {S_OK   , LCD_RAM_START_ADDRESS + 23, 3}, /* 00 */
    {S_DATE , LCD_RAM_START_ADDRESS + 6,  7}, /* 01 *///
    {S_TIME , LCD_RAM_START_ADDRESS + 6,  6}, /* 02 *///
    {S_EARTH, LCD_RAM_START_ADDRESS + 11, 7}, /* 03 *///
    {S_REV  , LCD_RAM_START_ADDRESS + 12, 7}, /* 04 *///
    {S_MN   , LCD_RAM_START_ADDRESS + 4,  0}, /* 05 */
    {S_MAG  , LCD_RAM_START_ADDRESS + 6 , 4}, /* 06 *///
    {S_BILL , LCD_RAM_START_ADDRESS + 5,  3}, /* 07 *///
    {S_PF   , LCD_RAM_START_ADDRESS + 5,  5}, /* 08 *///
    {S_T10  , LCD_RAM_START_ADDRESS + 2 , 3}, /* 09 */
    {S_T11  , LCD_RAM_START_ADDRESS + 2 , 2}, /* 10 */
    {S_T12  , LCD_RAM_START_ADDRESS + 2 , 1}, /* 11 */
    {S_T13  , LCD_RAM_START_ADDRESS + 2 , 0}, /* 12 */
    {S_R    , LCD_RAM_START_ADDRESS + 1 , 3}, /* 13 */
    {S_Y    , LCD_RAM_START_ADDRESS + 1 , 2}, /* 14 */
    {S_B    , LCD_RAM_START_ADDRESS + 1 , 1}, /* 15 */
    {S_T8   , LCD_RAM_START_ADDRESS + 0 , 3}, /* 16 */
    {S_h   , LCD_RAM_START_ADDRESS + 0 , 2}, /* 17 */
    {S_r   , LCD_RAM_START_ADDRESS + 0 , 1}, /* 18 */
    {S_A   , LCD_RAM_START_ADDRESS + 0 , 0}, /* 19 */
    {S_FWSL   , LCD_RAM_START_ADDRESS + 1 , 0}, /* 20 */
    {S_BWSL   , LCD_RAM_START_ADDRESS + 3 , 0}, /* 21 */
    {S_V    , LCD_RAM_START_ADDRESS + 5 , 2}, /* 22 */
    {S_k   , LCD_RAM_START_ADDRESS + 7 , 0}, /* 23 */
    {S_D6   , LCD_RAM_START_ADDRESS + 9 , 0}, /* 24 */
    {S_D5   , LCD_RAM_START_ADDRESS + 7,  7}, /* 25 *///
    {S_D4   , LCD_RAM_START_ADDRESS + 8,  7}, /* 26 */
    {S_D3   , LCD_RAM_START_ADDRESS + 9, 7}, /* 27 */
    {S_D2   , LCD_RAM_START_ADDRESS + 17, 0}, /* 28 */
    {S_D1   , LCD_RAM_START_ADDRESS + 11, 7}, /* 29 */
    {S_T9   , LCD_RAM_START_ADDRESS + 23, 0}, /* 30 */
    {S_TOD  , LCD_RAM_START_ADDRESS + 6,  1}, /* 31 *///
    {S_MD   , LCD_RAM_START_ADDRESS + 5,  4}, /* 32 *///
    /* Porting*/
};
#endif
const LCD_DECIMAL_INFO g_DecInfo5 = {4, S_D1 };
const LCD_DECIMAL_INFO g_DecInfo3 = {6, S_D3 };
const LCD_DECIMAL_INFO g_DecInfo2 = {7, S_D4 };
const LCD_DECIMAL_INFO g_DecInfo1 = {8, S_D5 };

/***********************************************************************************************************************
Private global variables and functions
***********************************************************************************************************************/
