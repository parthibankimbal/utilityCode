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
* File Name    : r_lcd_display.h
* Version      : 1.00
* Device(s)    : RL78/I1C
* Tool-Chain   : CA78K0R
* H/W Platform : RL78/I1C Energy Meter Platform
* Description  : LCD Segment Data display APIs
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
***********************************************************************************************************************/
#include "r_cg_macrodriver.h"       /* macro driver definitions */
#include "r_lcd_display.h"

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Imported global variables and functions (from other files)
***********************************************************************************************************************/

/***********************************************************************************************************************
Exported global variables and functions (to be accessed by other files)
***********************************************************************************************************************/

/***********************************************************************************************************************
Private global variables and functions
***********************************************************************************************************************/

/***********************************************************************************************************************
* Function Name: void LCD_DisplayAll(void)
* Description  : Display all digit on LCD screen
* Arguments    : none
* Output       : none
* Return Value : none
***********************************************************************************************************************/
void LCD_DisplayAll(void)
{
    LCD_RamSetAll();
}

/***********************************************************************************************************************
* Function Name: void LCD_ClearAll(void)
* Description  : Clear all digit on LCD screen
* Arguments    : none
* Output       : none
* Return Value : none
***********************************************************************************************************************/
void LCD_ClearAll(void)
{
    LCD_RamClearAll();
}

/***********************************************************************************************************************
* Function Name: uint8_t uint8_t LCD_DisplayDigit(uint8_t position, uint8_t value)
* Description  : Diplay [value] at [position] on LCD screen
* Arguments    : [value] value need to be display
*              : [position] the position of digit on LCD screen
* Output       : none
* Return Value : LCD_WRONG_INPUT_ARGUMENT: input wrong argument
*              : LCD_INPUT_OK: input OK argument
***********************************************************************************************************************/
uint8_t LCD_DisplayDigit(uint8_t position, uint8_t value)
{
    uint32_t lcdptr;
    uint8_t disp_value, data_status;
    LCD_NUM_INFO num_info;

    if (value >= 0x2d)
    {
        value -= 0x2d;
    }
    else if (value == 0x20)
    {
        value = 15;
    }
    else
    {
        value += 3;
    }
    /* check for range of input argument */
    if ((position > (LCD_NUM_DIGIT)) ||
        (value > (LCD_NUM_DIGIT_CHAR - 1)))
    {
        return LCD_WRONG_INPUT_ARGUMENT;
    }

    LCD_ClearDigit(position);

    /* Get display number information */
    data_status = LCD_GetNumInfo(value, position, &num_info);
    if (data_status == LCD_WRONG_INPUT_ARGUMENT)
    {
        return LCD_WRONG_INPUT_ARGUMENT;
    }

    lcdptr = num_info.addr;
    if (lcdptr == 0)
    {
        return LCD_WRONG_INPUT_ARGUMENT;
    }

    /* Turn on all related-segment of this digit */
    disp_value = num_info.value;
    //LCD_WriteRAMDigitInfo(lcdptr, disp_value);
    if (g_LCD_NumMap[position - 1].type == NUM_TYPE_A)
    {
        LCD_WriteRAMDigitInfo_8_Comm(lcdptr, disp_value);
    }
    else if (g_LCD_NumMap[position - 1].type == NUM_TYPE_AN)
    {
        LCD_WriteRAMDigitInfo_8_Comm_Alfa(lcdptr, num_info.value, num_info.value_2);
    }

    return LCD_INPUT_OK;
}

uint8_t LCD_DisplayDigit_TZ(uint8_t position, uint8_t value)
{
    uint32_t lcdptr;
    uint8_t disp_value, data_status;
    LCD_NUM_INFO num_info;

    if (value >= 0x2d)
    {
        value -= 0x2d;
    }
    else if (value == 0x20)
    {
        value = 15;
    }
    else
    {
        value += 3;
    }
    /* check for range of input argument */
    if ((position > (LCD_NUM_DIGIT_TZ)) ||
        (value > (LCD_NUM_DIGIT_CHAR - 1)))
    {
        return LCD_WRONG_INPUT_ARGUMENT;
    }
        
    /* Get display number information */    
    lcdptr = g_LCD_NumMap_TZ[position - 1].addr;;
    if (lcdptr == 0)
    {
        return LCD_WRONG_INPUT_ARGUMENT;
    }
    disp_value = g_LCD_NumType[value].typeA;
    LCD_WriteRAMDigitInfo_8_Comm_TZ(lcdptr, disp_value);
    return LCD_INPUT_OK;
}

