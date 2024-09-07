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
* File Name    : em_display.c
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
#include "r_cg_userdefine.h"
#include "typedef.h"            /* GCSE Standard definitions */
#include "r_cg_wdt.h"
#include "r_cg_rtc.h"
#include "inst_read.h"
#include "r_cg_lcd.h"

/* Wrapper layer */
#include <math.h>
#include "wrp_user_ext.h"

/* Application layer */
#include "platform.h"
//#include "energy.h"
//#include "energy_internal.h"        /* ENERGY module internal APIs and definitions */
#include "r_lcd_display.h"      /* LCD Display Application Layer */
#include "r_lcd_config.h"
#include "em_display.h"         /* EM Display Application Layer */
#include "server.h"
#include "tampers.h"
#include "eeprom_storage.h"

/* Display tamper condition */
//#include "format.h"
//#include "eeprom.h"
#include "em_core.h"

/* DLMS Data */
//#include "r_dlms_cumulate.h"
//#include "r_dlms_ctrl.h"
//#include "r_dlms_meter_mode.h"

#include "key.h"            /* KEY Interface Header File */
//#include "r_dlms_nameplate.h"
#include "RF_Comm.h"
//#include "bl_operation.h"       /* Bootloader module */
#include "SingleWireOp.h"

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/

#define EM_LCD_EXPORT_ENERGY           (0)
#define EM_LCD_IMPORT_ENERGY           (1)

#define EM_LCD_CAPACITIVE_REACTIVE     (0)
#define EM_LCD_INDUCTIVE_REACTIVE      (1)

#define EM_CONFIG_DEFAULT_VOLTAGE_RATING        (float)FACTORY_METER_V_REF
#define EM_CONFIG_DEFAULT_CURRENT_RATING_MAX    (float)FACTORY_METER_I_MAX


/***********************************************************************************************************************
Imported global variables and functions (from other files)
***********************************************************************************************************************/
extern uint32_t g_dLastBill_TamperCount;
//extern EVENT_STATE g_event_state;
extern uint8_t g_byStorageNotFormatted;
//ONE_MONTH_ENERGY_DATA_LOG   Current_bill_log;
uint8_t g_byDisplayMode = 1;
uint8_t byDisplayModePrintCounter = 0;
uint8_t byDisplayModeChangeCounter = 0;
//extern ST_MAGNET_STAT stMagnet; //REVIEW: Not required... Done
extern uint8_t g_u8RTCBatFault;
extern uint8_t g_clock_status;
extern uint8_t g_u8_RSSIVal;
LCD_icon_status e_LCD_icon_status;
static uint8_t byWelcomeMessageCounter = 0;
#ifdef UTILITY_JNK
ONE_MONTH_ENERGY_DATA_LOG   bill_log;
#endif
/***********************************************************************************************************************
Exported global variables and functions (to be accessed by other files)
***********************************************************************************************************************/

/***********************************************************************************************************************
Private global variables and functions
***********************************************************************************************************************/
LCD_FUNC static void EM_DisplayDate(void);
LCD_FUNC static void EM_DisplayUnitCode(void);
LCD_FUNC static void EM_DisplayInstantTime(void);
LCD_FUNC static void EM_LCD_DisplaySoftVersion(void);

LCD_FUNC static void EM_LCD_DisplayMeteringMode(void);
LCD_FUNC static void EM_LCD_DisplayPaymentMode(void);
LCD_FUNC static void EM_LCD_Display_RSSI(void);

LCD_FUNC static void EM_LCD_DisplayInstantVolt(void);
LCD_FUNC static void EM_LCD_DisplayInstantCurrent1(void);
LCD_FUNC static void EM_LCD_DisplayInstantCurrent2(void);

LCD_FUNC static void EM_LCD_DisplayPowerFactor(void);
LCD_FUNC static void EM_LCD_DisplayPowerFactor2(void);
LCD_FUNC static void EM_LCD_DisplayLineFrequency(void);
LCD_FUNC static void EM_LCD_DisplayAvgPowerFactor(void);

LCD_FUNC static void EM_LCD_DisplayInstantActivePower(void);
LCD_FUNC static void EM_LCD_DisplayInstantReactivePower(void);
LCD_FUNC static void EM_LCD_DisplayInstantFundamentalActivePower(void);
LCD_FUNC static void EM_LCD_DisplayInstantApparentPower(void);

LCD_FUNC static void EM_LCD_DisplayInstantActivePower2(void);
LCD_FUNC static void EM_LCD_DisplayInstantReactivePower2(void);
LCD_FUNC static void EM_LCD_DisplayInstantFundamentalActivePower2(void);
LCD_FUNC static void EM_LCD_DisplayInstantApparentPower2(void);

LCD_FUNC static void EM_LCD_DisplayImportActiveEnergy(void);
LCD_FUNC static void EM_LCD_DisplayImportCapacitiveReactiveEnergy(void);
LCD_FUNC static void EM_LCD_DisplayImportInductiveReactiveEnergy(void);
LCD_FUNC static void EM_LCD_DisplayImportApparentEnergy(void);
LCD_FUNC static void EM_LCD_DisplayExportActiveEnergy(void);
LCD_FUNC static void EM_LCD_DisplayExportCapacitiveReactiveEnergy(void);
LCD_FUNC static void EM_LCD_DisplayExportInductiveReactiveEnergy(void);
LCD_FUNC static void EM_LCD_DisplayExportApparentEnergy(void);

LCD_FUNC static void EM_LCD_DisplayImportActiveEnergyTariff(void);
LCD_FUNC static void EM_LCD_DisplayImportCapacitiveReactiveEnergyTariff(void);
LCD_FUNC static void EM_LCD_DisplayImportInductiveReactiveEnergyTariff(void);
LCD_FUNC static void EM_LCD_DisplayImportApparentEnergyTariff(void);
LCD_FUNC static void EM_LCD_DisplayExportActiveEnergyTariff(void);
LCD_FUNC static void EM_LCD_DisplayExportCapacitiveReactiveEnergyTariff(void);
LCD_FUNC static void EM_LCD_DisplayExportInductiveReactiveEnergyTariff(void);
LCD_FUNC static void EM_LCD_DisplayExportApparentEnergyTariff(void);

LCD_FUNC static void EM_LCD_DisplayActMaxDemand(void);
LCD_FUNC static void EM_LCD_DisplayCapacitiveReactiveMaxDemand(void);
LCD_FUNC static void EM_LCD_DisplayInductiveReactiveMaxDemand(void);
LCD_FUNC static void EM_LCD_DisplayAppMaxDemand(void);
#ifdef UTILITY_JNK
LCD_FUNC static void EM_LCD_DisplayCurrentBill_PF(void);
#endif

LCD_FUNC void EM_LCD_DisplayPOR(void);
LCD_FUNC void EM_LCD_DisplayNVMFail(void);

LCD_FUNC static void LCD_DisplayMode(void);
LCD_FUNC static void EM_LCD_DisplayStatus(void);
LCD_FUNC static void EM_LCD_DisplayBillingCount(void);
LCD_FUNC static void EM_LCD_DisplayBill_kWh(void);
LCD_FUNC static void EM_LCD_DisplayBill_kVAh(void);
LCD_FUNC static void EM_LCD_DisplayBill_PF(void);
LCD_FUNC static void EM_LCD_DisplayBill_MDkW(void);
LCD_FUNC static void EM_LCD_DisplayBill_MDkVA(void);
LCD_FUNC static void EM_LCD_Display_MDkW(void);
LCD_FUNC static void EM_LCD_Display_MDkVA(void);
LCD_FUNC static void EM_LCD_Display_Export_MDkW(void);
LCD_FUNC static void EM_LCD_Display_Export_MDkVA(void);
LCD_FUNC static void EM_LCD_DisplayBill_TC(void);
LCD_FUNC static void EM_LCD_DisplayBill_kWh_T1(void);
LCD_FUNC static void EM_LCD_DisplayBill_kWh_T2(void);
LCD_FUNC static void EM_LCD_DisplayBill_kWh_T3(void);
LCD_FUNC static void EM_LCD_Display_kWh_T1(void);
LCD_FUNC static void EM_LCD_Display_kWh_T2(void);
LCD_FUNC static void EM_LCD_Display_kWh_T3(void);
LCD_FUNC static void EM_LCD_Display_kWh_T4(void);
LCD_FUNC static void EM_LCD_Display_kWh_T5(void);
LCD_FUNC static void EM_LCD_Display_kWh_T6(void);
LCD_FUNC static void EM_LCD_Display_kVAh_T1(void);
LCD_FUNC static void EM_LCD_Display_kVAh_T2(void);
LCD_FUNC static void EM_LCD_Display_kVAh_T3(void);
LCD_FUNC static void EM_LCD_Display_kVAh_T4(void);
LCD_FUNC static void EM_LCD_Display_kVAh_T5(void);
LCD_FUNC static void EM_LCD_Display_kVAh_T6(void);
LCD_FUNC static void EM_LCD_DisplayBill_MDkVA_Time(void);
LCD_FUNC static void EM_LCD_DisplayBill_MDkVA_Date(void);
LCD_FUNC static void EM_LCD_DisplayBill_MDkW_Time(void);
LCD_FUNC static void EM_LCD_DisplayBill_MDkW_Date(void);
LCD_FUNC static void EM_LCD_Display_MDkVA_Time(void);
LCD_FUNC static void EM_LCD_Display_MDkVA_Date(void);
LCD_FUNC static void EM_LCD_Display_MDkW_Time(void);
LCD_FUNC static void EM_LCD_Display_MDkW_Date(void);
LCD_FUNC static void EM_LCD_Display_Export_MDkVA_Time(void);
LCD_FUNC static void EM_LCD_Display_Export_MDkVA_Date(void);
LCD_FUNC static void EM_LCD_Display_Export_MDkW_Time(void);
LCD_FUNC static void EM_LCD_Display_Export_MDkW_Date(void);
LCD_FUNC static void EM_LCD_DisplayCumPONHrs(void);
LCD_FUNC static void EM_LCD_DisplayHiRes_kWh(void);
LCD_FUNC static void EM_LCD_DisplayHiRes_kVAh(void);
LCD_FUNC static void EM_LCD_DisplayHiRes_kWh_Export(void);
LCD_FUNC static void EM_LCD_DisplayHiRes_kVAh_Export(void);
LCD_FUNC static void EM_LCD_DisplayHiRes_InstantCurrent1(void);
LCD_FUNC static void EM_LCD_DisplayHiResInstantCurrent2(void);
LCD_FUNC static void EM_LCD_Display_OverVolt(void);
LCD_FUNC static void EM_LCD_Display_LowVolt(void);
LCD_FUNC static void EM_LCD_Display_CurrentRev(void);
LCD_FUNC static void EM_LCD_Display_OverCurrent(void);
LCD_FUNC static void EM_LCD_Display_OverLoad(void);
LCD_FUNC static void EM_LCD_Display_EarthLoad(void);
LCD_FUNC static void EM_LCD_Display_Magnet(void);
LCD_FUNC static void EM_LCD_Display_NeutralDisturb(void);
LCD_FUNC static void EM_LCD_Display_NeutralMiss(void);
LCD_FUNC static void EM_LCD_Display_RFMiss(void);
LCD_FUNC static void EM_LCD_Display_CoverOpen(void);
LCD_FUNC static void EM_LCD_Display_FreqOut(void);
LCD_FUNC static void EM_LCD_Display_LowPF(void);
LCD_FUNC static void EM_LCD_DisplayTamperCount(void);
LCD_FUNC static void EM_LCD_DisplayLastTamperOccur(void);
LCD_FUNC static void EM_LCD_DisplayLastTamperOccurDate(void);
LCD_FUNC static void EM_LCD_DisplayLastTamperOccurTime(void);
LCD_FUNC static void EM_LCD_DisplayLastTamperRestore(void);
LCD_FUNC static void EM_LCD_DisplayLastTamperRestoreDate(void);
LCD_FUNC static void EM_LCD_DisplayLastTamperRestoreTime(void);
LCD_FUNC static void EM_LCD_DisplaySerialNumber(void);
LCD_FUNC static void LCD_DisplayEEPROMCounter(void);


LCD_FUNC static void LCD_DisplayLastTokenRechargeAmount(void);
LCD_FUNC static void LCD_DisplayLastTokenRechargeDate(void);
LCD_FUNC static void LCD_DisplayLastTokenRechargeTime(void);
LCD_FUNC static void LCD_DisplayLastTotalAmount(void);
LCD_FUNC static void LCD_DisplayCurrentBalanceAmount(void);
LCD_FUNC static void LCD_DisplayCurrentBalanceDate(void);
LCD_FUNC static void LCD_DisplayCurrentBalanceTime(void);
#ifdef UTILITY_JNK
LCD_FUNC static void EM_LCD_DisplayBill_kWh_H2(void);
LCD_FUNC static void EM_LCD_DisplayBill_MDkW_H2(void);
LCD_FUNC static void EM_LCD_DisplayBill_MDkW_Time_H2(void);
LCD_FUNC static void EM_LCD_DisplayBill_MDkW_Date_H2(void);
LCD_FUNC static void EM_LCD_DisplayBill_kWh_H3(void);
LCD_FUNC static void EM_LCD_DisplayBill_MDkW_H3(void);
LCD_FUNC static void EM_LCD_DisplayBill_MDkW_Time_H3(void);
LCD_FUNC static void EM_LCD_DisplayBill_MDkW_Date_H3(void);
#endif


LCD_FUNC static void EM_LCD_DisplayTestCount(void);
LCD_FUNC static void EM_LCD_DisplayTestRoutine(void);
LCD_FUNC static void EM_LCD_DisplayTestRoutine1(void);
LCD_FUNC static void EM_LCD_DisplayTestRoutine2(void);

void EM_LCD_Latch_IconStat(void);

uint32_t u32TestCount = 0;
volatile uint16_t   g_em_display_count = 0;
static uint16_t     g_em_display_count_old = 0;
static uint16_t     g_em_display_count_check = LCD_DELAY_TIME;
static uint8_t      g_is_display_changed_next = 1;
static uint8_t      g_DispCounter = 0;  /* first position is voltage */
static uint8_t      g_auto_roll = 1;
uint8_t             g_is_por = 0;

typedef void(*LCD_FUNC fp_display_t)(void);
/* Function pointer for EM Get Tariff Energy */
//typedef uint8_t(*ENE_FUNC fp_tariff_t)(uint8_t tariff, float32_t * energy);
/*
* Display list, scroll over at end
* Append or move the display API inside this list for arrangement
* of LCD display
*/
extern uint32_t        dI2CWriteCounter;

const fp_display_t fp_display[] =
{
    //EM_LCD_DisplayTestRoutine,
    //NULL,
    LCD_DisplayAll,
    EM_LCD_DisplayMeteringMode,
    EM_LCD_DisplayPaymentMode,
    EM_DisplayDate,
    EM_DisplayInstantTime,

    LCD_DisplayLastTokenRechargeAmount,
    LCD_DisplayLastTokenRechargeDate,
    LCD_DisplayLastTokenRechargeTime,
    LCD_DisplayLastTotalAmount,
    LCD_DisplayCurrentBalanceAmount,
    LCD_DisplayCurrentBalanceDate,
    LCD_DisplayCurrentBalanceTime,

    EM_LCD_DisplayImportActiveEnergy,
    EM_LCD_DisplayImportApparentEnergy,
    EM_LCD_Display_MDkW,  //Bill Point kW MD history 1
    EM_LCD_Display_MDkW_Date,   //Bill Point kW MD Date history 1
    EM_LCD_Display_MDkW_Time,   //Bill Point kW MD Time history 1
    EM_LCD_Display_MDkVA,   //Bill Point kVA MD history 1
    EM_LCD_Display_MDkVA_Date,   //Bill Point kVA MD Date history 1
    EM_LCD_Display_MDkVA_Time,   //Bill Point kVA MD Time history 1
    
    EM_LCD_DisplayExportActiveEnergy,
    EM_LCD_DisplayExportApparentEnergy,
    EM_LCD_Display_Export_MDkW,  //Bill Point kW MD history 1
    EM_LCD_Display_Export_MDkW_Date,   //Bill Point kW MD Date history 1
    EM_LCD_Display_Export_MDkW_Time,   //Bill Point kW MD Time history 1
    EM_LCD_Display_Export_MDkVA,   //Bill Point kVA MD history 1
    EM_LCD_Display_Export_MDkVA_Date,   //Bill Point kVA MD Date history 1
    EM_LCD_Display_Export_MDkVA_Time,   //Bill Point kVA MD Time history 1

    EM_LCD_DisplayInstantVolt,
    EM_LCD_DisplayInstantCurrent1,
    EM_LCD_DisplayInstantCurrent2,
    EM_LCD_DisplayInstantActivePower,
    EM_LCD_DisplayAvgPowerFactor,//Bill Point avg PF 
    EM_LCD_DisplayTamperCount,
    EM_LCD_DisplayLastTamperOccur,
    EM_LCD_DisplayLastTamperOccurDate,
    EM_LCD_DisplayLastTamperOccurTime,
    EM_LCD_DisplayLastTamperRestore,
    EM_LCD_DisplayLastTamperRestoreDate,
    EM_LCD_DisplayLastTamperRestoreTime,
    EM_LCD_DisplaySerialNumber,

    NULL,
};

