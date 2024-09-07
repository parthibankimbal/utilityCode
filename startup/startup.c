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
* File Name    : startup.c
* Version      : 1.00
* Device(s)    : RL78/I1C
* Tool-Chain   : 
* H/W Platform : RL78/I1C Energy Meter Platform
* Description  : Start-up source File
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
***********************************************************************************************************************/
/* Driver */
#include "r_cg_macrodriver.h"   /* CG Macro Driver */
#include "r_cg_sau.h"           /* Serial Driver */
#include "r_cg_lcd.h"           /* LCD Driver */
#include "r_cg_port.h"          /* Port Driver*/
#include "r_cg_tau.h"           /* Serial Driver */
#include "r_cg_rtc.h"           /* RTC Driver */
#include "r_cg_dsadc.h"         /* DSADC Driver */
#include "r_cg_sau.h"           /* Serial Driver */
#include "r_cg_lvd.h"           /* LVD Driver */
#include "r_cg_rtc.h"           /* RTC Driver */
#include "r_cg_intp.h"          /* INTP Driver */
#include "r_cg_iica.h"          /* IICA Driver */
#include "factory_settings.h"
#include "r_cg_userdefine.h"    /* CG User Define */
#include "r_cg_osdc.h"

/* Wrapper/User */
#include "wrp_em_mcu.h"

/* MW/Core */
#include "em_core.h"            /* EM Core APIs */

/* Application */
#include "platform.h"           /* Default Platform Information Header */
#include "config_storage.h"
#include "startup.h"            /* Startup Header File */
#include "main.h"

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Imported global variables and functions (from other files)
***********************************************************************************************************************/
extern void hdwinit(void);
/***********************************************************************************************************************
Exported global variables and functions (to be accessed by other files)
***********************************************************************************************************************/

st_em_startup_diag_t g_em_startup_diag;

/***********************************************************************************************************************
Private global variables and functions
***********************************************************************************************************************/

/***********************************************************************************************************************
* Function Name    : static uint8_t config_data_load(EM_CALIBRATION * p_calib, st_em_setting_t * p_em_setting)
* Description      : Load Configuration Page from MCU DataFlash memory
* Arguments        : uint8_t init_status: initialization status
* Return Value     : Execution Status
*                  :    CONFIG_OK         Load data OK
*                  :    CONFIG_ERROR         Load data error
***********************************************************************************************************************/
static uint8_t config_data_load( EM_CALIBRATION * p_calib, st_em_setting_t * p_em_setting)
{
    uint8_t init_status;

    /* Assign pointer for phase degree in calib holder value */
    p_calib->sw_phase_correction.i1_phase_degrees = p_em_setting->degree_list_i1;
    p_calib->sw_phase_correction.i2_phase_degrees = p_em_setting->degree_list_i2;

    /* Assign pointer for gain in calib to holder value */
    p_calib->sw_gain.i1_gain_values = p_em_setting->gain_list_i1;
    p_calib->sw_gain.i2_gain_values = p_em_setting->gain_list_i2;
    
    /* Init config */
    init_status = CONFIG_Init(0);
    
    /* Check device format to get out config data */
    if (init_status == CONFIG_OK)       /* Initial successful, already formatted */
    {

    }
    /* Initial successful, but not formatted */
    else if (init_status == CONFIG_NOT_FORMATTED)
    {
        /* Format device */
        if (CONFIG_Format() != CONFIG_OK)
        {
            /* When format fail,
             * we need to beak the start-up process here */     
            return CONFIG_ERROR;   /* Format fail */
        }       
        else    /* format ok */
        {

        }
    }
    /* Restore data from storage in every load config */
    if (CONFIG_LoadEMCalib(p_calib, &p_em_setting->regs) != CONFIG_OK)
    {
        NOP();
    }
    else
    {
        NOP();
    }

    /* Load data sucessfully */
    return CONFIG_OK;
}

