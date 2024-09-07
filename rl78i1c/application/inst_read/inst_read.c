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
* File Name    : inst_read.c
* Version      : 1.00
* Device(s)    : RL78/I1C
* Tool-Chain   : CCRL
* H/W Platform : 
* Description  : 
******************************************************************************
* History : DD.MM.YYYY Version Description
******************************************************************************/

/******************************************************************************
Includes   <System Includes> , "Project Includes"
******************************************************************************/
/* Driver */
#include "r_cg_macrodriver.h"   /* MD Macro Driver */
#include "r_cg_wdt.h"           /* MD WDT Driver */

/* Code Standard */
#include "typedef.h"            /* GSCE Standard Typedef */
#include "math.h"

/* EM */
#include "em_type.h"
#include "em_measurement.h"
#include "em_operation.h"
#include "factory_settings.h"

/* Application */
#include "inst_read.h"


/******************************************************************************
Typedef definitions
******************************************************************************/

/******************************************************************************
Macro definitions
******************************************************************************/

/******************************************************************************
Imported global variables and functions (from other files)
******************************************************************************/

/******************************************************************************
Exported global variables and functions (to be accessed by other files)
******************************************************************************/

/******************************************************************************
Private global variables and functions
******************************************************************************/
EM_INST_READ_PARAMS g_inst_read_params;