const fp_display_t fp_display2[] =
{
    LCD_DisplayAll,
    //EM_LCD_DisplayTestRoutine1,
    //EM_LCD_DisplayTestRoutine2,
    //NULL,
    EM_LCD_DisplayMeteringMode,
    EM_LCD_DisplayPaymentMode,
    EM_DisplayDate,
    EM_DisplayInstantTime,

    LCD_DisplayLastTokenRechargeAmount,
    LCD_DisplayLastTokenRechargeDate,
    LCD_DisplayLastTokenRechargeTime,
    LCD_DisplayLastTotalAmount,
    LCD_DisplayCurrentBalanceAmount,
    LCD_DisplayCurrentBalanceDate,
    LCD_DisplayCurrentBalanceTime,

    EM_LCD_DisplayImportActiveEnergy,
    EM_LCD_DisplayImportApparentEnergy,
    EM_LCD_Display_MDkW,  //Bill Point kW MD history 1
    EM_LCD_Display_MDkW_Date,   //Bill Point kW MD Date history 1
    EM_LCD_Display_MDkW_Time,   //Bill Point kW MD Time history 1
    EM_LCD_Display_MDkVA,   //Bill Point kVA MD history 1
    EM_LCD_Display_MDkVA_Date,   //Bill Point kVA MD Date history 1
    EM_LCD_Display_MDkVA_Time,   //Bill Point kVA MD Time history 1

    EM_LCD_DisplayExportActiveEnergy,
    EM_LCD_DisplayExportApparentEnergy,
    EM_LCD_Display_Export_MDkW,  //Bill Point kW MD history 1
    EM_LCD_Display_Export_MDkW_Date,   //Bill Point kW MD Date history 1
    EM_LCD_Display_Export_MDkW_Time,   //Bill Point kW MD Time history 1
    EM_LCD_Display_Export_MDkVA,   //Bill Point kVA MD history 1
    EM_LCD_Display_Export_MDkVA_Date,   //Bill Point kVA MD Date history 1
    EM_LCD_Display_Export_MDkVA_Time,   //Bill Point kVA MD Time history 1

    EM_LCD_DisplayInstantVolt,
    EM_LCD_DisplayInstantCurrent1,
    EM_LCD_DisplayInstantCurrent2,
    EM_LCD_DisplayInstantActivePower,
    EM_LCD_DisplayAvgPowerFactor,//Bill Point avg PF 
    EM_LCD_DisplayTamperCount,
    EM_LCD_DisplayLastTamperOccur,
    EM_LCD_DisplayLastTamperOccurDate,
    EM_LCD_DisplayLastTamperOccurTime,
    EM_LCD_DisplayLastTamperRestore,
    EM_LCD_DisplayLastTamperRestoreDate,
    EM_LCD_DisplayLastTamperRestoreTime,
    EM_LCD_DisplaySerialNumber,

    EM_LCD_DisplayImportInductiveReactiveEnergy,
    EM_LCD_DisplayImportCapacitiveReactiveEnergy,

    EM_LCD_Display_kWh_T1,     //Timezone 1 kWh 
    EM_LCD_Display_kWh_T2,     //Timezone 2 kWh 
    EM_LCD_Display_kWh_T3,     //Timezone 3 kWh
    EM_LCD_Display_kWh_T4,     //Timezone 4 kWh 
    EM_LCD_Display_kWh_T5,     //Timezone 5 kWh 
    EM_LCD_Display_kWh_T6,     //Timezone 6 kWh

    EM_LCD_Display_kVAh_T1,     //Timezone 1 kWh 
    EM_LCD_Display_kVAh_T2,     //Timezone 2 kWh 
    EM_LCD_Display_kVAh_T3,     //Timezone 3 kWh
    EM_LCD_Display_kVAh_T4,     //Timezone 4 kWh 
    EM_LCD_Display_kVAh_T5,     //Timezone 5 kWh 
    EM_LCD_Display_kVAh_T6,     //Timezone 6 kWh
    
    EM_LCD_DisplayBill_kWh,     //Bill Point kWh history 1
    EM_LCD_DisplayBill_kVAh,    //Bill Point kVAh history 1
    EM_LCD_DisplayBill_MDkW,    //Bill Point kW MD history 1
    EM_LCD_DisplayBill_MDkW_Date,   //Bill Point kW MD Date history 1
    EM_LCD_DisplayBill_MDkW_Time,   //Bill Point kW MD Time history 1
    EM_LCD_DisplayBill_PF,      //Bill Point avg PF history 1

#ifdef UTILITY_JNK
    EM_LCD_DisplayBill_kWh_H2,
    EM_LCD_DisplayBill_MDkW_H2,
    EM_LCD_DisplayBill_MDkW_Date_H2,
    EM_LCD_DisplayBill_MDkW_Time_H2,

    EM_LCD_DisplayBill_kWh_H3,
    EM_LCD_DisplayBill_MDkW_H3,
    EM_LCD_DisplayBill_MDkW_Date_H3,
    EM_LCD_DisplayBill_MDkW_Time_H3,
#endif

    NULL,
};

const fp_display_t fp_display3[] =
{
    EM_LCD_DisplayHiRes_kWh,
    EM_LCD_DisplayHiRes_kWh_Export,
    EM_LCD_DisplayHiRes_kVAh,
    EM_LCD_DisplayHiRes_kVAh_Export,
    EM_LCD_DisplayHiRes_InstantCurrent1,
    EM_LCD_Display_RSSI,

    NULL,
};


LCD_FUNC static void EM_LCD_DisplayTestCount(void)
{
    LCD_DisplayIntWithPos((int32_t)u32TestCount, LCD_LAST_POS_DIGIT);
}

LCD_FUNC static void EM_LCD_DisplayTestRoutine(void)
{
    static uint8_t u8DTRCnt = 0;
    uint8_t u8DTRIndex;
    if (0 == u8DTRCnt)
    {
        LCD_DisplayAll(); 
        u8DTRCnt++;
    }
    else if (1 == u8DTRCnt)
    {
        LCD_ClearAll();
        u8DTRCnt++;
    }
    else if (2 == u8DTRCnt)
    {
        LCD_ClearAll();
        for (u8DTRIndex = 5; u8DTRIndex < 21; u8DTRIndex += 2)
        {
            LCD_WriteRAMDigitInfo_8_Comm(LCD_RAM_START_ADDRESS + u8DTRIndex, 0xFF);
        }
        u8DTRCnt++;
    }
    else
    {
        LCD_ClearAll();
        for (u8DTRIndex = 6; u8DTRIndex < 21; u8DTRIndex += 2)
        {
            LCD_WriteRAMDigitInfo_8_Comm(LCD_RAM_START_ADDRESS + u8DTRIndex, 0xFF);
        }
        u8DTRCnt = 0;
    }
}

LCD_FUNC static void EM_LCD_DisplayTestRoutine1(void)
{
    uint8_t u8DTRIndex;
    for (u8DTRIndex = 5; u8DTRIndex < 21; u8DTRIndex += 2)
    {
        LCD_WriteRAMDigitInfo_8_Comm(LCD_RAM_START_ADDRESS + u8DTRIndex, 0xFF);
    }
}
LCD_FUNC static void EM_LCD_DisplayTestRoutine2(void)
{
    uint8_t u8DTRIndex;
    for (u8DTRIndex = 6; u8DTRIndex < 21; u8DTRIndex += 2)
    {
        LCD_WriteRAMDigitInfo_8_Comm(LCD_RAM_START_ADDRESS + u8DTRIndex, 0xFF);
    }
}


/***********************************************************************************************************************
* Function Name: void EM_DisplaySequenceReset(void)
* Description  : Display to start from Lamp Test whenever this function is called
* Arguments    :
* Output       :
* Return Value :
***********************************************************************************************************************/
LCD_FUNC void EM_DisplaySequenceReset(void)
{
    g_em_display_count = 0;
    g_byDisplayMode = 1;
    g_DispCounter = 0;
    LCD_DisplayAll();
}

/***********************************************************************************************************************
* Function Name: void EM_DisplaySequence(void)
* Description  :
* Arguments    :
* Output       :
* Return Value :
***********************************************************************************************************************/
LCD_FUNC void EM_DisplaySequence(void)
{
    /* Refresh the LCD if 0.5s changed */
    if ((g_em_display_count != g_em_display_count_old) || (g_is_display_changed_next == 1))
    {
	
        if(byWelcomeMessageCounter < METER_VERSION_TIME)
        {
          byWelcomeMessageCounter++;
          LCD_ClearAll();
          LCD_Print_string(METER_VERSION_NUMBER, strlen(METER_VERSION_NUMBER));
          return;
        }
        if ((KEY_UP == KEY_PRESSED))
        {
            byDisplayModeChangeCounter++;
            if (byDisplayModeChangeCounter > DISPLAY_MODE_ENTER_TIME)    //5 seconds
            {
                if(((*(uint32_t*)&st_tamper.g_tampers_bits.tamper_bits) & (1UL << TAMPER_SINGLE_WIRE_STAT)) && (!IS_MAINS_PRESENT()))
                {
                    keyPressEnableDisplayAndPulses();
                    EM_DisplaySequenceReset();
                }
                if (IS_MAINS_PRESENT())
                {
                    byDisplayModeChangeCounter = 0;
                    byDisplayModePrintCounter = 8;
                    g_em_display_count = 0;
                    g_byDisplayMode++;
                    if (g_byDisplayMode > 3)
                    {
                        g_byDisplayMode = 1;
                    }
                }
            }
        }
        else
        {
            byDisplayModeChangeCounter = 0;
        }

        if (0 != byDisplayModePrintCounter)
        {
            byDisplayModePrintCounter--;
            if (0 == byDisplayModePrintCounter)
            {
                g_DispCounter = 0;
                g_em_display_count = 0;
            }
            else
            {
                LCD_DisplayMode();
            }
        }
        LCD_ClearSpSign(S_RX);
        LCD_ClearSpSign(S_TX);
        if (0 == byDisplayModePrintCounter)
        {
            /* Clear before display new parameter */
            if (g_is_display_changed_next == 1)
            {
                //if ((fp_display2[g_DispCounter] == EM_LCD_Display_kWh_T1) || (fp_display[g_DispCounter] == EM_LCD_Display_MDkW))
                {
                //    R_DLMS_DataLog_Get_CurrentCycleBillingParameters(&Current_bill_log);
                }
                LCD_ClearAll();
                LCD_DisplaySpSign(S_LOGO);
                g_is_display_changed_next = 0;
            }

            /* Display the parameter */
            if ((st_tamper.g_tampers_bits.bits.cover_open != 0) && (0 == Key_Press_Counter))
            {
                if (st_tamper.g_tampers_bits.bits.cover_open != 0)
                {
                    LCD_ClearAll();
                    LCD_Print_string("C-OPEN  ", LCD_NUM_DIGIT);
                }
            }
            else
            {
            switch (g_byDisplayMode)
            {
            case 2:
                if (fp_display2[g_DispCounter] != NULL)
                {
                    (fp_display2[g_DispCounter])();
                }
                break;
            case 3:
                if (fp_display3[g_DispCounter] != NULL)
                {
                    (fp_display3[g_DispCounter])();
                }
                break;
            case 1:
            default:
                if (fp_display[g_DispCounter] != NULL)
                {
                    (fp_display[g_DispCounter])();
                }
                break;
            }
            }

            /* Display status */
            if ((g_DispCounter != 0) && (3 != g_byDisplayMode))
            {
                EM_LCD_DisplayStatus();
            }
        }
        g_em_display_count_old = g_em_display_count;
        R_WDT_Restart();
    }

    if (g_auto_roll)
    {
        if (1 != g_byDisplayMode)
        {
                g_em_display_count_check = LCD_DELAY_TIME_MODE_CHANGE;
        }
        else
            {
                g_em_display_count_check = LCD_DELAY_TIME;
            }
        if (g_em_display_count > g_em_display_count_check)
        {
            //g_DispCounter++;
            //if (fp_display[g_DispCounter] == NULL)
            //{
            //    g_DispCounter = 0;
            //}
            //g_is_display_changed_next = 1;
            //g_em_display_count = 0;
            //Harjeet
            LCD_ChangeNext();
            if (1 != g_byDisplayMode)
            {
                g_DispCounter = 0;

                g_byDisplayMode = 1;
                byDisplayModePrintCounter = 8;
                LCD_DisplayMode();
            }
        }
    }
    R_WDT_Restart();
}


LCD_FUNC void LCD_Print_string(char *data, uint8_t len)
{
    uint8_t pos = 1;
    LCD_DECIMAL_INFO   g_DecInfo;
    uint8_t print_decimal;

    while (len)
    {
        if (*data != '.')
        {
            LCD_DisplayDigit(pos, *data);
            pos++;
        }
        else
        {
            print_decimal = 1;
            switch (LCD_LAST_POS_DIGIT-pos+1)
            {
              case 1:  g_DecInfo = g_DecInfo1; break;
              case 2:  g_DecInfo = g_DecInfo2; break;
              case 3:  g_DecInfo = g_DecInfo3; break;
              case 5:  g_DecInfo = g_DecInfo5; break;
              default: print_decimal = 0; break;
            }
            if(print_decimal)
            {
              LCD_DisplaySpSign(g_DecInfo.sign);
            }
        }
        len--;
        data++;
    }
}

/***********************************************************************************************************************
* Function Name: void LCD_RTC_InterruptCallback(void)
* Description  : This function is called by RTC to increase the counter which
*              : is used to control the displayed time of variable
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
LCD_FUNC void EM_RTC_DisplayInterruptCallback(void)
{
    g_em_display_count++;   /* Counter used for displaying measured value */
}

/***********************************************************************************************************************
* Function Name: void EM_DisplayStatus(EM_STATUS *status)
* Description  : Display tamper status of EM
* Arguments    : none
* Output       : none
* Return Value : none
***********************************************************************************************************************/
LCD_FUNC static void EM_LCD_DisplayStatus(void)
{
    EM_LCD_Toggle_Logo(FALSE);
    LCD_ClearSpSign(S_RX);
    LCD_ClearSpSign(S_TX);
    
    (e_LCD_icon_status.cover_open > 0) || (e_LCD_icon_status.DG_Cover > 0) ? LCD_DisplaySpSign(S_COPN) : LCD_ClearSpSign(S_COPN);
    
    (e_LCD_icon_status.battery > 0) ? LCD_DisplaySpSign(S_BAT) : LCD_ClearSpSign(S_BAT);
    
    (e_LCD_icon_status.clock > 0) ? LCD_DisplaySpSign(S_CLK) : LCD_ClearSpSign(S_CLK);
    
    (e_LCD_icon_status.reverse > 0) ? LCD_DisplaySpSign(S_REV) : LCD_ClearSpSign(S_REV);
#if defined SINGLE_PHASE_METER || defined WHOLE_CURRENT_METER
    EM_LCD_Latch_IconStat();
#endif
    (e_LCD_icon_status.r_phase > 0) ? LCD_DisplaySpSign(S_R) : LCD_ClearSpSign(S_R);
    
    (e_LCD_icon_status.y_phase > 0) ? LCD_DisplaySpSign(S_Y) : LCD_ClearSpSign(S_Y);
    
    (e_LCD_icon_status.b_phase > 0) ? LCD_DisplaySpSign(S_B) : LCD_ClearSpSign(S_B);
    
    ((e_LCD_icon_status.r_phase > 0) || (e_LCD_icon_status.y_phase > 0) || (e_LCD_icon_status.b_phase > 0)) ? LCD_DisplaySpSign(S_MAINS) : LCD_ClearSpSign(S_MAINS);
    
    (e_LCD_icon_status.magnet > 0) ? LCD_DisplaySpSign(S_MAG) : LCD_ClearSpSign(S_MAG);
    
    (e_LCD_icon_status.bypass > 0) ? LCD_DisplaySpSign(S_BYPAS) : LCD_ClearSpSign(S_BYPAS);
    
    (e_LCD_icon_status.earth_loading > 0) ? LCD_DisplaySpSign(S_EARTH) : LCD_ClearSpSign(S_EARTH);
    
    (e_LCD_icon_status.warning > 0) ? LCD_DisplaySpSign(S_WARN) : LCD_ClearSpSign(S_WARN);
    
    (e_LCD_icon_status.single_wire > 0) ? LCD_DisplaySpSign(S_NMIS) : LCD_ClearSpSign(S_NMIS);
    
    (e_LCD_icon_status.prepaid > 0) ? LCD_DisplaySpSign(S_PP) : LCD_ClearSpSign(S_PP);
    
    EM_LCD_Display_Signal_bar();
}

void EM_LCD_Display_Signal_bar(void)
{
  static uint8_t toggle = 1;
  LCD_ClearSpSign(S_SIG1);
  LCD_ClearSpSign(S_SIG2);
  LCD_ClearSpSign(S_SIG3);
  LCD_ClearSpSign(S_SIG4);
  LCD_ClearSpSign(S_SIG5);
  switch(get_RSSI_signal_bar())
  {
    case 0:
      if(is_RF_missing())
      {
        if(toggle)
        {
          toggle = 0;
          //LCD_ClearSpSign(S_RX);
          //LCD_ClearSpSign(S_TX);
          break;
        }
        else
        {
          toggle = 1;
          //LCD_DisplaySpSign(S_RX);
          //LCD_DisplaySpSign(S_TX);
        }
      }
      else
      {
        break;
      }
    case 5:
      LCD_DisplaySpSign(S_SIG5);
    case 4:
      LCD_DisplaySpSign(S_SIG4);
    case 3:
      LCD_DisplaySpSign(S_SIG3);
    case 2:
      LCD_DisplaySpSign(S_SIG2);
    case 1:
      LCD_DisplaySpSign(S_SIG1);
      break;
  }
}

void EM_LCD_Toggle_Logo(uint8_t toggle_val)
{
    static uint8_t toggle = 0, glow = 1;
    if(toggle_val)
    {
    if(toggle)
    {
        LCD_ClearSpSign(S_LOGO);
        toggle = 0;
    }
    else
    {
        LCD_DisplaySpSign(S_LOGO);
        toggle = 1;
    }
        glow = 0;
    }
    else
    {
        if(glow)
        {
            LCD_DisplaySpSign(S_LOGO);
            toggle = 1;
        }
        else
        {
            glow = 1;
        }
    }
}