/***********************************************************************************************************************
* Function Name    : uint8_t start_peripheral_and_app(void)
* Description      : Start-up energy meter
* Arguments        : None
* Return Value     : Execution Status
*                  :    STARTUP_OK              Start-up Ok
*                  :    STARTUP_ERROR           Startup error
***********************************************************************************************************************/
uint8_t start_peripheral_and_app(void)
{
    /*****************************************************************
    * Metrology init
    ******************************************************************/
    EM_CALIBRATION      calib;
    st_em_setting_t     em_hold_setting_value;

    /* Init configuration and restore calibration data from dataflash */
    g_em_startup_diag.config_load_status = config_data_load(&calib, &em_hold_setting_value);

    /* Init load data from storage for ADC driver */
    R_DSADC_SetGain(em_hold_setting_value.regs);
   
    /* Init for EM */
    g_em_startup_diag.em_init_status = EM_Init((EM_PLATFORM_PROPERTY FAR_PTR *)&g_EM_DefaultProperty, &calib);

    /* Starting metrology */
    g_em_startup_diag.em_start_status = EM_Start();
    
    /*****************************************************************
    * Start all peripheral, device, EM Core
    ******************************************************************/
    /* Start other peripherals */
    R_RTC_Set_ConstPeriodInterruptOn(SEC1_2);
    R_RTC_Start();
    R_OSDC_Start();
    
    /* LCD */
    R_LCD_PowerOn(FACTORY_DEFAULT_LCD_CONTRRAST_LEVEL);
    
    /* Key */
//    R_INTC0_Start();                                /* Rly_sens_2 */
    R_INTC1_Start();                                /* Keys Sense */
    R_INTC3_Start();                                /* RF sense */
    R_INTC6_Start();                                /* Cover Open Sense */
//    R_INTRTCIC0_Start();                            /* Case */
//    R_INTRTCIC1_Start();                            /* SW2 */
//    R_INTRTCIC2_Start();                            /* SW1 */

    /* Timer */
    R_TAU0_Channel0_Start();
    
    /* LVD */
    R_LVD_Start_EXLVD();
    R_LVD_Start_VBAT();
    R_LVD_Start_VRTC();
    
    /* UART */
    R_UART0_SetConfig(SAU_STD_LENGTH_8, SAU_STD_PARITY_NONE, SAU_STD_STOPBITS_1);
    R_UART0_SetBaudRate(SAU_STD_BAUD_RATE_9600);
    R_UART0_Start();
    R_UART2_SetConfig(SAU_STD_LENGTH_8, SAU_STD_PARITY_NONE, SAU_STD_STOPBITS_1);
    R_UART2_SetBaudRate(SAU_STD_BAUD_RATE_19200);
    R_UART2_Start();
    
    R_IICA0_Start();
    R_LCD_Start();
    
    /* System checking end, turn off peripherals */
    
    return STARTUP_OK;
}

uint8_t check_rtc_battery_status(void)
{
    uint32_t wIndex1;
    VRTC_DISABLE();
    for (wIndex1 = 0; wIndex1 < 100; wIndex1++)
    {
        EM_MCU_Delay(10);
        if (check_if_RTC_battery_low())
        {
            return 1;
        }
    }
    VRTC_ENABLE();
    return 0;
}

/***********************************************************************************************************************
* Function Name    : uint8_t startup(void)
* Description      : Start-up energy meter
* Arguments        : None
* Functions Called : State selection for startup
* Return Value     : Execution Status
*                  :    STARTUP_OK              Start-up Ok
*                  :    STARTUP_ERROR           Startup error
***********************************************************************************************************************/
uint8_t startup(void)
{
    uint8_t status;
    
    g_reset_flag = RESF;
    hdwinit();
    wake_on_battery_mode();
    status = start_peripheral_and_app();
    #ifndef FACTORY_DEBUG_TEST_CASE_BANK_SWAP
      if(g_u8RTCBatFault)//if(check_rtc_battery_status())
      {
        hdwinit();
        wake_on_battery_mode();
        status = start_peripheral_and_app();
      }
    #endif
    return status;
}

void start_all_LVDs(void)
{
    /* LVD */
    R_LVD_Start_EXLVD();
    R_LVD_Start_VBAT();
    R_LVD_Start_VRTC();
}