/***********************************************************************************************************************
* Function Name: void LCD_ClearDigit(uint8_t position)
* Description  : Clear digit at specified position
* Arguments    : [position] where the digit need to be cleared
* Output       : none
* Return Value : LCD_WRONG_INPUT_ARGUMENT: input wrong argument
*              : LCD_INPUT_OK: input OK argument
***********************************************************************************************************************/
uint8_t LCD_ClearDigit(uint8_t position)
{
    /* Write 20 to specified position to clear the digit */
    uint32_t lcdptr;
    uint8_t disp_value, data_status;
    LCD_NUM_INFO num_info;

    /* check for range of input argument */
    if (position > (LCD_NUM_DIGIT))
    {
        return LCD_WRONG_INPUT_ARGUMENT;
    }

    /* Get display number information */
    data_status = LCD_GetNumInfo(LCD_CLEAR_DIGIT, position, &num_info);
    if (data_status == LCD_WRONG_INPUT_ARGUMENT)
    {
        return LCD_WRONG_INPUT_ARGUMENT;
    }

    lcdptr = num_info.addr;
    if (lcdptr == 0)
    {
        return LCD_WRONG_INPUT_ARGUMENT;
    }

    /* Turn off all related-segment of this digit */
    disp_value = num_info.value;

    if (g_LCD_NumMap[position - 1].type == NUM_TYPE_A)
    {
        LCD_ClearRAMDigitInfo_8_Comm(lcdptr, disp_value);
    }
    else if (g_LCD_NumMap[position - 1].type == NUM_TYPE_AN)
    {
        LCD_ClearRAMDigitInfo_8_Comm_Alfa(lcdptr, 0x80, 0x80);
    }
    return LCD_INPUT_OK;
}

/***********************************************************************************************************************
* Function Name: static uint8_t LCD_DisplayRawSign(LCD_EM_SPEC_SIGN raw_sign)
* Description  : Display directly each raw_sign on LCD
* Arguments    : [raw_sign] value need to be displayed
* Output       : none
* Return Value : LCD_WRONG_INPUT_ARGUMENT: input wrong argument
*              : LCD_INPUT_OK: input OK argument
***********************************************************************************************************************/
uint8_t LCD_DisplaySpSign(LCD_EM_SPEC_SIGN raw_sign)
{
    uint32_t lcdptr;
    uint8_t value, sign_status;
    LCD_SIGN_INFO sign_info;

    /* check for range of raw_sign: only in (0, 22) */
    if ((raw_sign > (LCD_NUM_SIGN - 1)) || (raw_sign < 0))
    {
        return LCD_WRONG_INPUT_ARGUMENT;
    }

    /* turn on the related raw_sign */
    sign_status = LCD_GetSignInfo(raw_sign, &sign_info);
    if (sign_status != LCD_INPUT_OK)
    {
        return LCD_WRONG_INPUT_ARGUMENT;
    }

    lcdptr = sign_info.addr;
    if (lcdptr == 0)
    {
        return LCD_WRONG_INPUT_ARGUMENT;
    }

    /* Write 0x1 to specified position to light this raw_sign */
    value = (uint8_t)(0x1 << sign_info.pos);
    LCD_WriteRAMSignInfo(lcdptr, value);

    return LCD_INPUT_OK;
}

/***********************************************************************************************************************
* Function Name: static uint8_t LCD_ClearRawSign(LCD_SPECIAL_SIGN raw_sign)
* Description  : Clear directly each raw_sign on LCD
* Arguments    : [raw_sign] value need to be displayed
* Output       : none
* Return Value : LCD_WRONG_INPUT_ARGUMENT: input wrong argument
*              : LCD_INPUT_OK: input OK argument
***********************************************************************************************************************/
uint8_t LCD_ClearSpSign(LCD_EM_SPEC_SIGN raw_sign)
{
    uint32_t lcdptr;
    uint8_t value, sign_status;
    LCD_SIGN_INFO sign_info;

    /* check for range of raw_sign: only in (0, 22) */
    if ((raw_sign > (LCD_NUM_SIGN - 1)) || (raw_sign < 0))
    {
        return LCD_WRONG_INPUT_ARGUMENT;
    }

    /* turn on the related raw_sign */
    sign_status = LCD_GetSignInfo(raw_sign, &sign_info);
    if (sign_status != LCD_INPUT_OK)
    {
        return LCD_WRONG_INPUT_ARGUMENT;
    }

    lcdptr = sign_info.addr;
    if (lcdptr == 0)
    {
        return LCD_WRONG_INPUT_ARGUMENT;
    }

    /* Write 0x0 to specified position to clear this raw_sign */
    value = (uint8_t)(~(0x1 << sign_info.pos));
    LCD_ClearRAMDigitInfo_8_Comm(lcdptr, value);

    return LCD_INPUT_OK;
}