void EM_LCD_Latch_IconStat(void)
{
    static uint8_t toggle = 0;
    if(e_LCD_icon_status.latch_stat & 0x02)
    {
      toggle ^= 1;
      (toggle > 0) ? LCD_ClearSpSign(S_CONT) : LCD_DisplaySpSign(S_CONT);
      if((e_LCD_icon_status.latch_stat & 0x01) > 0)
      {
        LCD_DisplaySpSign(S_DCONT);
      }
      else
      {
        LCD_ClearSpSign(S_DCONT);
      }
    }
    else
    {
      ((e_LCD_icon_status.latch_stat & 0x01) > 0) ? LCD_DisplaySpSign(S_DCONT) : LCD_DisplaySpSign(S_CONT);
      ((e_LCD_icon_status.latch_stat & 0x01) > 0) ? LCD_ClearSpSign(S_CONT) : LCD_ClearSpSign(S_DCONT);
    }
}
/*  Display instantaneous measured value */
/******************************************************************************
* Function Name: void EM_DisplayUnitCode(void)
* Description  : Display unit code on LCD screen (now, it is fixed RE0001)
* Arguments    : none
* Output       : none
* Return Value : none
******************************************************************************/
LCD_FUNC static void EM_DisplayUnitCode(void)
{
    LCD_DisplayDigit(1, LCD_CHAR_A);
    LCD_DisplayDigit(2, LCD_CHAR_E);
    LCD_DisplayDigit(3, LCD_CHAR_N);
    LCD_DisplayDigit(4, 0);
    LCD_DisplayDigit(5, 0);
    LCD_DisplayDigit(6, 3);
    LCD_DisplayDigit(7, 7);
    LCD_DisplayDigit(8, 2);
    LCD_DisplayDigit(9, 1);
}

/******************************************************************************
* Function Name: void EM_LCD_DisplaySoftVersion(void)
* Description  : Display unit code on LCD screen
* Arguments    : none
* Output       : none
* Return Value : none
******************************************************************************/
LCD_FUNC static void EM_LCD_DisplaySoftVersion(void)
{
    LCD_Print_string((char *)METER_VERSION_NUMBER, strlen(METER_VERSION_NUMBER));

    //LCD_DisplaySpSign(S_D4);
    /*LCD_DisplayDigit(1, 'C');
    LCD_DisplayDigit(2, 'R');
    LCD_DisplayDigit(3, 'Y');
    LCD_DisplayDigit(4, ' ');
    LCD_DisplayDigit(5, ' ');
    LCD_DisplayDigit(6, 0);
    LCD_DisplayDigit(7, 1);
    LCD_DisplayDigit(8, 6);
    LCD_DisplayDigit(9, 3);*/
}

/******************************************************************************
* Function Name: void EM_LCD_DisplayMeteringMode(void)
* Description  : Display Metering Mode On LCD screen FWD or NET
* Arguments    : none
* Output       : none
* Return Value : none
******************************************************************************/
LCD_FUNC static void EM_LCD_DisplayMeteringMode(void)
{
    if (e_Import_Only == get_metering_mode())
    {
         LCD_Print_string("FWD NN/D", sizeof("FWD NN/D") - 1);
    }
    else
    {
         LCD_Print_string("NET NN/D", sizeof("NET NN/D") - 1);
    }
}

/******************************************************************************
* Function Name: void EM_LCD_DisplayPaymentMode(void)
* Description  : Display Payment Mode On LCD screen Prepaid or postpaid
* Arguments    : none
* Output       : none
* Return Value : none
******************************************************************************/
LCD_FUNC static void EM_LCD_DisplayPaymentMode(void)
{
    if (e_Postpaid == Payment_Mode)
    {
            LCD_Print_string("POSTPAID", sizeof("POSTPAID") - 1);
    }
    else
    {
            LCD_Print_string("PREPAID ", sizeof("POSTPAID") - 1);
    }
}

/******************************************************************************
* Function Name: void EM_DisplayInstantTime(void)
* Description  : Display time (hh:mm:ss) and TIME sign on LCD screen
* Arguments    : none
* Output       : none
* Return Value : none
******************************************************************************/
LCD_FUNC static void EM_DisplayInstantTime(void)
{
    rtc_counter_value_t data;
    TIME_DATA_INFO              disp_data;

    /* Get RTC data */
    R_RTC_Get_CalendarCounterValue(&data);

    /* Set time data to display */
    disp_data.hour = data.hour;
    disp_data.min = data.min;
    disp_data.sec = data.sec;
    LCD_DisplayTime(disp_data, 1);

    LCD_DisplayDigit_TZ(1, 'T');
    //LCD_DisplaySpSign(S_TIME);
}

/******************************************************************************
* Function Name: void EM_DisplayDate(void)
* Description  : Display time (dd:MM:yy) and DATE sign on LCD screen
* Arguments    : none
* Output       : none
* Return Value : none
******************************************************************************/
LCD_FUNC static void EM_DisplayDate(void)
{
    rtc_counter_value_t data;
    TIME_DATA_INFO              disp_data;

    /* Get RTC data */
    R_RTC_Get_CalendarCounterValue(&data);

    /* Set time data to display */
    disp_data.hour = data.day;
    disp_data.min = data.month;
    disp_data.sec = (uint8_t)data.year;
    LCD_DisplayTime(disp_data, 1);
    LCD_DisplayDigit_TZ(1, 'D');
    LCD_DisplayDigit_TZ(2, 'T');
    LCD_DisplaySpSign(S_DATE);
}

/*  Display instantaneous measured value */

/******************************************************************************
* Function Name: void LCD_DisplayInstantVolt(void)
* Description  : Display instanatanous measured voltage
* Arguments    : none
* Output       : none
* Return Value : none
******************************************************************************/
LCD_FUNC static void EM_LCD_DisplayInstantVolt(void)
{
    //float32_t volt;
    //
    ///* Modify to get enough length for LCD if necessary */    
    //if ((0 != st_tamper.tampers_bits.bits.neutral_disturbance) || (0 != st_tamper.tampers_bits.bits.single_wire) || (0 != st_tamper.tampers_bits.bits.magnet_detected))          ///MyChanged ACMagnet 20210723
    //{
    //        volt = EM_CONFIG_DEFAULT_VOLTAGE_RATING;
    //}
    //else
    //{
    //        volt = get_ph_voltage();
    //}

    /* Display measured value */
    LCD_DisplayFloat(get_ph_voltage());

    /* Display "V" sign */
    LCD_DisplaySpSign(S_V);
}

/******************************************************************************
* Function Name: void EM_LCD_CommonDisplayInstantCurrent(EM_LINE line)
* Description  : Common display for current input
* Arguments    : none
* Output       : none
* Return Value : none
******************************************************************************/
LCD_FUNC static void EM_LCD_CommonDisplayInstantCurrentHighRes(EM_LINE line)
{
    float32_t current;

    /* Modify to get enough length for LCD if necessary */
    if (line == EM_LINE_PHASE)
    {
        current = get_signed_ph_current();
    }
    else
    {
        current = get_signed_neu_current();
    }
    //if((0 != st_tamper.tampers_bits.bits.magnet_detected))          ///MyChanged ACMagnet 20210723
    //{
    //        current = EM_CONFIG_DEFAULT_CURRENT_RATING_MAX;
    //}

    /* Display measured value */
    LCD_DisplayFloat(current);

    /* Display "A" sign */
    LCD_DisplaySpSign(S_FWSL);
    LCD_DisplaySpSign(S_A);

    /* Display A at digit 8 */
    //LCD_DisplayDigit(8, LCD_CHAR_A);
}

LCD_FUNC static void EM_LCD_CommonDisplayInstantCurrent(EM_LINE line)
{
    float32_t current;
    int32_t icurrent;

    /* Modify to get enough length for LCD if necessary */
    if (line == EM_LINE_PHASE)
    {
        current = get_signed_ph_current();
    }
    else
    {
        current = get_signed_neu_current();
    }
    //if ((0 != st_tamper.tampers_bits.bits.magnet_detected))          ///MyChanged ACMagnet 20210723
    //{
    //    current = EM_CONFIG_DEFAULT_CURRENT_RATING_MAX;
    //}

    /* Display measured value */
//    LCD_DisplayFloat(current);
    icurrent = current * 10;
    LCD_DisplayIntWithPos(icurrent, LCD_LAST_POS_DIGIT);
    if (WRP_EXT_Absf(current) < 1.0)
    {
        LCD_DisplayDigit(7, 0);
        if (current < 0)
        {
            LCD_DisplayDigit(6, LCD_MINUS_SIGN);
        }
    }
    LCD_DisplaySpSign(S_D5);
    /* Display "A" sign */
    LCD_DisplaySpSign(S_FWSL);
    LCD_DisplaySpSign(S_A);

    /* Display A at digit 8 */
    //LCD_DisplayDigit(8, LCD_CHAR_A);
}

/******************************************************************************
* Function Name: void EM_LCD_DisplayInstantCurrent1(void)
* Description  : Display instantanous measured phase current
* Arguments    : none
* Output       : none
* Return Value : none
******************************************************************************/
LCD_FUNC static void EM_LCD_DisplayInstantCurrent1(void)
{
    EM_LCD_CommonDisplayInstantCurrent(EM_LINE_PHASE);

    /* Display 1 at digit 9 */
    LCD_DisplayDigit(1, 'P');
    LCD_DisplayDigit(2, 'H');
}

/******************************************************************************
* Function Name: void EM_LCD_DisplayInstantCurrent2(void)
* Description  : Display instantanous measured neutral current
* Arguments    : none
* Output       : none
* Return Value : none
******************************************************************************/
LCD_FUNC static void EM_LCD_DisplayInstantCurrent2(void)
{
    EM_LCD_CommonDisplayInstantCurrent(EM_LINE_NEUTRAL);

    /* Display 1 at digit 9 */
    LCD_DisplayDigit(1, 'N');
    LCD_DisplayDigit(2, 'E');
    LCD_DisplayDigit(3, 'U');
}

LCD_FUNC static void EM_LCD_DisplayHiRes_InstantCurrent1(void)
{
    EM_LCD_CommonDisplayInstantCurrentHighRes(EM_LINE_PHASE);

    /* Display 1 at digit 9 */
    LCD_DisplayDigit(1, 'P');
    LCD_DisplayDigit(2, 'H');
}


/******************************************************************************
* Function Name: void EM_LCD_CommonDisplayInstantCurrent(EM_LINE line)
* Description  : Common display for current input
* Arguments    : none
* Output       : none
* Return Value : none
******************************************************************************/
LCD_FUNC static void EM_LCD_CommonDisplayInstantActivePower(EM_LINE line)
{
//    float32_t power;
//    float32_t current;

//    /* Modify to get enough length for LCD if necessary */
//    if (line == EM_LINE_PHASE)
//    {
//        power = g_inst_read_params.active_power;
//    }
//    else
//    {
//        power = g_inst_read_params.active_power2;
//    }

//    //if (IS_NMISS_ACTIVE())
//    if ((st_tamper.tampers_bits.bits.neutral_disturbance) || (0 != st_tamper.tampers_bits.bits.single_wire) || (0 != st_tamper.tampers_bits.bits.magnet_detected))          ///MyChanged ACMagnet 20210723
//    {
//        if (line == EM_LINE_PHASE)
//        {
//            power = g_inst_read_params.irms_phase * EM_CONFIG_DEFAULT_VOLTAGE_RATING;
//        }
//        else
//        {
//            power = g_inst_read_params.irms_neutral * EM_CONFIG_DEFAULT_VOLTAGE_RATING;
//        }
//        if (0 != st_tamper.tampers_bits.bits.magnet_detected)          ///MyChanged ACMagnet 20210723
//        {
//            power = EM_CONFIG_DEFAULT_CURRENT_RATING_MAX * EM_CONFIG_DEFAULT_VOLTAGE_RATING;
//        }
//    }
//    power = power / 1000.0;

    /* Display measured value */
    LCD_DisplayFloat((get_signed_tot_kw()/1000.0));

    /* Display "kW" sign */
    LCD_DisplaySpSign(S_k);
    LCD_DisplaySpSign(S_V);
    LCD_DisplaySpSign(S_BWSL);
    LCD_DisplaySpSign(S_FWSL);
}

/******************************************************************************
* Function Name: void EM_LCD_DisplayInstantActivePower(void)
* Description  : Display instantanous measured active power
* Arguments    : none
* Output       : none
* Return Value : none
******************************************************************************/
LCD_FUNC static void EM_LCD_DisplayInstantActivePower(void)
{
    /* Display measured value */
//    if (EM_LINE_PHASE == EM_GetRMSLine())
    {
    EM_LCD_CommonDisplayInstantActivePower(EM_LINE_PHASE);
    }
//    else
//    {
//            EM_LCD_CommonDisplayInstantActivePower(EM_LINE_NEUTRAL);
//    }

    /* Display 1 at digit 9 */
    //LCD_DisplayDigit(9, 1);
//    LCD_DisplayDigit(1, 'P');
//    LCD_DisplayDigit(2, 'H');
}

/******************************************************************************
* Function Name: void EM_LCD_DisplayInstantActivePower2(void)
* Description  : Display instantanous measured active power 2
* Arguments    : none
* Output       : none
* Return Value : none
******************************************************************************/
LCD_FUNC static void EM_LCD_DisplayInstantActivePower2(void)
{
    /* Display measured value */
    EM_LCD_CommonDisplayInstantActivePower(EM_LINE_NEUTRAL);

    /* Display 2 at digit 9 */
    LCD_DisplayDigit(1, 'N');
    LCD_DisplayDigit(2, 'E');
    LCD_DisplayDigit(3, 'U');
}

/******************************************************************************
* Function Name: void EM_LCD_CommonDisplayInstantReactivePower(EM_LINE line)
* Description  : Common display for current input
* Arguments    : none
* Output       : none
* Return Value : none
******************************************************************************/
LCD_FUNC static void EM_LCD_CommonDisplayInstantReactivePower(EM_LINE line)
{
//    float32_t power;

//    /* Modify to get enough length for LCD if necessary */
//    if (line == EM_LINE_PHASE)
//    {
//        power = g_inst_read_params.reactive_power;
//    }
//    else
//    {
//        power = g_inst_read_params.reactive_power2;
//    }
//    power = power / 1000.0;

//    if ((st_tamper.tampers_bits.bits.neutral_disturbance) || (0 != st_tamper.tampers_bits.bits.single_wire) || (0 != st_tamper.tampers_bits.bits.magnet_detected))          ///MyChanged ACMagnet 20210723
//    {
//            power = 0;
//    }

    /* Display measured value */
    LCD_DisplayFloat((get_signed_tot_kvar()/1000.0));

    /* Display "VAr" sign */
    LCD_DisplaySpSign(S_V);
    LCD_DisplaySpSign(S_FWSL);
    LCD_DisplaySpSign(S_A);
    LCD_DisplaySpSign(S_r);
}

/******************************************************************************
* Function Name: void EM_LCD_DisplayInstantReactivePower(void)
* Description  : Display instantanous measured reactive power
* Arguments    : none
* Output       : none
* Return Value : none
******************************************************************************/
LCD_FUNC static void EM_LCD_DisplayInstantReactivePower(void)
{
    EM_LCD_CommonDisplayInstantReactivePower(EM_LINE_PHASE);

    /* Display 1 at digit 9 */

    LCD_DisplayDigit(1, 'P');
    LCD_DisplayDigit(2, 'H');
}

/******************************************************************************
* Function Name: void EM_LCD_DisplayInstantReactivePower2(void)
* Description  : Display instantanous measured reactive power 2
* Arguments    : none
* Output       : none
* Return Value : none
******************************************************************************/
LCD_FUNC static void EM_LCD_DisplayInstantReactivePower2(void)
{
    EM_LCD_CommonDisplayInstantReactivePower(EM_LINE_NEUTRAL);

    /* Display 2 at digit 9 */
    LCD_DisplayDigit(1, 'N');
    LCD_DisplayDigit(2, 'E');
    LCD_DisplayDigit(3, 'U');
}

///******************************************************************************
//* Function Name: void EM_LCD_CommonDisplayInstantFundamentalPower(EM_LINE line)
//* Description  : Common display for current input
//* Arguments    : none
//* Output       : none
//* Return Value : none
//******************************************************************************/
//LCD_FUNC static void EM_LCD_CommonDisplayInstantFundamentalPower(EM_LINE line)
//{
//    float32_t power;
//
//    /* Modify to get enough length for LCD if necessary */
//    power = EM_GetFundamentalActivePower(line);
//
//    /* Display measured value */
//    LCD_DisplayFloat(power);
//
//    /* Display "W" sign */
//    LCD_DisplaySpSign(S_V);
//    LCD_DisplaySpSign(S_BWSL);
//    LCD_DisplaySpSign(S_FWSL);
//
//    /* Display F0 at digit 7 and 8 */
//    LCD_DisplayDigit(1, LCD_CHAR_F);
//    LCD_DisplayDigit(2, '-');
//}
//
///******************************************************************************
//* Function Name: void EM_LCD_DisplayInstantFundamentalActivePower(void)
//* Description  : Display instantanous measured fundamental active power
//* Arguments    : none
//* Output       : none
//* Return Value : none
//******************************************************************************/
//LCD_FUNC static void EM_LCD_DisplayInstantFundamentalActivePower(void)
//{
//    EM_LCD_CommonDisplayInstantFundamentalPower(EM_LINE_PHASE);
//
//    /* Display 1 at digit 9 */
//    LCD_DisplayDigit(3, 'P');
//}
//
///******************************************************************************
//* Function Name: void EM_LCD_DisplayInstantFundamentalActivePower2(void)
//* Description  : Display instantanous measured fundamental active power 2
//* Arguments    : none
//* Output       : none
//* Return Value : none
//******************************************************************************/
//LCD_FUNC static void EM_LCD_DisplayInstantFundamentalActivePower2(void)
//{
//    EM_LCD_CommonDisplayInstantFundamentalPower(EM_LINE_NEUTRAL);
//
//    /* Display 2 at digit 9 */
//    LCD_DisplayDigit(3, 'N');
//}
//
/******************************************************************************
* Function Name: void EM_LCD_CommonDisplayInstantApparentPower(EM_LINE line)
* Description  : Common display for current input
* Arguments    : none
* Output       : none
* Return Value : none
******************************************************************************/
LCD_FUNC static void EM_LCD_CommonDisplayInstantApparentPower(EM_LINE line)
{
//    float32_t power;

//    /* Modify to get enough length for LCD if necessary */
//    if (line == EM_LINE_PHASE)
//    {
//        power = g_inst_read_params.apparent_power;
//    }
//    else
//    {
//        power = g_inst_read_params.apparent_power2;
//    }

//    //if (IS_NMISS_ACTIVE())
//    if ((st_tamper.tampers_bits.bits.neutral_disturbance) || (0 != st_tamper.tampers_bits.bits.single_wire) || (0 != st_tamper.tampers_bits.bits.magnet_detected))          ///MyChanged ACMagnet 20210723
//    {
//        if (line == EM_LINE_PHASE)
//        {
//            power = g_inst_read_params.irms_phase * EM_CONFIG_DEFAULT_VOLTAGE_RATING;
//        }
//        else
//        {
//            power = g_inst_read_params.irms_neutral * EM_CONFIG_DEFAULT_VOLTAGE_RATING;
//        }
//        if (0 != st_tamper.tampers_bits.bits.magnet_detected)          ///MyChanged ACMagnet 20210723
//        {
//            power = EM_CONFIG_DEFAULT_CURRENT_RATING_MAX * EM_CONFIG_DEFAULT_VOLTAGE_RATING;
//        }
//    }
//    power = power / 1000.0;

    /* Display measured value */
    LCD_DisplayFloat((get_tot_kva()/1000.0));

    /* Display "kVA" sign */
    LCD_DisplaySpSign(S_k);
    LCD_DisplaySpSign(S_V);
    LCD_DisplaySpSign(S_FWSL);
    LCD_DisplaySpSign(S_A);
}