/******************************************************************************
* Function Name: void INST_READ_RTC_InterruptCallBack(void)
* Description  : Capture instantaneous parameters from metrology
*              : This capture done at every 1s interval
* Arguments    : None
* Return Value : None
******************************************************************************/
void Metrology_Read_CallBack(void)
{
    static uint8_t count = 2;
    float temp_float;
    count++;
    if (count >= 2)
    {
        /* Update the parameter */
        
        /* Selected Line*/
        g_inst_read_params.selected_line = EM_GetRMSLine();
		
        /* VRMS */
        g_inst_read_params.vrms = EM_GetVoltageRMS();
        g_inst_read_params.real_vrms = g_inst_read_params.vrms;
        
        /* IRMS */
        g_inst_read_params.irms_phase = EM_GetCurrentRMS(EM_LINE_PHASE);
        g_inst_read_params.real_irms_phase = g_inst_read_params.irms_phase;
        
        /* Active power */
        g_inst_read_params.active_power = EM_GetActivePower(EM_LINE_PHASE);
        
        /* Reactive power */
        g_inst_read_params.reactive_power = EM_GetReactivePower(EM_LINE_PHASE);
        
        /* Apparent power */
        g_inst_read_params.apparent_power = EM_GetApparentPower(EM_LINE_PHASE);
        
        // filter Current due to shunt noise.s
        if((g_inst_read_params.active_power == 0)
            && (g_inst_read_params.reactive_power == 0)
            && (g_inst_read_params.vrms == 0)
            && (g_inst_read_params.irms_phase < FACTORY_METER_I_STARTING*2))
            {
              g_inst_read_params.irms_phase = 0;
            }
        //
        //re calculation of the power factor based on formula activeP/sqrt(sq(act)+sq(react))
        //metrology gives pf = active/(v*i) results in wrong pf on non sine waves.
        temp_float = sqrt(pow(g_inst_read_params.active_power,2)+pow(g_inst_read_params.reactive_power,2));
        if(temp_float != 0)
        {
          g_inst_read_params.power_factor = fabs(g_inst_read_params.active_power/temp_float);
        }
        else
        {
          g_inst_read_params.power_factor = 0;
        }
        //PF sanity check
        if(g_inst_read_params.power_factor > 1)
        {
          g_inst_read_params.power_factor = 1;
        }
       
        /* Power factor */
        //g_inst_read_params.power_factor = EM_GetPowerFactor(EM_LINE_PHASE);
        g_inst_read_params.power_factor_sign = EM_GetPowerFactorSign(EM_LINE_PHASE);
        if ((g_inst_read_params.power_factor_sign == PF_SIGN_LEAD_C) && (g_inst_read_params.power_factor > 0))
        {
          g_inst_read_params.power_factor = -g_inst_read_params.power_factor;
        }
        if ((g_inst_read_params.power_factor_sign == PF_SIGN_LAG_L) && (g_inst_read_params.power_factor < 0))
        {
          g_inst_read_params.power_factor = -g_inst_read_params.power_factor;
        }
        
        /* Fundamental active power */
        g_inst_read_params.fundamental_power = EM_GetFundamentalActivePower(EM_LINE_PHASE);
        
        /* IRMS */
        g_inst_read_params.irms_neutral = EM_GetCurrentRMS(EM_LINE_NEUTRAL);
        g_inst_read_params.real_irms_neutral = g_inst_read_params.irms_neutral;
        
        /* Active power */
        g_inst_read_params.active_power2 = EM_GetActivePower(EM_LINE_NEUTRAL);
        
        /* Reactive power */
        g_inst_read_params.reactive_power2 = EM_GetReactivePower(EM_LINE_NEUTRAL);
        
        /* Apparent power */
        g_inst_read_params.apparent_power2 = EM_GetApparentPower(EM_LINE_NEUTRAL);
        
        //
        if((g_inst_read_params.active_power2 == 0)
            && (g_inst_read_params.reactive_power2 == 0)
            && (g_inst_read_params.vrms == 0)
            && (g_inst_read_params.irms_neutral < FACTORY_METER_I_STARTING*2))
            {
              g_inst_read_params.irms_neutral = 0;
            }
        //
        
        temp_float = sqrt(pow(g_inst_read_params.active_power2,2)+pow(g_inst_read_params.reactive_power2,2));
        if(temp_float != 0)
        {
          g_inst_read_params.power_factor2 = fabs(g_inst_read_params.active_power2/temp_float);
        }
        else
        {
          g_inst_read_params.power_factor2 = 0;
        }
        if(g_inst_read_params.power_factor2 > 1)
        {
          g_inst_read_params.power_factor2 = 1;
        }
        /* Power factor */
        //g_inst_read_params.power_factor2 = EM_GetPowerFactor(EM_LINE_NEUTRAL);
        g_inst_read_params.power_factor_sign2 = EM_GetPowerFactorSign(EM_LINE_NEUTRAL);
        if ((g_inst_read_params.power_factor_sign2 == PF_SIGN_LEAD_C) && (g_inst_read_params.power_factor2 > 0))
        {
          g_inst_read_params.power_factor2 = -g_inst_read_params.power_factor2;
        }
        if ((g_inst_read_params.power_factor_sign2 == PF_SIGN_LAG_L) && (g_inst_read_params.power_factor2 < 0))
        {
          g_inst_read_params.power_factor2 = -g_inst_read_params.power_factor2;
        }

        /* Fundamental active power */
        g_inst_read_params.fundamental_power2 = EM_GetFundamentalActivePower(EM_LINE_NEUTRAL);
        
        g_inst_read_params.freq = EM_GetLineFrequency();
        
        if(g_inst_read_params.selected_line == EM_LINE_PHASE)
        {
          /* IRMS */
          g_inst_read_params.irms_active = g_inst_read_params.irms_phase;
          g_inst_read_params.real_irms_active = g_inst_read_params.real_irms_phase;
          /* Power factor */
          g_inst_read_params.power_factor_active = g_inst_read_params.power_factor;
          g_inst_read_params.power_factor_sign_active = g_inst_read_params.power_factor_sign;
          
          /* Active power */
          g_inst_read_params.active_power_active = g_inst_read_params.active_power;
          
          /* Reactive power */
          g_inst_read_params.reactive_power_active = g_inst_read_params.reactive_power;
          
          /* Apparent power */
          g_inst_read_params.apparent_power_active = g_inst_read_params.apparent_power;
          
          /* Fundamental active power */
          g_inst_read_params.fundamental_power_active = g_inst_read_params.fundamental_power;
        }
        else
        {
          /* IRMS */
          g_inst_read_params.irms_active = g_inst_read_params.irms_neutral;
          g_inst_read_params.real_irms_active = g_inst_read_params.real_irms_neutral;
          
          /* Power factor */
          g_inst_read_params.power_factor_active = g_inst_read_params.power_factor2;
          g_inst_read_params.power_factor_sign_active = g_inst_read_params.power_factor_sign2;
          
          /* Active power */
          g_inst_read_params.active_power_active = g_inst_read_params.active_power2;
          
          /* Reactive power */
          g_inst_read_params.reactive_power_active = g_inst_read_params.reactive_power2;
          
          /* Apparent power */
          g_inst_read_params.apparent_power_active = g_inst_read_params.apparent_power2;
          
          /* Fundamental active power */
          g_inst_read_params.fundamental_power_active = g_inst_read_params.fundamental_power2;
        }
        /* Active power */
        g_inst_read_params.real_active_power_active = g_inst_read_params.active_power_active;
        g_inst_read_params.real_power_factor_active = g_inst_read_params.power_factor_active;

        if(g_inst_read_params.start_dummy_power)
        {
          if(g_inst_read_params.magnet_tamper)
          {
            g_inst_read_params.dummy_current = FACTORY_METER_I_MAX;
          }
          else
          {
            g_inst_read_params.dummy_current = g_inst_read_params.irms_active;
          }
          
          g_inst_read_params.vrms = (float)FACTORY_METER_V_REF;
          g_inst_read_params.irms_phase = g_inst_read_params.dummy_current;
          g_inst_read_params.irms_neutral = g_inst_read_params.dummy_current;
          g_inst_read_params.irms_active = g_inst_read_params.dummy_current;
          g_inst_read_params.power_factor_active = 1.0;
          g_inst_read_params.power_factor = 1.0;
          g_inst_read_params.power_factor2 = 1.0;
          g_inst_read_params.freq = 50;
          g_inst_read_params.active_power_active = g_inst_read_params.vrms*g_inst_read_params.dummy_current;
          g_inst_read_params.reactive_power_active = 0;
          g_inst_read_params.apparent_power_active = g_inst_read_params.active_power_active;
          g_inst_read_params.fundamental_power_active = g_inst_read_params.active_power_active;
          EM_SetEnergyAccumulationMode(0);
          EM_SetEnergyAccumulationPower(g_inst_read_params.active_power_active, 
                                        g_inst_read_params.reactive_power_active, 
                                        g_inst_read_params.apparent_power_active);
                                        
        }
        else if (EM_GetEnergyAccumulationMode() == 0)
        {
          EM_SetEnergyAccumulationMode(3);
        }

        if ((g_inst_read_params.vrms < 10.0) && (g_inst_read_params.irms_active < 0.020))
        {
            g_inst_read_params.freq = 0;        //REVIEW: Library showing 50 Hz in case of battery also
        }
        
//        {
//            EM_ENERGY_COUNTER em_energy_counter;
//            EM_ENERGY_VALUE em_energy_value;
            
//            /* Critical section, energy update in DSAD */
//            DI();
//	          EM_GetEnergyCounter(&em_energy_counter);
//            EI();
//            EM_EnergyCounterToEnergyValue(&em_energy_counter, &em_energy_value);
//	          g_inst_read_params.active_energy_total_import = em_energy_value.active_imp;
//	          g_inst_read_params.active_energy_total_export = em_energy_value.active_exp;
//	          g_inst_read_params.reactive_energy_lag_total_import = em_energy_value.reactive_ind_imp;
//	          g_inst_read_params.reactive_energy_lag_total_export = em_energy_value.reactive_ind_exp;
//	          g_inst_read_params.reactive_energy_lead_total_import = em_energy_value.reactive_cap_imp;
//	          g_inst_read_params.reactive_energy_lead_total_export = em_energy_value.reactive_cap_exp;
//	          g_inst_read_params.apparent_energy_total_import = em_energy_value.apparent_imp;
//	          g_inst_read_params.apparent_energy_total_export = em_energy_value.apparent_exp;
//        }

        count = 0;
    }
}