/******************************************************************************
* Function Name: void EM_LCD_DisplayInstantApparentPower(void)
* Description  : Display instantanous measured apparent power
* Arguments    : none
* Output       : none
* Return Value : none
******************************************************************************/
LCD_FUNC static void EM_LCD_DisplayInstantApparentPower(void)
{
    EM_LCD_CommonDisplayInstantApparentPower(EM_LINE_PHASE);

    /* Display 1 at digit 9 */
    //LCD_DisplayDigit(1, 'P');
    //LCD_DisplayDigit(2, 'H');
}

/******************************************************************************
* Function Name: void EM_LCD_DisplayInstantApparentPower2(void)
* Description  : Display instantanous measured apparent power 2
* Arguments    : none
* Output       : none
* Return Value : none
******************************************************************************/
LCD_FUNC static void EM_LCD_DisplayInstantApparentPower2(void)
{
    EM_LCD_CommonDisplayInstantApparentPower(EM_LINE_NEUTRAL);

    /* Display 2 at digit 9 */
    LCD_DisplayDigit(1, 'N');
    LCD_DisplayDigit(2, 'E');
    LCD_DisplayDigit(3, 'U');
}

/******************************************************************************
* Function Name: void EM_LCD_DisplayLineFrequency(void)
* Description  : Display operation line frequency of EM
* Arguments    : none
* Output       : none
* Return Value : none
******************************************************************************/
LCD_FUNC static void EM_LCD_DisplayLineFrequency(void)
{
    float32_t line_freq;
    /* Modify to get enough length for LCD if necessary */
    line_freq = EM_GetLineFrequency();

    /* Display measured value */
    LCD_DisplayFloat(line_freq);

    /* Display "h" sign */
    LCD_DisplayDigit(1, 'H');
    LCD_DisplayDigit(2, 'Z');
}

/******************************************************************************
* Function Name: void EM_LCD_CommonDisplayPowerFactor(EM_LINE line)
* Description  : Common display for power factor
* Arguments    : none
* Output       : none
* Return Value : none
******************************************************************************/
LCD_FUNC static void EM_LCD_CommonDisplayPowerFactor(EM_LINE line)
{
    float32_t   pf;
    EM_PF_SIGN  pf_sign;

    /* Modify to get enough length for LCD if necessary */
    if (line == EM_LINE_PHASE)
    {
        pf = g_inst_read_params.power_factor;
        //pf_sign = g_inst_read_params.power_factor_sign;
    }
    else
    {
        pf = g_inst_read_params.power_factor2;
        //pf_sign = g_inst_read_params.power_factor_sign2;
    }
    
    

    //if (IS_NMISS_ACTIVE())
//{
    //    pf = 1.0;
    //    pf_sign = PF_SIGN_UNITY;
//}

    /* Display measured value */
    LCD_DisplayFloat(pf);

//    LCD_DisplayDigit(1, 'P');
    //LCD_DisplayDigit(2, 'F');
    //LCD_DisplayDigit(3, '-');

    //if (pf_sign == PF_SIGN_LEAD_C)
    {
    //    LCD_DisplayDigit(6, '-');
    }
    //else if (pf_sign == PF_SIGN_LAG_L)
//{
    //    LCD_DisplayDigit(6, LCD_CHAR_L);
//}
    //else if (pf_sign == PF_SIGN_UNITY)
//    {
    //    LCD_ClearDigit(6);
    //    //LCD_ClearDigit(4);
//    }
}

/******************************************************************************
* Function Name: void EM_LCD_DisplayPowerFactor(void)
* Description  : Display power factor of input signal
* Arguments    : none
* Output       : none
* Return Value : none
******************************************************************************/
LCD_FUNC static void EM_LCD_DisplayPowerFactor(void)
{
    EM_LCD_CommonDisplayPowerFactor(EM_LINE_PHASE);

    /* Display 1 at digit 9 */
    LCD_DisplayDigit(1, 'P');
    LCD_DisplayDigit(2, '-');
    LCD_DisplayDigit(3, 'P');
    LCD_DisplayDigit(4, 'F');
}

/******************************************************************************
* Function Name: void EM_LCD_DisplayPowerFactor(void)
* Description  : Display power factor of input signal
* Arguments    : none
* Output       : none
* Return Value : none
******************************************************************************/
LCD_FUNC static void EM_LCD_DisplayAvgPowerFactor(void)
{
    LCD_DisplayFloat(get_signed_pf());

    /* Display 2 at digit 9 */
    LCD_DisplayDigit(1, 'A');
    LCD_DisplayDigit(2, 'V');
    LCD_DisplayDigit(3, 'P');
    LCD_DisplayDigit(4, 'F');
}

/******************************************************************************
* Function Name: void EM_LCD_DisplayImportExportStatus(uint8_t is_import)
* Description  : Display import or export status on LCD
* Arguments    : uint8_t is_import: energy direction
* Output       : none
* Return Value : none
******************************************************************************/
LCD_FUNC static void EM_LCD_DisplayImportExportStatus(uint8_t is_import)
{
    if (is_import != EM_LCD_IMPORT_ENERGY)
    {
        /* Display "import" sign "1" on digit 9 */
        LCD_DisplayDigit_TZ(1, 'E');
    }
    else
    {
        /* Display "import" sign "1" on digit 9 */
        LCD_DisplayDigit_TZ(1, 'I');
    }
}

/******************************************************************************
* Function Name: void EM_LCD_DisplayInputActiveEnergy(float32_t value, uint8_t is_import)
* Description  : Display input reactive energy on LCD with energy direction option
* Arguments    : float32_t value: energy value
*              : uint8_t is_import: energy direction
* Output       : none
* Return Value : none
******************************************************************************/
LCD_FUNC static void EM_LCD_DisplayInputActiveEnergy(float32_t value, uint8_t is_import)
{
    LCD_DisplayFloat(value);

    /* Display "kWh" sign */
    LCD_DisplaySpSign(S_k);
    LCD_DisplaySpSign(S_V);
    LCD_DisplaySpSign(S_BWSL);
    LCD_DisplaySpSign(S_FWSL);
    LCD_DisplaySpSign(S_h);

    /* Display import & export status */
    EM_LCD_DisplayImportExportStatus(is_import);
}

/******************************************************************************
* Function Name: void EM_LCD_DisplayInputApparentEnergy(float32_t value, uint8_t is_import, uint8_t is_inductive)
* Description  : Display input reactive energy on LCD with energy direction and reactive type option
* Arguments    : float32_t value: energy value
*              : uint8_t is_import: energy direction
*              : uint8_t is_inductive: reactive type
* Output       : none
* Return Value : none
******************************************************************************/
LCD_FUNC static void EM_LCD_DisplayInputReactiveEnergy(float32_t value, uint8_t is_import, uint8_t is_inductive)
{
    LCD_DisplayFloat(value);

    /* Display "kVArh" sign */
    LCD_DisplaySpSign(S_k);
    LCD_DisplaySpSign(S_V);
    LCD_DisplaySpSign(S_FWSL);
    LCD_DisplaySpSign(S_A);
    LCD_DisplaySpSign(S_r);
    LCD_DisplaySpSign(S_h);

    if (is_inductive == EM_LCD_INDUCTIVE_REACTIVE)
    {
        /* Display "L" as Lag at digit 1 */
        LCD_DisplayDigit(1, LCD_CHAR_L);
    }
    else
    {
        /* Display "C" as Lead at digit 1 */
        LCD_DisplayDigit(1, LCD_CHAR_C);
    }

    /* Display import & export status */
    EM_LCD_DisplayImportExportStatus(is_import);
}

/******************************************************************************
* Function Name: void EM_LCD_DisplayInputApparentEnergy(float32_t value)
* Description  : Display input apparent energy on LCD
* Arguments    : float32_t value: apparent energy value
* Output       : none
* Return Value : none
******************************************************************************/
LCD_FUNC static void EM_LCD_DisplayInputApparentEnergy(float32_t value, uint8_t is_import)
{
    LCD_DisplayFloat(value);

    /* Display "kVAh" sign */
    LCD_DisplaySpSign(S_k);
    LCD_DisplaySpSign(S_V);
    LCD_DisplaySpSign(S_FWSL);
    LCD_DisplaySpSign(S_A);
    LCD_DisplaySpSign(S_h);

    /* Display import & export status */
    EM_LCD_DisplayImportExportStatus(is_import);
}

///******************************************************************************
//* Function Name: void EM_LCD_DisplayTariffNumber(uint8_t tariff_no)
//* Description  : Display tariff number on LCD
//* Arguments    : uint8_t tariff_no:
//* Output       : none
//* Return Value : none
//******************************************************************************/
//LCD_FUNC static void EM_LCD_DisplayTariffNumber(uint8_t tariff_no)
//{
//    /* Display tariff number at digit 8 */
//    LCD_DisplayDigit(8, tariff_no);
//}

///******************************************************************************
//* Function Name: void EM_LCD_DisplayInputActiveEnergyTariff(fp_tariff_t fp_tariff, uint8_t is_import)
//* Description  : Call to get the tariff reactive energy with option of energy direction
//* Arguments    : fp_tariff_t fp_tariff: function pointer to tariff function on EM
//*              : uint8_t is_import: energy direction
//* Output       : none
//* Return Value : none
//******************************************************************************/
//LCD_FUNC static void EM_LCD_DisplayInputActiveEnergyTariff(fp_tariff_t fp_tariff, uint8_t is_import)
//{
//    float   act_energy;
//    uint8_t current_tariff;
//    uint8_t return_status;
//
//    /* Get the current active active tariff */
//    current_tariff = ENERGY_TARIFF_GetCurrentTariff();
//
//    /* Get active energy at active tariff */
//    return_status = fp_tariff(current_tariff, &act_energy);
//
//    if (return_status == EM_OK)
//    {
//        EM_LCD_DisplayInputActiveEnergy(act_energy, is_import);
//
//        EM_LCD_DisplayTariffNumber(current_tariff);
//    }
//}
//
///******************************************************************************
//* Function Name: void EM_LCD_DisplayInputReactiveEnergyTariff(fp_tariff_t fp_tariff, uint8_t is_import, uint8_t is_inductive)
//* Description  : Call to get the tariff reactive energy with option of energy direction and reactive type
//* Arguments    : fp_tariff_t fp_tariff: function pointer to tariff function on EM
//*              : uint8_t is_import: energy direction
//*              : uint8_t is_inductive: reactive type
//* Output       : none
//* Return Value : none
//******************************************************************************/
//LCD_FUNC static void EM_LCD_DisplayInputReactiveEnergyTariff(fp_tariff_t fp_tariff, uint8_t is_import, uint8_t is_inductive)
//{
//    float   react_energy;
//    uint8_t current_tariff;
//    uint8_t return_status;
//
//    /* Get the current active active tariff */
//    current_tariff = ENERGY_TARIFF_GetCurrentTariff();
//
//    /* Get active energy at active tariff */
//    return_status = fp_tariff(current_tariff, &react_energy);
//
//    if (return_status == EM_OK)
//    {
//        EM_LCD_DisplayInputReactiveEnergy(react_energy, is_import, is_inductive);
//
//        EM_LCD_DisplayTariffNumber(current_tariff);
//    }
//}
//
/******************************************************************************
* Function Name: void EM_LCD_DisplayInputReactiveMaxDemand(uint32_t value, uint8_t is_inductive)
* Description  : Call to display max demand unit and reactive type sign
* Arguments    : uint32_t value: max demand value (integer number)
*              : uint8_t is_inductive: reactive type
* Output       : none
* Return Value : none
******************************************************************************/
LCD_FUNC static void EM_LCD_DisplayInputReactiveMaxDemand(float32_t value, uint8_t is_inductive)
{
    float   react_energy;
    uint8_t current_tariff;
    uint8_t return_status;

    return_status = LCD_DisplayFloat(value);
    if (return_status == LCD_WRONG_INPUT_ARGUMENT)
    {
        /* TODO: notice input value is out of range */
    }

    /* Display "kVar" sign */
    LCD_DisplaySpSign(S_k);
    LCD_DisplaySpSign(S_V);
    LCD_DisplaySpSign(S_FWSL);
    LCD_DisplaySpSign(S_A);
    LCD_DisplaySpSign(S_r);

    /* Display "MD" sign on LCD screen */
    LCD_DisplaySpSign(S_MD);

    if (is_inductive == EM_LCD_INDUCTIVE_REACTIVE)
    {
        /* Display "L" as Lag at digit 1 */
        LCD_DisplayDigit(1, LCD_CHAR_L);
    }
    else
    {
        /* Display "C" as Lead at digit 1 */
        LCD_DisplayDigit(1, LCD_CHAR_C);
    }
}

/*
* LCD Energy display function
*/
/******************************************************************************
* Function Name: void EM_LCD_DisplayImportActiveEnergy(void)
* Description  : Display import active enegy (sum of all tariff) of EM
* Arguments    : none
* Output       : none
* Return Value : none
******************************************************************************/
LCD_FUNC static void EM_LCD_DisplayImportActiveEnergy(void)
{
    /* Modify to get enough length for LCD if necessary */
    /* TBD */

    /* Display measured value */
    EM_LCD_DisplayInputActiveEnergy(get_energy(Active_Imp)/100000.0, EM_LCD_IMPORT_ENERGY);
}

/******************************************************************************
* Function Name: void EM_LCD_DisplayImportCapacitiveReactiveEnergy(void)
* Description  : Display import capacitive reactive enegy (sum of all tariff) of EM
* Arguments    : none
* Output       : none
* Return Value : none
******************************************************************************/
LCD_FUNC static void EM_LCD_DisplayImportCapacitiveReactiveEnergy(void)
{
    /* Modify to get enough length for LCD if necessary */
    /* TBD */

    /* Display measured value */
    EM_LCD_DisplayInputReactiveEnergy(get_energy(Reactive_Cap_Imp) / 100000.0, EM_LCD_IMPORT_ENERGY, EM_LCD_CAPACITIVE_REACTIVE);
}

/******************************************************************************
* Function Name: void EM_LCD_DisplayImportInductiveReactiveEnergy(void)
* Description  : Display import inductive reactive enegy (sum of all tariff) of EM
* Arguments    : none
* Output       : none
* Return Value : none
******************************************************************************/
LCD_FUNC static void EM_LCD_DisplayImportInductiveReactiveEnergy(void)
{
    /* Modify to get enough length for LCD if necessary */
    /* TBD */

    /* Display measured value */
    EM_LCD_DisplayInputReactiveEnergy(get_energy(Reactive_Ind_Imp) / 100000.0, EM_LCD_IMPORT_ENERGY, EM_LCD_INDUCTIVE_REACTIVE);
}

/******************************************************************************
* Function Name: void EM_LCD_DisplayImportApparentEnergy(void)
* Description  : Display apparent enegy (sum of all tariff) of EM
* Arguments    : none
* Output       : none
* Return Value : none
******************************************************************************/
LCD_FUNC static void EM_LCD_DisplayImportApparentEnergy(void)
{
    /* Modify to get enough length for LCD if necessary */
    /* TBD */

    /* Display measured value */
    EM_LCD_DisplayInputApparentEnergy(get_energy(Apparent_Imp) / 100000.0, EM_LCD_IMPORT_ENERGY);
}

/******************************************************************************
* Function Name: void EM_LCD_DisplayExportActiveEnergy(void)
* Description  : Display export active enegy (sum of all tariff) of EM
* Arguments    : none
* Output       : none
* Return Value : none
******************************************************************************/
LCD_FUNC static void EM_LCD_DisplayExportActiveEnergy(void)
{
    /* Modify to get enough length for LCD if necessary */
    /* TBD */

    /* Display measured value */
    EM_LCD_DisplayInputActiveEnergy(get_energy(Active_Exp) / 100000.0, EM_LCD_EXPORT_ENERGY);
}

///******************************************************************************
//* Function Name: void EM_LCD_DisplayExportCapacitiveReactiveEnergy(void)
//* Description  : Display export capacitive reactive enegy (sum of all tariff) of EM
//* Arguments    : none
//* Output       : none
//* Return Value : none
//******************************************************************************/
//LCD_FUNC static void EM_LCD_DisplayExportCapacitiveReactiveEnergy(void)
//{
//    /* Modify to get enough length for LCD if necessary */
//    /* TBD */
//
//    /* Display measured value */
//    EM_LCD_DisplayInputReactiveEnergy(ENERGY_GetExportCapacitiveReactiveEnergyTotal(), EM_LCD_EXPORT_ENERGY, EM_LCD_CAPACITIVE_REACTIVE);
//}
//
///******************************************************************************
//* Function Name: void EM_LCD_DisplayExportInductiveReactiveEnergy(void)
//* Description  : Display export inductive reactive enegy (sum of all tariff) of EM
//* Arguments    : none
//* Output       : none
//* Return Value : none
//******************************************************************************/
//LCD_FUNC static void EM_LCD_DisplayExportInductiveReactiveEnergy(void)
//{
//    /* Modify to get enough length for LCD if necessary */
//    /* TBD */
//
//    /* Display measured value */
//    EM_LCD_DisplayInputReactiveEnergy(ENERGY_GetExportInductiveReactiveEnergyTotal(), EM_LCD_EXPORT_ENERGY, EM_LCD_INDUCTIVE_REACTIVE);
//}
//
/******************************************************************************
* Function Name: void EM_LCD_DisplayExportApparentEnergy(void)
* Description  : Display apparent enegy (sum of all tariff) of EM
* Arguments    : none
* Output       : none
* Return Value : none
******************************************************************************/
LCD_FUNC static void EM_LCD_DisplayExportApparentEnergy(void)
{
    /* Modify to get enough length for LCD if necessary */
    /* TBD */

    /* Display measured value */
    EM_LCD_DisplayInputApparentEnergy(get_energy(Apparent_Exp) / 100000.0, EM_LCD_EXPORT_ENERGY);
}

///******************************************************************************
//* Function Name: void EM_LCD_DisplayImportActiveEnergyTariff(void)
//* Description  : Display import active energy follow tariff
//* Arguments    : none
//* Output       : none
//* Return Value : none
//******************************************************************************/
//LCD_FUNC static void EM_LCD_DisplayImportActiveEnergyTariff(void)
//{
//    EM_LCD_DisplayInputActiveEnergyTariff(ENERGY_GetImportActiveEnergyTariff, EM_LCD_IMPORT_ENERGY);
//}
//
///******************************************************************************
//* Function Name: void EM_LCD_DisplayImportCapacitiveReactiveEnergyTariff(void)
//* Description  : Display import capacitive reactive energy follow tariff
//* Arguments    : none
//* Output       : none
//* Return Value : none
//******************************************************************************/
//LCD_FUNC static void EM_LCD_DisplayImportCapacitiveReactiveEnergyTariff(void)
//{
//    EM_LCD_DisplayInputReactiveEnergyTariff(ENERGY_GetImportCapacitiveReactiveEnergyTariff, EM_LCD_IMPORT_ENERGY, EM_LCD_CAPACITIVE_REACTIVE);
//}
//
///******************************************************************************
//* Function Name: void EM_LCD_DisplayImportInductiveReactiveEnergyTariff(void)
//* Description  : Display import inductive reactive energy follow tariff
//* Arguments    : none
//* Output       : none
//* Return Value : none
//******************************************************************************/
//LCD_FUNC static void EM_LCD_DisplayImportInductiveReactiveEnergyTariff(void)
//{
//    EM_LCD_DisplayInputReactiveEnergyTariff(ENERGY_GetImportInductiveReactiveEnergyTariff, EM_LCD_IMPORT_ENERGY, EM_LCD_INDUCTIVE_REACTIVE);
//}
//
///******************************************************************************
//* Function Name: void EM_LCD_DisplayImportApparentEnergyTariff(void)
//* Description  : Display apparent energy follow tariff
//* Arguments    : none
//* Output       : none
//* Return Value : none
//******************************************************************************/
//LCD_FUNC static void EM_LCD_DisplayImportApparentEnergyTariff(void)
//{
//    float   app_energy;
//    uint8_t current_tariff;
//    uint8_t return_status;
//
//    /* Get the current active active tariff */
//    current_tariff = ENERGY_TARIFF_GetCurrentTariff();
//
//    /* Get apparent energy at active tariff */
//    return_status = ENERGY_GetImportApparentEnergyTariff(current_tariff, &app_energy);
//    if (return_status == EM_OK)
//    {
//        EM_LCD_DisplayInputApparentEnergy(app_energy, EM_LCD_IMPORT_ENERGY);
//
//        EM_LCD_DisplayTariffNumber(current_tariff);
//    }
//}
//
///******************************************************************************
//* Function Name: void EM_LCD_DisplayExportActiveEnergyTariff(void)
//* Description  : Display export active energy follow tariff
//* Arguments    : none
//* Output       : none
//* Return Value : none
//******************************************************************************/
//LCD_FUNC static void EM_LCD_DisplayExportActiveEnergyTariff(void)
//{
//    EM_LCD_DisplayInputActiveEnergyTariff(ENERGY_GetExportActiveEnergyTariff, EM_LCD_EXPORT_ENERGY);
//}
//
///******************************************************************************
//* Function Name: void EM_LCD_DisplayExportCapacitiveReactiveEnergyTariff(void)
//* Description  : Display export capacitive reactive energy follow tariff
//* Arguments    : none
//* Output       : none
//* Return Value : none
//******************************************************************************/
//LCD_FUNC static void EM_LCD_DisplayExportCapacitiveReactiveEnergyTariff(void)
//{
//    EM_LCD_DisplayInputReactiveEnergyTariff(ENERGY_GetExportCapacitiveReactiveEnergyTariff, EM_LCD_EXPORT_ENERGY, EM_LCD_CAPACITIVE_REACTIVE);
//}
//
///******************************************************************************
//* Function Name: void EM_LCD_DisplayExportInductiveReactiveEnergyTariff(void)
//* Description  : Display export inductive reactive energy follow tariff
//* Arguments    : none
//* Output       : none
//* Return Value : none
//******************************************************************************/
//LCD_FUNC static void EM_LCD_DisplayExportInductiveReactiveEnergyTariff(void)
//{
//    EM_LCD_DisplayInputReactiveEnergyTariff(ENERGY_GetExportInductiveReactiveEnergyTariff, EM_LCD_EXPORT_ENERGY, EM_LCD_INDUCTIVE_REACTIVE);
//}
//
///******************************************************************************
//* Function Name: void EM_LCD_DisplayExportApparentEnergyTariff(void)
//* Description  : Display apparent energy follow tariff
//* Arguments    : none
//* Output       : none
//* Return Value : none
//******************************************************************************/
//LCD_FUNC static void EM_LCD_DisplayExportApparentEnergyTariff(void)
//{
//    float   app_energy;
//    uint8_t current_tariff;
//    uint8_t return_status;
//
//    /* Get the current active active tariff */
//    current_tariff = ENERGY_TARIFF_GetCurrentTariff();
//
//    /* Get apparent energy at active tariff */
//    return_status = ENERGY_GetExportApparentEnergyTariff(current_tariff, &app_energy);
//    if (return_status == EM_OK)
//    {
//        EM_LCD_DisplayInputApparentEnergy(app_energy, EM_LCD_EXPORT_ENERGY);
//
//        EM_LCD_DisplayTariffNumber(current_tariff);
//    }
//}
//
///******************************************************************************
//* Function Name: void EM_LCD_DisplayActMaxDemand(void)
//* Description  : Display active max demand value
//* Arguments    : none
//* Output       : none
//* Return Value : none
//******************************************************************************/
//LCD_FUNC static void EM_LCD_DisplayActMaxDemand(void)
//{
//    float32_t   actMD;
//    uint8_t     return_status;
//
//    actMD = ENERGY_GetActiveMaxDemand();
//
//    /* Modify to get enough length for LCD if necessary */
//    /* TBD */
//
//    /* Display measured value */
//    return_status = LCD_DisplayFloat(actMD);
//    if (return_status == LCD_WRONG_INPUT_ARGUMENT)
//    {
//        /* TODO: notice input value is out of range */
//    }
//
//    /* Display "kW" sign */
//    LCD_DisplaySpSign(S_k);
//    LCD_DisplaySpSign(S_V);
//    LCD_DisplaySpSign(S_BWSL);
//    LCD_DisplaySpSign(S_FWSL);
//
//    /* Display "MD" sign on LCD screen */
//    LCD_DisplaySpSign(S_MD);
//}
//
///******************************************************************************
//* Function Name: void EM_LCD_DisplayCapacitiveReactiveMaxDemand(void)
//* Description  : Display capacitive reactive max demand value
//* Arguments    : none
//* Output       : none
//* Return Value : none
//******************************************************************************/
//LCD_FUNC void EM_LCD_DisplayCapacitiveReactiveMaxDemand(void)
//{
//    EM_LCD_DisplayInputReactiveMaxDemand(ENERGY_GetCapacitiveReactiveMaxDemand(), EM_LCD_CAPACITIVE_REACTIVE);
//}
//
///******************************************************************************
//* Function Name: void EM_LCD_DisplayInductiveReactiveMaxDemand(void)
//* Description  : Display inductive reactive max demand value
//* Arguments    : none
//* Output       : none
//* Return Value : none
//******************************************************************************/
//LCD_FUNC void EM_LCD_DisplayInductiveReactiveMaxDemand(void)
//{
//    EM_LCD_DisplayInputReactiveMaxDemand(ENERGY_GetInductiveReactiveMaxDemand(), EM_LCD_INDUCTIVE_REACTIVE);
//}
//
///******************************************************************************
//* Function Name: void EM_LCD_DisplayAppMaxDemand(void)
//* Description  : Display apparent max demand value
//* Arguments    : none
//* Output       : none
//* Return Value : none
//******************************************************************************/
//LCD_FUNC static void EM_LCD_DisplayAppMaxDemand(void)
//{
//    float32_t   appMD;
//    uint8_t     return_status;
//
//    appMD = ENERGY_GetApparentMaxDemand();
//
//    /* Modify to get enough length for LCD if necessary */
//    /* TBD */
//
//    /* Display measured value */
//    return_status = LCD_DisplayFloat(appMD);
//    if (return_status == LCD_WRONG_INPUT_ARGUMENT)
//    {
//        /* TODO: notice input value is out of range */
//    }
//
//    /* Display "kVA" sign */
//    LCD_DisplaySpSign(S_k);
//    LCD_DisplaySpSign(S_V);
//    LCD_DisplaySpSign(S_FWSL);
//    LCD_DisplaySpSign(S_A);
//
//    /* Display "MD" sign on LCD screen */
//    LCD_DisplaySpSign(S_MD);
//}

/******************************************************************************
* Function Name: void EM_SwitchAutoRoll(void)
* Description  : Toogle between using auto roll and not use auto roll
* Arguments    : none
* Output       : [g_auto_roll]: flag to determine if using auto roll or not
* Return Value :
******************************************************************************/
LCD_FUNC void LCD_SwitchAutoRoll(void)
{
    /* Toogle auto scroll flag */
    g_auto_roll ^= 0x1;
    /* Reset display counter */
    g_em_display_count = 0;
}

/******************************************************************************
* Function Name: void EM_ChangeNext(void)
* Description  : Switch to the next display value on EM
* Arguments    : none
* Output       : [g_display_pos] determine the order of displayed information
*              : [g_em_display_count] the counter for displaying
* Return Value : none
******************************************************************************/
LCD_FUNC void LCD_ChangeNext(void)
{
    /* Determine the next kind of value which is displayed on LCD */
    g_DispCounter++;
    switch (g_byDisplayMode)
    {
    case 2:
        if ((fp_display2[g_DispCounter] == LCD_DisplayLastTokenRechargeAmount) && (Payment_Mode == e_Postpaid))
        {
                g_DispCounter += 7;
        }
        if ((fp_display2[g_DispCounter] == EM_LCD_DisplayExportActiveEnergy) && (e_metering_mode == e_Import_Only))
        {
                g_DispCounter += 8;
        }
        if (fp_display2[g_DispCounter] == NULL)
        {
            g_DispCounter = 0;
        }
        break;

    case 3:
        if (fp_display3[g_DispCounter] == NULL)
        {
            g_DispCounter = 0;
        }
        break;

    case 1:
    default:
        if ((fp_display[g_DispCounter] == LCD_DisplayLastTokenRechargeAmount) && (Payment_Mode == e_Postpaid))
        {
                g_DispCounter += 7;
        }
        if ((fp_display[g_DispCounter] == EM_LCD_DisplayExportActiveEnergy) && (e_metering_mode == e_Import_Only))
        {
                g_DispCounter += 8;
        }
        if (fp_display[g_DispCounter] == NULL)
        {
            g_DispCounter = 0;
        }
        break;
    }
    g_is_display_changed_next = 1;
    /* Reset display counter */
    g_em_display_count = 0;
}

/******************************************************************************
* Function Name: void LCD_DisplayTime(TIME_DATA_INFO time_info, uint8_t is_used_BCD)
* Description  : Display the set time of EM on LCD screen
* Arguments    : [time_info]struct which store the displayed time
*              : [is_used_BCD] = 1 : displayed time is on BCD type
*              :               = 0 : displayed time is on normal type
* Output       : [LCD_WRONG_INPUT_ARGUMENT] input argument error
*              : [LCD_INPUT_OK]             input argument ok
* Return Value : none
******************************************************************************/
LCD_FUNC uint8_t LCD_DisplayTime(TIME_DATA_INFO time_info, uint8_t is_used_BCD)
{
    uint32_t    disp_data;
    uint8_t     is_zero_hour = 0;

    /* If input data is BCD type, convert it into normal type */
    if (is_used_BCD == 1)
    {
        time_info.sec = WRP_EXT_Bcd2Dec(time_info.sec);
        time_info.min = WRP_EXT_Bcd2Dec(time_info.min);
        time_info.hour = WRP_EXT_Bcd2Dec(time_info.hour);
    }

    /* Display time on LCD */
    /* Check hour value to guarantee it differs than 0 */
    if (time_info.hour <= 0)
    {
        is_zero_hour = 1;
        /* Add 1 to hour to compensate displayed value */
        time_info.hour = 1;
    }

    disp_data = time_info.hour * (uint32_t)10000;
    disp_data += time_info.min * (uint32_t)100;
    disp_data += time_info.sec;

    LCD_DisplayIntWithPos(disp_data, LCD_LAST_POS_DIGIT);
    /* If the digit of hour less than 10 */
    if (is_zero_hour == 1)
    {
        LCD_DisplayDigit(4, 0); /* Display 0 at lower digit of hour value */
    }
    if (time_info.hour < 10)
    {
        LCD_DisplayDigit(3, 0); /* Display 0 at higher digit of hour value */
    }

    /* Clear the 8th digit on LCD */
    //LCD_ClearDigit(8);

    /* Diplay ":" between hour:min:sec */
    //LCD_DisplaySpSign(S_D1);
    LCD_DisplaySpSign(S_SEP);

    return LCD_INPUT_OK;
}

/******************************************************************************
* Function Name: void LCD_DisplayFloat(float32_t fnum)
* Description  : Display float number on LCD screen
* Arguments    : [fNum] displayed number
* Output       : none
* Return Value : LCD_INPUT_WRONG_ARGUMENT: length of integer part is more than 5
               : LCD_OK: length of input number is ok
******************************************************************************/
float roundTo3Decimal(float num) {
    float multiplier = powf(10, 3);
    return (float)((int32_t)(num * multiplier) / multiplier);
}
LCD_FUNC uint8_t LCD_DisplayFloat(float32_t fnum)
{
    /* Information of input number */
    uint8_t     i, sign;            /* Get sign of input number */
    float       f_fra_part;     /* Integer and fraction part of fnum at float type */
    uint8_t     num_fra_digit;  /* Get the number of number on fractional part */

    /* Information for displaying process */
    uint8_t     disp_status = LCD_INPUT_OK;
    uint8_t     is_zero_point = 0;
    //uint8_t     is_input_float_number = 0;  /* Check the input type */
    int32_t    ref_value;
    //fnum = roundTo3Decimal(fnum);

    /* Check the validation of fnum */
    if (fnum > 9999999) /* Out of allowable range of float type
                         * supported by CA78K0 compiler */
    {
        disp_status = LCD_WRONG_INPUT_ARGUMENT;
    }

    /* Get the number of digit on fractional part */
    num_fra_digit = (LCD_LAST_POS_DIGIT - g_DecInfo2.pos) + 1;

    //    /* Check if input number is flaoting type or integer type */
    //    f_fra_part = fnum - (int32_t)fnum;
    //    is_input_float_number = 1;

    //    /* Check if the input number is on [0,1] */
    //    if ((fnum >= 0) && (fnum < 1))
    //    {
    //        is_zero_point = 1;
    //        /* Compensate data when displayed digit less than number of digit on fractional part */
    //        fnum += 1;
    //    }
    //    if ((fnum > -1) && (fnum < 0))
    //    {
    //        is_zero_point = 1;
    //        /* Compensate data when displayed digit less than number of digit on fractional part */
    //        fnum -= 1;
    //    }

    //    /* Convert the floating type into integer type */
    //    if (is_input_float_number == 1)
    //    {
    //        for (i = 0; i < num_fra_digit; i++)
    //        {
    //            fnum *= 10;
    //            ref_value *= 10;
    //        }
    //    }

        /* The input number is less than 1/(10^(fra_digit)) */
    //    if ((WRP_EXT_Absf(fnum) < (ref_value + 1)) && (is_zero_point == 1))
    //    {
    //        fnum = 100;
    //    }

        /* Display integer number */

    ref_value = (int32_t)(((double)fnum) * 100);
    if ((ref_value < 100) && (ref_value >= 0))
    {
        ref_value += 100;
        is_zero_point = 1;
    }
    if ((ref_value > -100) && (ref_value < 0))
    {
        ref_value -= 100;
        is_zero_point = 1;
    }
    disp_status = LCD_DisplayIntWithPos(ref_value, LCD_LAST_POS_DIGIT);

    /* Display 0 if input number in [0,1] */
    if (is_zero_point)
    {
        LCD_DisplayDigit(g_DecInfo2.pos - 1, 0);
    }

    /* Display decimal point */
    //if (is_input_float_number == 1)
    {
        LCD_DisplaySpSign(g_DecInfo2.sign);
    }

    return disp_status;
}

/******************************************************************************
* Function Name: void LCD_DisplayFloat3Digit(float32_t fnum)
* Description  : Display float number on LCD screen
* Arguments    : [fNum] displayed number
* Output       : none
* Return Value : LCD_INPUT_WRONG_ARGUMENT: length of integer part is more than 5
               : LCD_OK: length of input number is ok
******************************************************************************/
LCD_FUNC uint8_t LCD_DisplayFloat3Digit(float32_t fnum)
{
    /* Information of input number */
    uint8_t     i, sign;            /* Get sign of input number */
    float32_t       f_fra_part;     /* Integer and fraction part of fnum at float type */
    uint8_t     num_fra_digit;  /* Get the number of number on fractional part */

    /* Information for displaying process */
    uint8_t     disp_status = LCD_INPUT_OK;
    uint8_t     is_zero_point = 0;
    uint8_t     is_input_float_number = 0;  /* Check the input type */
    uint32_t    ref_value = 1;

    /* Check the validation of fnum */
    if (WRP_EXT_Absf(fnum) < 0.001)
    {
        fnum = 0;
    }
    if (fnum >= 9999999) /* Out of allowable range of float type
                         * supported by CA78K0 compiler */
    {
        disp_status = LCD_WRONG_INPUT_ARGUMENT;
        fnum = fnum / 1000;
    }

    /* Get the number of digit on fractional part */
    num_fra_digit = (LCD_LAST_POS_DIGIT - g_DecInfo5.pos) + 1;

    /* Check if input number is flaoting type or integer type */
    f_fra_part = fnum - (int32_t)fnum;
    is_input_float_number = 1;

    /* Check if the input number is on [0,1] */
    if ((fnum >= 0) && (fnum < 1))
    {
        is_zero_point = 1;
        /* Compensate data when displayed digit less than number of digit on fractional part */
        fnum += 1;
    }
    if ((fnum > -1) && (fnum < 0))
    {
        is_zero_point = 1;
        /* Compensate data when displayed digit less than number of digit on fractional part */
        fnum -= 1;
    }

    /* Convert the floating type into integer type */
    if (is_input_float_number == 1)
    {
        for (i = 0; i < num_fra_digit; i++)
        {
            fnum *= 10;
            ref_value *= 10;
        }
    }

    /* Display integer number */
    disp_status = LCD_DisplayIntWithPos((int32_t)fnum, LCD_LAST_POS_DIGIT);

    /* Display 0 if input number in [0,1] */
    if (is_zero_point == 1)
    {
        LCD_DisplayDigit(g_DecInfo5.pos - 1, 0);
    }

    /* Display decimal point */
    if (is_input_float_number == 1)
    {
        LCD_DisplaySpSign(g_DecInfo5.sign);
    }

    return disp_status;
}

/******************************************************************************
* Function Name: uint8_t LCD_DisplayIntWithPos(long lNum, uint8_t position)
* Description  : Display integer number on LCD at specified position
* Arguments    :
*              :
* Output       :
* Return Value : LCD_WRONG_INPUT_ARGUMENT: input wrong argument
*              : LCD_INPUT_OK: input OK argument
******************************************************************************/
LCD_FUNC uint8_t LCD_DisplayIntWithPos(long lNum, int8_t position)
{
    int8_t pos = position;
    int8_t sign;            /* store the sign of value: positive, negative */
    uint8_t is_disp_error = LCD_INPUT_OK;

    /* check pos */
    if ((pos > LCD_LAST_POS_DIGIT) || (pos < LCD_FIRST_POS_DIGIT))
    {
        return LCD_WRONG_INPUT_ARGUMENT;
    }

    /* get the sign of value */
    sign = (lNum < 0) ? -1 : 1;
    lNum *= sign;               /* Get absolusted value of fNum for displaying */

    /* Clear all decimal point */
    LCD_ClearSpSign(S_D2);
    LCD_ClearSpSign(S_D4);
    LCD_ClearSpSign(S_D5);
    //LCD_ClearSpSign(S_D6);

    /* display all digit */
    do
    {
        LCD_DisplayDigit(pos--, (uint8_t)(lNum % 10));
        lNum /= 10;
    } while (lNum && pos >= LCD_FIRST_POS_DIGIT);

    if ((lNum != 0) && (pos < LCD_FIRST_POS_DIGIT)) /* overflow */
    {
        is_disp_error = LCD_WRONG_INPUT_ARGUMENT;
    }

    /* display the sign if there are some remain digit in the main region */
    if (sign < 0)
    {
        if (pos >= LCD_FIRST_POS_DIGIT)
        {
            LCD_DisplayDigit(pos--, LCD_MINUS_SIGN);
        }
        else
        {
            is_disp_error = LCD_WRONG_INPUT_ARGUMENT;
        }
    }

    /* Clear all other digit at left hand */
    while (pos >= LCD_FIRST_POS_DIGIT)
    {
        LCD_ClearDigit(pos--);
    }

    return is_disp_error;
}

/******************************************************************************
* Function Name: void EM_LCD_DisplayPOR(void)
* Description  : Display the sign to mark as PORSR
* Arguments    : None
* Return Value : NOne
******************************************************************************/
LCD_FUNC void EM_LCD_DisplayPOR(void)
{
    g_is_por = 0x01;
}

/******************************************************************************
* Function Name: void EM_LCD_DisplayNVMFail(void)
* Description  : Display the sign to mark as PORSR
* Arguments    : None
* Return Value : NOne
******************************************************************************/
LCD_FUNC void EM_LCD_DisplayNVMFail(void)
{
        
    LCD_ClearAll();
    //LCD_DisplaySpSign(S_LOGO);
    LCD_Print_string("NVM FAIL", sizeof("NVM FAIL") - 1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Display functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LCD_FUNC static void LCD_DisplayMode(void)
{
    LCD_ClearAll();
    //LCD_DisplaySpSign(S_LOGO);
    LCD_DisplayIntWithPos(g_byDisplayMode, LCD_LAST_POS_DIGIT);
    LCD_DisplayDigit(1, 'M');
    LCD_DisplayDigit(2, 'O');
    LCD_DisplayDigit(3, 'D');
    LCD_DisplayDigit(4, 'E');
}

LCD_FUNC static void EM_LCD_DisplayBillingCount(void)
{
    LCD_DisplayIntWithPos((uint32_t)get_cumulative_bill_count(), LCD_LAST_POS_DIGIT);

    /* Display "MD" sign on LCD screen */
    LCD_DisplaySpSign(S_BILL);

    LCD_DisplayDigit(1, 'C');
    LCD_DisplayDigit(2, 'N');
    LCD_DisplayDigit(3, 'T');
}

LCD_FUNC static void EM_LCD_DisplayBill_kWh(void)
{
    
    EM_LCD_DisplayInputActiveEnergy(g_Last_bill_log.u64_Cumm_Energy_KWh/100000.0, EM_LCD_IMPORT_ENERGY);
    /* Display "BILL" sign on LCD screen */
    LCD_DisplaySpSign(S_BILL);

    LCD_DisplayDigit_TZ(1, 'H');
    LCD_DisplayDigit_TZ(2, 1);
    //EM_LCD_DisplayImportExportStatus(EM_LCD_IMPORT_ENERGY);
}

#ifdef UTILITY_JNK
LCD_FUNC static void EM_LCD_DisplayBill_kWh_H2(void)
{
    EM_LCD_DisplayInputActiveEnergy(g_bill_kwh_h2 / 100000.0, EM_LCD_IMPORT_ENERGY);
    /* Display "BILL" sign on LCD screen */
    LCD_DisplaySpSign(S_BILL);

    LCD_DisplayDigit_TZ(1, 'H');
    LCD_DisplayDigit_TZ(2, 2);
    //LCD_DisplaySpSign(S_IMPO);
}

LCD_FUNC static void EM_LCD_DisplayBill_kWh_H3(void)
{
    EM_LCD_DisplayInputActiveEnergy(g_bill_kwh_h3 / 100000.0, EM_LCD_IMPORT_ENERGY);
    /* Display "BILL" sign on LCD screen */
    LCD_DisplaySpSign(S_BILL);

    LCD_DisplayDigit_TZ(1, 'H');
    LCD_DisplayDigit_TZ(2, 3);
    //LCD_DisplaySpSign(S_IMPO);
}
#endif

LCD_FUNC static void EM_LCD_DisplayBill_kVAh(void)
{
    EM_LCD_DisplayInputApparentEnergy(g_Last_bill_log.u64_Cumm_Energy_KVAh/100000.0, EM_LCD_IMPORT_ENERGY);

    /* Display "BILL" sign on LCD screen */
    LCD_DisplaySpSign(S_BILL);

    LCD_DisplayDigit_TZ(1, 'H');
    LCD_DisplayDigit_TZ(2, 1);
    //EM_LCD_DisplayImportExportStatus(EM_LCD_IMPORT_ENERGY);
}

#ifdef UTILITY_JNK
LCD_FUNC static void EM_LCD_DisplayCurrentBill_PF(void)
{
    ONE_MONTH_ENERGY_DATA_LOG profile_log;
    get_running_billing_data(&profile_log);
    LCD_DisplayFloat(((float)profile_log.u8_Sys_Power_Factor) / 100);

    /* Display "BILL" sign on LCD screen */
    LCD_DisplaySpSign(S_BILL);

    LCD_DisplayDigit(1, 'P');
    LCD_DisplayDigit(2, 'F');
}
#endif

LCD_FUNC static void EM_LCD_DisplayBill_PF(void)
{
    LCD_DisplayFloat(((float)g_Last_bill_log.u8_Sys_Power_Factor)/100.0);

    /* Display "BILL" sign on LCD screen */
    LCD_DisplaySpSign(S_BILL);

    LCD_DisplayDigit_TZ(1, 'H');
    LCD_DisplayDigit_TZ(2, 1);
    LCD_DisplayDigit(1, 'P');
    LCD_DisplayDigit(2, 'F');
}

LCD_FUNC static void EM_LCD_DisplayBill_MDkW(void)
{
    LCD_DisplayFloat(((float)g_Last_bill_log.u32_MD_KW)/100000.0);

    /* Display "kW" sign */
    LCD_DisplaySpSign(S_k);
    LCD_DisplaySpSign(S_V);
    LCD_DisplaySpSign(S_BWSL);
    LCD_DisplaySpSign(S_FWSL);

    /* Display "BILL" sign on LCD screen */
    LCD_DisplaySpSign(S_BILL);

    /* Display "MD" sign on LCD screen */
    LCD_DisplaySpSign(S_MD);

    LCD_DisplayDigit_TZ(1, 'H');
    LCD_DisplayDigit_TZ(2, 1);
    //EM_LCD_DisplayImportExportStatus(EM_LCD_IMPORT_ENERGY);
}
#ifdef UTILITY_JNK
LCD_FUNC static void EM_LCD_DisplayBill_MDkW_H2(void)
{
    LCD_DisplayFloat(((float)g_bill_mdkw_h2) / 100000.0);

    /* Display "kW" sign */
    LCD_DisplaySpSign(S_k);
    LCD_DisplaySpSign(S_V);
    LCD_DisplaySpSign(S_BWSL);
    LCD_DisplaySpSign(S_FWSL);

    /* Display "BILL" sign on LCD screen */
    LCD_DisplaySpSign(S_BILL);

    /* Display "MD" sign on LCD screen */
    LCD_DisplaySpSign(S_MD);

    LCD_DisplayDigit_TZ(1, 'H');
    LCD_DisplayDigit_TZ(2, 2);
    //LCD_DisplaySpSign(S_IMPO);
}

LCD_FUNC static void EM_LCD_DisplayBill_MDkW_H3(void)
{
    LCD_DisplayFloat(((float)g_bill_mdkw_h3) / 100000.0);

    /* Display "kW" sign */
    LCD_DisplaySpSign(S_k);
    LCD_DisplaySpSign(S_V);
    LCD_DisplaySpSign(S_BWSL);
    LCD_DisplaySpSign(S_FWSL);

    /* Display "BILL" sign on LCD screen */
    LCD_DisplaySpSign(S_BILL);

    /* Display "MD" sign on LCD screen */
    LCD_DisplaySpSign(S_MD);

    LCD_DisplayDigit_TZ(1, 'H');
    LCD_DisplayDigit_TZ(2, 3);
    //LCD_DisplaySpSign(S_IMPO);
}
#endif
LCD_FUNC static void EM_LCD_DisplayBill_MDkVA(void)
{
    LCD_DisplayFloat(((float)g_Last_bill_log.u32_MD_KVA)/100000.0);

    /* Display "kW" sign */
    LCD_DisplaySpSign(S_k);
    LCD_DisplaySpSign(S_V);
    LCD_DisplaySpSign(S_FWSL);
    LCD_DisplaySpSign(S_A);

    /* Display "BILL" sign on LCD screen */
    LCD_DisplaySpSign(S_BILL);

    /* Display "MD" sign on LCD screen */
    LCD_DisplaySpSign(S_MD);

    LCD_DisplayDigit_TZ(1, 'H');
    LCD_DisplayDigit_TZ(2, 1);
    EM_LCD_DisplayImportExportStatus(EM_LCD_IMPORT_ENERGY);
}

LCD_FUNC static void EM_LCD_Display_MDkW(void)
{
    LCD_DisplayFloat((get_mdkw_value(0) / 100000.0));

    /* Display "kW" sign */
    LCD_DisplaySpSign(S_k);
    LCD_DisplaySpSign(S_V);
    LCD_DisplaySpSign(S_BWSL);
    LCD_DisplaySpSign(S_FWSL);

    /* Display "BILL" sign on LCD screen */
    //LCD_DisplaySpSign(S_BILL);

    /* Display "MD" sign on LCD screen */
    LCD_DisplaySpSign(S_MD);
    EM_LCD_DisplayImportExportStatus(EM_LCD_IMPORT_ENERGY);
}
LCD_FUNC static void EM_LCD_Display_MDkVA(void)
{
    //R_DLMS_DataLog_Get_CurrentCycleBillingParameters(&Current_bill_log);
    LCD_DisplayFloat((get_mdkva_value(0) / 100000.0));

    /* Display "kW" sign */
    LCD_DisplaySpSign(S_k);
    LCD_DisplaySpSign(S_V);
    LCD_DisplaySpSign(S_FWSL);
    LCD_DisplaySpSign(S_A);

    /* Display "BILL" sign on LCD screen */
    //LCD_DisplaySpSign(S_BILL);

    /* Display "MD" sign on LCD screen */
    LCD_DisplaySpSign(S_MD);
    EM_LCD_DisplayImportExportStatus(EM_LCD_IMPORT_ENERGY);
}

LCD_FUNC static void EM_LCD_DisplayBill_TC(void)
{
    LCD_DisplayIntWithPos(g_Last_bill_log.u32_tamper_count, LCD_LAST_POS_DIGIT);

    /* Display "BILL" sign on LCD screen */
    LCD_DisplaySpSign(S_BILL);

    LCD_DisplayDigit_TZ(1, 'H');
    LCD_DisplayDigit_TZ(2, 1);
    LCD_DisplayDigit(1, 'T');
    LCD_DisplayDigit(2, 'C');
}

LCD_FUNC void EM_LCD_DisplayBill_kWh_TimeZone(uint8_t byTimeZone, uint8_t byHistory)
{
    if (0 != byHistory)
    {
        LCD_DisplayFloat(((float)g_Last_bill_log.u64_Cumm_Energy_KWh_TZ[byTimeZone - 1])/100000.0);
        LCD_DisplayDigit(1, 'H');
        /* Display "BILL" sign on LCD screen */
        LCD_DisplaySpSign(S_BILL);
    }
    else
    {
        LCD_DisplayFloat(((float)get_kwh_time_zone(byTimeZone))/100000.0);
        //R_DLMS_DataLog_Get_CurrentCycleBillingParameters(&Current_bill_log);
//TODO: Rakesh single ph        LCD_DisplayFloat(Current_bill_log.value[1 + byTimeZone]);
    }

    /* Display "kWh" sign */
    LCD_DisplaySpSign(S_k);
    LCD_DisplaySpSign(S_V);
    LCD_DisplaySpSign(S_BWSL);
    LCD_DisplaySpSign(S_FWSL);
    LCD_DisplaySpSign(S_h);

    LCD_DisplayDigit_TZ(1, 'T');
    LCD_DisplayDigit_TZ(2, byTimeZone);
    //EM_LCD_DisplayImportExportStatus(EM_LCD_IMPORT_ENERGY);
}

LCD_FUNC void EM_LCD_DisplayBill_kVAh_TimeZone(uint8_t byTimeZone, uint8_t byHistory)
{
    if (0 != byHistory)
    {
        LCD_DisplayFloat(((float)g_Last_bill_log.u64_Cumm_Energy_KVAh_TZ[byTimeZone - 1])/100000.0);
        LCD_DisplayDigit(1, 'H');
        /* Display "BILL" sign on LCD screen */
        LCD_DisplaySpSign(S_BILL);
    }
    else
    {
        LCD_DisplayFloat(((float)get_kvah_time_zone(byTimeZone))/100000.0);
    }

    /* Display "kVAh" sign */
    LCD_DisplaySpSign(S_k);
    LCD_DisplaySpSign(S_V);
    LCD_DisplaySpSign(S_FWSL);
    LCD_DisplaySpSign(S_A);
    LCD_DisplaySpSign(S_h);

    LCD_DisplayDigit_TZ(1, 'T');
    LCD_DisplayDigit_TZ(2, byTimeZone);
    //EM_LCD_DisplayImportExportStatus(EM_LCD_IMPORT_ENERGY);
}

LCD_FUNC static void EM_LCD_DisplayBill_kWh_T1(void)
{
    EM_LCD_DisplayBill_kWh_TimeZone(1, 1);
}
LCD_FUNC static void EM_LCD_DisplayBill_kWh_T2(void)
{
    EM_LCD_DisplayBill_kWh_TimeZone(2, 1);
}
LCD_FUNC static void EM_LCD_DisplayBill_kWh_T3(void)
{
    EM_LCD_DisplayBill_kWh_TimeZone(3, 1);
}

LCD_FUNC static void EM_LCD_Display_kWh_T1(void)
{
    EM_LCD_DisplayBill_kWh_TimeZone(1, 0);
}
LCD_FUNC static void EM_LCD_Display_kWh_T2(void)
{
    EM_LCD_DisplayBill_kWh_TimeZone(2, 0);
}
LCD_FUNC static void EM_LCD_Display_kWh_T3(void)
{
    EM_LCD_DisplayBill_kWh_TimeZone(3, 0);
}
LCD_FUNC static void EM_LCD_Display_kWh_T4(void)
{
    EM_LCD_DisplayBill_kWh_TimeZone(4, 0);
}
LCD_FUNC static void EM_LCD_Display_kWh_T5(void)
{
    EM_LCD_DisplayBill_kWh_TimeZone(5, 0);
}
LCD_FUNC static void EM_LCD_Display_kWh_T6(void)
{
    EM_LCD_DisplayBill_kWh_TimeZone(6, 0);
}


LCD_FUNC static void EM_LCD_Display_kVAh_T1(void)
{
    EM_LCD_DisplayBill_kVAh_TimeZone(1, 0);
}
LCD_FUNC static void EM_LCD_Display_kVAh_T2(void)
{
    EM_LCD_DisplayBill_kVAh_TimeZone(2, 0);
}
LCD_FUNC static void EM_LCD_Display_kVAh_T3(void)
{
    EM_LCD_DisplayBill_kVAh_TimeZone(3, 0);
}
LCD_FUNC static void EM_LCD_Display_kVAh_T4(void)
{
    EM_LCD_DisplayBill_kVAh_TimeZone(4, 0);
}
LCD_FUNC static void EM_LCD_Display_kVAh_T5(void)
{
    EM_LCD_DisplayBill_kVAh_TimeZone(5, 0);
}
LCD_FUNC static void EM_LCD_Display_kVAh_T6(void)
{
    EM_LCD_DisplayBill_kVAh_TimeZone(6, 0);
}

LCD_FUNC void EM_DisplayBill_MD_Date(uint8_t bykW_kVA, uint8_t byHistory)
{
    uint8_t data_ptr[12];
    TIME_DATA_INFO disp_data;
    

    if(!bykW_kVA)
    {
        /* Set time data to display */
        if (0 != byHistory)
        {
            //Epoch_To_DLMS_Time(g_Last_bill_log.u32_MD_KW_Epoch, data_ptr);
            switch (byHistory)
            {
            case 1: Epoch_To_DLMS_Time(g_Last_bill_log.u32_MD_KW_Epoch, data_ptr); break;
#ifdef UTILITY_JNK
            case 2: Epoch_To_DLMS_Time(g_bill_mdkw_epoch_h2, data_ptr); break;
            case 3: Epoch_To_DLMS_Time(g_bill_mdkw_epoch_h3, data_ptr); break;
#endif
            }
        }
        else
        {
            Epoch_To_DLMS_Time(get_mdkw_time(0), data_ptr);
        }
    }
    else
    {
        if (0 != byHistory)
        {
            Epoch_To_DLMS_Time(g_Last_bill_log.u32_MD_KVA_Epoch, data_ptr);
        }
        else
        {
            Epoch_To_DLMS_Time(get_mdkva_time(0), data_ptr);
        }
    }
    disp_data.hour = data_ptr[3];
    disp_data.min = data_ptr[2];
    disp_data.sec = (((uint16_t)data_ptr[0]<<8) + data_ptr[1])%100;
    LCD_DisplayTime(disp_data, 0);
    LCD_DisplaySpSign(S_DATE);
    LCD_DisplaySpSign(S_MD);
    EM_LCD_DisplayImportExportStatus(EM_LCD_IMPORT_ENERGY);
    if (0 != byHistory)
    {
        //LCD_DisplayDigit(1, 'H');
        //LCD_DisplayDigit(2, byHistory);
        LCD_DisplayDigit_TZ(1, 'H');
        LCD_DisplayDigit_TZ(2, byHistory);
        LCD_DisplaySpSign(S_BILL);
    }
}

LCD_FUNC void EM_DisplayBill_MD_Time(uint8_t bykW_kVA, uint8_t byHistory)
{
    uint8_t data_ptr[12];
    TIME_DATA_INFO disp_data;
    
    if(!bykW_kVA)
    {
        /* Set time data to display */
        if (0 != byHistory)
        {
            //Epoch_To_DLMS_Time(g_Last_bill_log.u32_MD_KW_Epoch, data_ptr);
            switch (byHistory)
            {
            case 1: Epoch_To_DLMS_Time(g_Last_bill_log.u32_MD_KW_Epoch, data_ptr); break;
#ifdef UTILITY_JNK
            case 2: Epoch_To_DLMS_Time(g_bill_mdkw_epoch_h2, data_ptr); break;
            case 3: Epoch_To_DLMS_Time(g_bill_mdkw_epoch_h3, data_ptr); break;
#endif
            }
        }
        else
        {
            Epoch_To_DLMS_Time(get_mdkw_time(0), data_ptr);
        }
    }
    else
    {
        if (0 != byHistory)
        {
            Epoch_To_DLMS_Time(g_Last_bill_log.u32_MD_KVA_Epoch, data_ptr);
        }
        else
        {
            Epoch_To_DLMS_Time(get_mdkva_time(0), data_ptr);
        }
    }
    disp_data.hour = data_ptr[5];
    disp_data.min = data_ptr[6];
    disp_data.sec = data_ptr[7];
    LCD_DisplayTime(disp_data, 0);
    //LCD_DisplaySpSign(S_TIME);
    LCD_DisplaySpSign(S_MD);
    EM_LCD_DisplayImportExportStatus(EM_LCD_IMPORT_ENERGY);
    if (0 != byHistory)
    {
        //LCD_DisplayDigit(1, 'H');
        //LCD_DisplayDigit(2, byHistory);
        LCD_DisplayDigit_TZ(1, 'H');
        LCD_DisplayDigit_TZ(2, byHistory);
        LCD_DisplaySpSign(S_BILL);
    }
}

LCD_FUNC static void EM_LCD_DisplayBill_MDkVA_Time(void)
{
    EM_DisplayBill_MD_Time(1, 1);
}
LCD_FUNC static void EM_LCD_DisplayBill_MDkVA_Date(void)
{
    EM_DisplayBill_MD_Date(1, 1);
}
LCD_FUNC static void EM_LCD_DisplayBill_MDkW_Time(void)
{
    EM_DisplayBill_MD_Time(0, 1);
}
#ifdef UTILITY_JNK
LCD_FUNC static void EM_LCD_DisplayBill_MDkW_Time_H2(void)
{
    EM_DisplayBill_MD_Time(0, 2);
}
LCD_FUNC static void EM_LCD_DisplayBill_MDkW_Time_H3(void)
{
    EM_DisplayBill_MD_Time(0, 3);
}
#endif
LCD_FUNC static void EM_LCD_DisplayBill_MDkW_Date(void)
{
    EM_DisplayBill_MD_Date(0, 1);
}
#ifdef UTILITY_JNK
LCD_FUNC static void EM_LCD_DisplayBill_MDkW_Date_H2(void)
{
    EM_DisplayBill_MD_Date(0, 2);
}

LCD_FUNC static void EM_LCD_DisplayBill_MDkW_Date_H3(void)
{
    EM_DisplayBill_MD_Date(0, 3);
}
#endif

LCD_FUNC static void EM_LCD_Display_MDkVA_Time(void)
{
    EM_DisplayBill_MD_Time(1, 0);
}
LCD_FUNC static void EM_LCD_Display_MDkVA_Date(void)
{
    EM_DisplayBill_MD_Date(1, 0);
}
LCD_FUNC static void EM_LCD_Display_MDkW_Time(void)
{
    EM_DisplayBill_MD_Time(0, 0);
}
LCD_FUNC static void EM_LCD_Display_MDkW_Date(void)
{
    EM_DisplayBill_MD_Date(0, 0);
}

LCD_FUNC static void EM_LCD_DisplayCumPONHrs(void)
{
#if (defined(UTILITY_INTELLI) || defined(UTILITY_APRAAVA) || (UTILITY_PURBANCHAL))
    LCD_DisplayIntWithPos(get_cumu_power_on_time() / 60, LCD_LAST_POS_DIGIT);
#elif UTILITY_JNK
    LCD_DisplayIntWithPos(get_cumu_power_on_time() / 3600, LCD_LAST_POS_DIGIT);
#endif

    LCD_DisplayDigit(1, 'O');
    LCD_DisplayDigit(2, 'N');
    LCD_DisplayDigit(3, 'H');
    LCD_DisplayDigit(4, 'R');
}

LCD_FUNC static void Print_DisplayHiRes_Energy(int32_t energy)
{
    uint8_t low_enegry_flag = 0;

    if(energy < 100000)
    {
      low_enegry_flag = 1;
      energy += 100000;
    }
    LCD_DisplayIntWithPos(energy, LCD_LAST_POS_DIGIT);
    if(low_enegry_flag)
    {
      LCD_DisplayDigit(3, '0');
    }
}

LCD_FUNC static void EM_LCD_DisplayHiRes_kWh(void)
{
    int32_t iEnergy = get_energy(Active_Imp) % 100000000;// / 100;

    Print_DisplayHiRes_Energy(iEnergy);

    /* Display "KWh" sign */
    LCD_DisplaySpSign(S_k);
    LCD_DisplaySpSign(S_V);
    LCD_DisplaySpSign(S_BWSL);
    LCD_DisplaySpSign(S_FWSL);
    LCD_DisplaySpSign(S_h);

    LCD_DisplaySpSign(S_D1);
}

LCD_FUNC static void EM_LCD_DisplayHiRes_kVAh(void)
{
    int32_t iEnergy = get_energy(Apparent_Imp) % 100000000;// / 100;

    Print_DisplayHiRes_Energy(iEnergy);

    /* Display "KVAh" sign */
    LCD_DisplaySpSign(S_k);
    LCD_DisplaySpSign(S_V);
    LCD_DisplaySpSign(S_FWSL);
    LCD_DisplaySpSign(S_A);
    LCD_DisplaySpSign(S_h);

    LCD_DisplaySpSign(S_D1);
}


LCD_FUNC static void EM_LCD_DisplayHiRes_kWh_Export(void)
{
    int32_t iEnergy = get_energy(Active_Exp) % 100000000;// / 100;

    Print_DisplayHiRes_Energy(iEnergy);

    /* Display "KWh" sign */
    LCD_DisplaySpSign(S_k);
    LCD_DisplaySpSign(S_V);
    LCD_DisplaySpSign(S_BWSL);
    LCD_DisplaySpSign(S_FWSL);
    LCD_DisplaySpSign(S_h);

    LCD_DisplaySpSign(S_D1);
    EM_LCD_DisplayImportExportStatus(EM_LCD_EXPORT_ENERGY);
}

LCD_FUNC static void EM_LCD_DisplayHiRes_kVAh_Export(void)
{
    int32_t iEnergy = get_energy(Apparent_Exp) % 100000000;// / 100;

    Print_DisplayHiRes_Energy(iEnergy);

    /* Display "KVAh" sign */
    LCD_DisplaySpSign(S_k);
    LCD_DisplaySpSign(S_V);
    LCD_DisplaySpSign(S_FWSL);
    LCD_DisplaySpSign(S_A);
    LCD_DisplaySpSign(S_h);

    LCD_DisplaySpSign(S_D1);
    EM_LCD_DisplayImportExportStatus(EM_LCD_EXPORT_ENERGY);
}
LCD_FUNC static void EM_LCD_Display_RSSI(void)
{
    NIC_Type nic_type;
    int16_t rssi_value = get_RSSI_signal_value(&nic_type);
    LCD_DisplayIntWithPos(rssi_value, LCD_LAST_POS_DIGIT);
    if (nic_type == NIC_Type_RF)
    {
        LCD_Print_string("RSSI", sizeof("RSSI") - 1);
    }
    else if(nic_type == NIC_Type_4G)
    {
        LCD_Print_string("CSQ", sizeof("CSQ") - 1);
    }
    else
    {
        LCD_Print_string("NO-RF   ", sizeof("NO-RF   ") - 1);
    }
}
LCD_FUNC static void EM_LCD_DisplayTamperCount(void)
{
    LCD_DisplayIntWithPos((int32_t)get_tamper_counts(), LCD_LAST_POS_DIGIT);

    LCD_DisplayDigit(1, 'T');
    LCD_DisplayDigit(2, 'C');
    LCD_DisplayDigit(3, 'N');
    LCD_DisplayDigit(4, 'T');
}
///////////////////////////////////////////////////////////////////////////////////////////////////
//LCD_FUNC void LCD_Print_string(char *data, uint8_t len)
//{
//    uint8_t pos = 1;
//
//    while (len)
//    {
//        if (*data != '.')
//        {
//            LCD_DisplayDigit(pos, *data);
//            len--;
//            pos++;
//        }
//        //if((*data == '.') && (pos < 4))
//        //        pos++;
//        data++;
//    }
//}
//#ifdef UTILITY_JNK
LCD_FUNC static void EM_LCD_Display_OverVolt(void)
{
    LCD_Print_string("V-OVER F", LCD_NUM_DIGIT);
    if (st_tamper.g_tampers_bits.bits.over_volt != 0)
    {
        LCD_DisplayDigit(LCD_NUM_DIGIT, 'T');
    }
}
LCD_FUNC static void EM_LCD_Display_LowVolt(void)
{
    LCD_Print_string("V-LOW  F", LCD_NUM_DIGIT);
    if (st_tamper.g_tampers_bits.bits.low_volt != 0)
    {
        LCD_DisplayDigit(LCD_NUM_DIGIT, 'T');
    }
}
LCD_FUNC static void EM_LCD_Display_CurrentRev(void)
{
    LCD_Print_string("I-REV  F", LCD_NUM_DIGIT);
    if (st_tamper.g_tampers_bits.bits.r_ph_current_reverse != 0)
    {
        LCD_DisplayDigit(LCD_NUM_DIGIT, 'T');
    }
}
LCD_FUNC static void EM_LCD_Display_OverCurrent(void)
{
    LCD_Print_string("OVRCUR F", LCD_NUM_DIGIT);
    if (st_tamper.g_tampers_bits.bits.over_current != 0)
    {
        LCD_DisplayDigit(LCD_NUM_DIGIT, 'T');
    }
}
LCD_FUNC static void EM_LCD_Display_OverLoad(void)
{
    LCD_Print_string("O-LOAD F", LCD_NUM_DIGIT);
    if (st_tamper.g_tampers_bits.bits.over_load != 0)
    {
        LCD_DisplayDigit(LCD_NUM_DIGIT, 'T');
    }
}
LCD_FUNC static void EM_LCD_Display_EarthLoad(void)
{
    LCD_Print_string("ETH-LD F", LCD_NUM_DIGIT);
    if (st_tamper.g_tampers_bits.bits.earth_loading != 0)
    {
        LCD_DisplayDigit(LCD_NUM_DIGIT, 'T');
    }
}
LCD_FUNC static void EM_LCD_Display_Magnet(void)
{
    LCD_Print_string("MAGNET F", LCD_NUM_DIGIT);
    if (st_tamper.g_tampers_bits.bits.magnet_detected != 0)
    {
        LCD_DisplayDigit(LCD_NUM_DIGIT, 'T');
    }
}
LCD_FUNC static void EM_LCD_Display_NeutralDisturb(void)
{
    LCD_Print_string("N-DIS  F", LCD_NUM_DIGIT);
    if (st_tamper.g_tampers_bits.bits.neutral_disturbance != 0)
    {
        LCD_DisplayDigit(LCD_NUM_DIGIT, 'T');
    }
}
LCD_FUNC static void EM_LCD_Display_NeutralMiss(void)
{
    LCD_Print_string("N-MISS F", LCD_NUM_DIGIT);
    if (st_tamper.g_tampers_bits.bits.single_wire != 0)
    {
        LCD_DisplayDigit(LCD_NUM_DIGIT, 'T');
    }
}
LCD_FUNC static void EM_LCD_Display_RFMiss(void)
{
    LCD_Print_string("RF-MIS F", LCD_NUM_DIGIT);
    if (st_tamper.g_tampers_bits.bits.rf_removed != 0)
    {
        LCD_DisplayDigit(LCD_NUM_DIGIT, 'T');
    }
}
LCD_FUNC static void EM_LCD_Display_CoverOpen(void)
{
    LCD_Print_string("COPEN  F", LCD_NUM_DIGIT);
    if (st_tamper.g_tampers_bits.bits.cover_open != 0)
    {
        LCD_DisplayDigit(LCD_NUM_DIGIT, 'T');
    }
}
//LCD_FUNC static void EM_LCD_Display_FreqOut(void)
//{
//    LCD_Print_string("FRQ-OUT F", 9);
//    if (st_tamper.g_tampers_bits.bits.over_current != 0)
//    {
//        LCD_DisplayDigit(9, 'T');
//    }
//}
LCD_FUNC static void EM_LCD_Display_LowPF(void)
{
    LCD_Print_string("LOW-PF F", LCD_NUM_DIGIT);
    if (st_tamper.g_tampers_bits.bits.low_PF != 0)
    {
        LCD_DisplayDigit(LCD_NUM_DIGIT, 'T');
    }
}

LCD_FUNC static void EM_LCD_DisplayLastTamperOccur(void)
{
    ONE_EVENT_ID_LOG        last_event_id_log;
    read_page_eeprom(STORAGE_EEPROM_DLMS_LAST_TAMPER_OCCUR_ADDR, (uint8_t*)&last_event_id_log, STORAGE_EEPROM_DLMS_LAST_TAMPER_OCCUR_SIZE);
    switch (last_event_id_log.u16_event_code)
    {
    case 7:		LCD_Print_string("VOL-HI O", LCD_NUM_DIGIT);	break;
    case 9:		LCD_Print_string("VOL-LO O", LCD_NUM_DIGIT);	break;
    case 51:	LCD_Print_string("RE-CUR O", LCD_NUM_DIGIT);	break;
    case 67:	LCD_Print_string("OVRCUR O", LCD_NUM_DIGIT);	break;
    case 69:	LCD_Print_string("ETH-LD O", LCD_NUM_DIGIT);	break;
    case 201:	LCD_Print_string("MAGNET O", LCD_NUM_DIGIT);	break;
    case 203:	LCD_Print_string("N-DIST O", LCD_NUM_DIGIT);	break;
    case 205:	LCD_Print_string("LOW-PF O", LCD_NUM_DIGIT);	break;
    case 207:	LCD_Print_string("N-MISS O", LCD_NUM_DIGIT);	break;
    case 209:	LCD_Print_string("RF-MIS O", LCD_NUM_DIGIT);	break;
    case 215:	LCD_Print_string("O-LOAD O", LCD_NUM_DIGIT);	break;
    case 251:	LCD_Print_string("C-OPEN O", LCD_NUM_DIGIT);	break;
    case 253:	LCD_Print_string("WELD-S O", LCD_NUM_DIGIT);	break;
    default:    LCD_Print_string("TAMP-CLR", LCD_NUM_DIGIT);	break;
    }
}
LCD_FUNC static void EM_LCD_DisplayLastTamperOccurDate(void)
{
    stTime_struct time;
    TIME_DATA_INFO disp_data = { 0 };
    ONE_EVENT_ID_LOG        last_event_id_log;

    read_page_eeprom(STORAGE_EEPROM_DLMS_LAST_TAMPER_OCCUR_ADDR, (uint8_t*)&last_event_id_log, STORAGE_EEPROM_DLMS_LAST_TAMPER_OCCUR_SIZE);
    //if ((last_event_id_log.event_code == 7) || (last_event_id_log.event_code == 9) || (last_event_id_log.event_code == 51) || (last_event_id_log.event_code == 67) || (last_event_id_log.event_code == 69) || (last_event_id_log.event_code == 201) || (last_event_id_log.event_code == 203) || (last_event_id_log.event_code == 205) || (last_event_id_log.event_code == 207) || (last_event_id_log.event_code == 209) || (last_event_id_log.event_code == 215) || (last_event_id_log.event_code == 251))
    if ((last_event_id_log.u16_event_code != 0) && (last_event_id_log.u16_event_code < 300))
    {
        /* Set time data to display */
        UnixToDatteTime(last_event_id_log.u32_epoch, &time);
        disp_data.hour = time.day;
        disp_data.min = time.month;
        disp_data.sec = (uint8_t)(time.year % 100);
    }

    LCD_DisplayTime(disp_data, 0);
    LCD_DisplaySpSign(S_DATE);
}
LCD_FUNC static void EM_LCD_DisplayLastTamperOccurTime(void)
{
    stTime_struct time;
    TIME_DATA_INFO disp_data = { 0 };
    ONE_EVENT_ID_LOG        last_event_id_log;

    read_page_eeprom(STORAGE_EEPROM_DLMS_LAST_TAMPER_OCCUR_ADDR, (uint8_t*)&last_event_id_log, STORAGE_EEPROM_DLMS_LAST_TAMPER_OCCUR_SIZE);
    //if ((last_event_id_log.event_code == 7) || (last_event_id_log.event_code == 9) || (last_event_id_log.event_code == 51) || (last_event_id_log.event_code == 67) || (last_event_id_log.event_code == 69) || (last_event_id_log.event_code == 201) || (last_event_id_log.event_code == 203) || (last_event_id_log.event_code == 205) || (last_event_id_log.event_code == 207) || (last_event_id_log.event_code == 209) || (last_event_id_log.event_code == 215) || (last_event_id_log.event_code == 251))
    if ((last_event_id_log.u16_event_code != 0) && (last_event_id_log.u16_event_code < 300))
    {
        /* Set time data to display */
        UnixToDatteTime(last_event_id_log.u32_epoch, &time);
        disp_data.hour = time.hour;
        disp_data.min = time.minute;
        disp_data.sec = time.second;
    }

    LCD_DisplayTime(disp_data, 0);
    //LCD_DisplaySpSign(S_TIME);
}
LCD_FUNC static void EM_LCD_DisplayLastTamperRestore(void)
{
    ONE_EVENT_ID_LOG        last_event_id_log;

    read_page_eeprom(STORAGE_EEPROM_DLMS_LAST_TAMPER_RESTORE_ADDR, (uint8_t*)&last_event_id_log, STORAGE_EEPROM_DLMS_LAST_TAMPER_RESTORE_SIZE);
    switch (last_event_id_log.u16_event_code)
    {
    case 8:		LCD_Print_string("VOL-HI R", 9);	break;
    case 10:	LCD_Print_string("VOL-LO R", 9);	break;
    case 52:	LCD_Print_string("RE-CUR R", 9);	break;
    case 68:	LCD_Print_string("OVRCUR R", 9);	break;
    case 70:	LCD_Print_string("ETH-LD R", 9);	break;
    case 202:	LCD_Print_string("MAGNET R", 9);	break;
    case 204:	LCD_Print_string("N-DIST R", 9);	break;
    case 206:	LCD_Print_string("LOW-PF R", 9);	break;
    case 208:	LCD_Print_string("N-MISS R", 9);	break;
    case 210:	LCD_Print_string("RF-MIS R", 9);	break;
    case 216:	LCD_Print_string("O-LOAD R", 9);	break;
    default:    LCD_Print_string("TAMP-CLR", 9);	break;
    }
}
LCD_FUNC static void EM_LCD_DisplayLastTamperRestoreDate(void)
{
    stTime_struct time;
    TIME_DATA_INFO disp_data = { 0 };
    ONE_EVENT_ID_LOG        last_event_id_log;

    read_page_eeprom(STORAGE_EEPROM_DLMS_LAST_TAMPER_RESTORE_ADDR, (uint8_t*)&last_event_id_log, STORAGE_EEPROM_DLMS_LAST_TAMPER_RESTORE_SIZE);
    //if ((last_event_id_log.event_code == 8) || (last_event_id_log.event_code == 10) || (last_event_id_log.event_code == 52) || (last_event_id_log.event_code == 68) || (last_event_id_log.event_code == 70) || (last_event_id_log.event_code == 202) || (last_event_id_log.event_code == 204) || (last_event_id_log.event_code == 206) || (last_event_id_log.event_code == 208) || (last_event_id_log.event_code == 210) || (last_event_id_log.event_code == 216))
    if ((last_event_id_log.u16_event_code != 0) && (last_event_id_log.u16_event_code < 300))
    {
        /* Set time data to display */
        UnixToDatteTime(last_event_id_log.u32_epoch, &time);
        disp_data.hour = time.day;
        disp_data.min = time.month;
        disp_data.sec = (uint8_t)(time.year % 100);
    }

    LCD_DisplayTime(disp_data, 0);
    LCD_DisplaySpSign(S_DATE);
}
LCD_FUNC static void EM_LCD_DisplayLastTamperRestoreTime(void)
{
    stTime_struct time;
    TIME_DATA_INFO disp_data = { 0 };
    ONE_EVENT_ID_LOG        last_event_id_log;

    read_page_eeprom(STORAGE_EEPROM_DLMS_LAST_TAMPER_RESTORE_ADDR, (uint8_t*)&last_event_id_log, STORAGE_EEPROM_DLMS_LAST_TAMPER_RESTORE_SIZE);
    //if ((last_event_id_log.event_code == 8) || (last_event_id_log.event_code == 10) || (last_event_id_log.event_code == 52) || (last_event_id_log.event_code == 68) || (last_event_id_log.event_code == 70) || (last_event_id_log.event_code == 202) || (last_event_id_log.event_code == 204) || (last_event_id_log.event_code == 206) || (last_event_id_log.event_code == 208) || (last_event_id_log.event_code == 210) || (last_event_id_log.event_code == 216))
    if ((last_event_id_log.u16_event_code != 0) && (last_event_id_log.u16_event_code < 300))
    {
        /* Set time data to display */
        UnixToDatteTime(last_event_id_log.u32_epoch, &time);
        disp_data.hour = time.hour;
        disp_data.min = time.minute;
        disp_data.sec = time.second;
    }

    LCD_DisplayTime(disp_data, 0);
    //LCD_DisplaySpSign(S_TIME);
}
//#endif
//
LCD_FUNC static void EM_LCD_DisplaySerialNumber(void)
{
    static uint8_t toggle = 3;
    toggle++;
    if (2 == toggle)
    {
        LCD_Print_string((char*)&Meter_Sr_No[4], Meter_Sr_No[1] - 2);
    }
    else if (4 == toggle)
    {
        LCD_ClearAll();
        LCD_Print_string((char*)&Meter_Sr_No[2], 2);
        toggle = 0;
    }    
    LCD_DisplaySpSign(S_SNO);
}

//LCD_FUNC static void LCD_DisplayEEPROMCounter(void)
//{
//    LCD_DisplayIntWithPos(dI2CWriteCounter, LCD_LAST_POS_DIGIT);
//}
//
//
//
LCD_FUNC static void LCD_DisplayLastTokenRechargeAmount(void)
{
    int32_t Last_Token_Recharge_Amount = from_eeprom(STORAGE_DLMS_LAST_TOKEN_AMOUNT_ADDR, STORAGE_DLMS_LAST_TOKEN_AMOUNT_SIZE);
    LCD_DisplayFloat(Last_Token_Recharge_Amount /100.0);

    LCD_DisplayDigit_TZ(1, 'T');
    LCD_DisplayDigit_TZ(2, 'A');
    LCD_DisplaySpSign(S_RUPEE);
}

LCD_FUNC static void LCD_DisplayLastTokenRechargeDate(void)
{
    TIME_DATA_INFO      disp_data;
    uint8_t Last_Token_Recharge_Time[14];

    read_page_eeprom(STORAGE_DLMS_LAST_AMOUNT_RECHARGE_TIME_ADDR, &Last_Token_Recharge_Time[0], STORAGE_DLMS_LAST_AMOUNT_RECHARGE_TIME_SIZE);

    disp_data.hour = Last_Token_Recharge_Time[3];
    disp_data.min = Last_Token_Recharge_Time[2];
    disp_data.sec = (uint8_t)(((Last_Token_Recharge_Time[0] * 256) + Last_Token_Recharge_Time[1]) % 100);

    LCD_DisplayTime(disp_data, 0);
    LCD_DisplayDigit_TZ(1, 'T');
    LCD_DisplayDigit_TZ(2, 'A');
    LCD_DisplaySpSign(S_DATE);
}

LCD_FUNC static void LCD_DisplayLastTokenRechargeTime(void)
{
    TIME_DATA_INFO      disp_data;
    uint8_t Last_Token_Recharge_Time[14];

    read_page_eeprom(STORAGE_DLMS_LAST_AMOUNT_RECHARGE_TIME_ADDR, &Last_Token_Recharge_Time[0], STORAGE_DLMS_LAST_AMOUNT_RECHARGE_TIME_SIZE);

    disp_data.hour = Last_Token_Recharge_Time[5];
    disp_data.min = Last_Token_Recharge_Time[6];
    disp_data.sec = Last_Token_Recharge_Time[7];

    LCD_DisplayTime(disp_data, 0);
    LCD_DisplayDigit_TZ(1, 'T');
    LCD_DisplayDigit_TZ(2, 'A');
    //LCD_DisplaySpSign(S_TIME);
}

LCD_FUNC static void LCD_DisplayLastTotalAmount(void)
{
    int32_t Total_Amount_Last_Recharge = from_eeprom(STORAGE_DLMS_TOTAL_LAST_AMOUNT_RECHARGE_ADDR, STORAGE_DLMS_TOTAL_LAST_AMOUNT_RECHARGE_SIZE);
    LCD_DisplayFloat(Total_Amount_Last_Recharge/100.0);
    LCD_DisplayDigit_TZ(1, 'L');
    LCD_DisplayDigit_TZ(2, 'R');
    LCD_DisplaySpSign(S_RUPEE);
}

LCD_FUNC static void LCD_DisplayCurrentBalanceAmount(void)
{
    int32_t Current_Balance_Amount = from_eeprom(STORAGE_DLMS_CURRENT_BALANCE_AMOUNT_ADDR, STORAGE_DLMS_CURRENT_BALANCE_AMOUNT_SIZE);
    LCD_DisplayFloat(Current_Balance_Amount/100.0);
    LCD_DisplayDigit_TZ(1, 'C');
    LCD_DisplayDigit_TZ(2, 'B');
    LCD_DisplaySpSign(S_RUPEE);
}

LCD_FUNC static void LCD_DisplayCurrentBalanceDate(void)
{
    uint8_t Current_Balance_Time[14];
    TIME_DATA_INFO      disp_data;

    read_page_eeprom(STORAGE_DLMS_CURRENT_BALANCE_TIME_ADDR, &Current_Balance_Time[0], STORAGE_DLMS_CURRENT_BALANCE_TIME_SIZE);

    disp_data.hour = Current_Balance_Time[3];
    disp_data.min = Current_Balance_Time[2];
    disp_data.sec = (uint8_t)(((Current_Balance_Time[0] * 256) + Current_Balance_Time[1]) % 100);

    LCD_DisplayTime(disp_data, 0);
    LCD_DisplayDigit_TZ(1, 'C');
    LCD_DisplayDigit_TZ(2, 'B');
    LCD_DisplaySpSign(S_DATE);
}

LCD_FUNC static void LCD_DisplayCurrentBalanceTime(void)
{
    uint8_t Current_Balance_Time[14];
    TIME_DATA_INFO      disp_data;

    read_page_eeprom(STORAGE_DLMS_CURRENT_BALANCE_TIME_ADDR, &Current_Balance_Time[0], STORAGE_DLMS_CURRENT_BALANCE_TIME_SIZE);

    disp_data.hour = Current_Balance_Time[5];
    disp_data.min = Current_Balance_Time[6];
    disp_data.sec = Current_Balance_Time[7];

    LCD_DisplayTime(disp_data, 0);
    LCD_DisplayDigit_TZ(1, 'C');
    LCD_DisplayDigit_TZ(2, 'B');
    //LCD_DisplaySpSign(S_TIME);
}

//TODO: Rakesh Modify for export values... all the below written functions
LCD_FUNC static void EM_LCD_Display_Export_MDkW(void)
{
    //R_DLMS_DataLog_Get_CurrentCycleBillingParameters(&Current_bill_log);
    LCD_DisplayFloat((get_mdkw_expo_value() / 100000.0));

    /* Display "kW" sign */
    LCD_DisplaySpSign(S_k);
    LCD_DisplaySpSign(S_V);
    LCD_DisplaySpSign(S_BWSL);
    LCD_DisplaySpSign(S_FWSL);

    /* Display "BILL" sign on LCD screen */
    //LCD_DisplaySpSign(S_BILL);

    /* Display "MD" sign on LCD screen */
    LCD_DisplaySpSign(S_MD);
    EM_LCD_DisplayImportExportStatus(EM_LCD_EXPORT_ENERGY);
}
LCD_FUNC static void EM_LCD_Display_Export_MDkVA(void)
{
    //R_DLMS_DataLog_Get_CurrentCycleBillingParameters(&Current_bill_log);
    LCD_DisplayFloat((get_mdkva_expo_value() / 100000.0));

    /* Display "kW" sign */
    LCD_DisplaySpSign(S_k);
    LCD_DisplaySpSign(S_V);
    LCD_DisplaySpSign(S_FWSL);
    LCD_DisplaySpSign(S_A);

    /* Display "BILL" sign on LCD screen */
    //LCD_DisplaySpSign(S_BILL);

    /* Display "MD" sign on LCD screen */
    LCD_DisplaySpSign(S_MD);
    EM_LCD_DisplayImportExportStatus(EM_LCD_EXPORT_ENERGY);
}

LCD_FUNC void EM_DisplayExport_MD_Date_Time(uint8_t bykW_kVA, uint8_t bydate_time)
{
    TIME_DATA_INFO              disp_data;
    uint8_t data_ptr[12];

    if (0 == bykW_kVA)
    {
        Epoch_To_DLMS_Time(get_mdkw_expo_time(), data_ptr);
    }
    else
    {
        Epoch_To_DLMS_Time(get_mdkva_expo_time(), data_ptr);
    }
    /* Set time data to display */
    if (0 == bydate_time)
    {
        disp_data.hour = data_ptr[3];
        disp_data.min = data_ptr[2];
        disp_data.sec = (((uint16_t)data_ptr[0] << 8) + data_ptr[1]) % 100;
        LCD_DisplaySpSign(S_DATE);
    }
    else
    {
        disp_data.hour = data_ptr[5];
        disp_data.min = data_ptr[6];
        disp_data.sec = data_ptr[7];
        //LCD_DisplaySpSign(S_TIME);
    }
    LCD_DisplayTime(disp_data, 0);
    EM_LCD_DisplayImportExportStatus(EM_LCD_EXPORT_ENERGY);
}


LCD_FUNC static void EM_LCD_Display_Export_MDkVA_Time(void)
{
    EM_DisplayExport_MD_Date_Time(1, 1);
}
LCD_FUNC static void EM_LCD_Display_Export_MDkVA_Date(void)
{
    EM_DisplayExport_MD_Date_Time(1, 0);
}
LCD_FUNC static void EM_LCD_Display_Export_MDkW_Time(void)
{
    EM_DisplayExport_MD_Date_Time(0, 1);
}
LCD_FUNC static void EM_LCD_Display_Export_MDkW_Date(void)
{
    EM_DisplayExport_MD_Date_Time(0, 0);
}


