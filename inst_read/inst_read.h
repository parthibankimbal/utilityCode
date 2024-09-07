/******************************************************************************
  Copyright (C) 2011 Renesas Electronics Corporation, All Rights Reserved.
*******************************************************************************
* File Name    : dataflash.h
* Version      : 1.00
* Description  : Data Flash Application Layer APIs
******************************************************************************
* History : DD.MM.YYYY Version Description
******************************************************************************/

#ifndef _INST_READ_H
#define _INST_READ_H

/******************************************************************************
Includes   <System Includes> , "Project Includes"
******************************************************************************/
#include "typedef.h"        /* GSCE Standard Typedef */
#include "em_constraint.h"
#include "em_energies.h"

#define get_energy(x)                                      (uint64_t)(myEnergies.total.energy[x]*100 + myEnergies.remainder.energy[x]*100)

#define get_ph_voltage()                                   g_inst_read_params.vrms
#define get_real_ph_voltage()                              g_inst_read_params.real_vrms
#define get_signed_ph_current()                            g_inst_read_params.irms_phase
#define get_signed_neu_current()                           g_inst_read_params.irms_neutral
#define get_signed_act_current()                           g_inst_read_params.irms_active
#define get_real_signed_ph_current()                       g_inst_read_params.real_irms_phase
#define get_real_signed_neu_current()                      g_inst_read_params.real_irms_neutral
#define get_real_signed_act_current()                      g_inst_read_params.real_irms_active
#define get_signed_pf()                                    g_inst_read_params.power_factor_active
#define get_real_signed_pf()                               g_inst_read_params.real_power_factor_active
#define get_freq()                                         g_inst_read_params.freq
#define get_tot_kva()                                      g_inst_read_params.apparent_power_active
#define get_signed_tot_kw()                                g_inst_read_params.active_power_active
#define get_signed_tot_kvar()                              g_inst_read_params.reactive_power_active
#define get_real_signed_tot_kw()                           g_inst_read_params.real_active_power_active
#define is_reverse()                                       (g_inst_read_params.real_active_power_active < 0 ? 1 : 0)
#define INST_DUMMY_CURRENT                                 g_inst_read_params.dummy_current
#define INST_DUMMY_POWER_START                             g_inst_read_params.start_dummy_power
#define INST_SET_MAGNET_TAMPER                             g_inst_read_params.magnet_tamper
/******************************************************************************
Typedef definitions
******************************************************************************/
typedef struct tagEMInstantRead
{
    float32_t     vrms;
    float32_t     real_vrms;

    float32_t     irms_phase;
    float32_t     real_irms_phase;
    float32_t     power_factor;
    EM_PF_SIGN    power_factor_sign;
    float32_t     active_power;
    float32_t     reactive_power;
    float32_t     apparent_power;
    float32_t     fundamental_power;
    
    float32_t     irms_neutral;
    float32_t     real_irms_neutral;
    float32_t     power_factor2;
    EM_PF_SIGN    power_factor_sign2;
    float32_t     active_power2;
    float32_t     reactive_power2;
    float32_t     apparent_power2;
    float32_t     fundamental_power2;
    
    float32_t     irms_active;
    float32_t     real_irms_active;
    float32_t     power_factor_active;
    float32_t     real_power_factor_active;
    EM_PF_SIGN    power_factor_sign_active;
    float32_t     active_power_active;
    float32_t     real_active_power_active;
    float32_t     reactive_power_active;
    float32_t     apparent_power_active;
    float32_t     fundamental_power_active;

//    float32_t     active_energy_total_import;
//    float32_t     active_energy_total_export;
//    float32_t     reactive_energy_lag_total_import;
//    float32_t     reactive_energy_lag_total_export;
//    float32_t     reactive_energy_lead_total_import;
//    float32_t     reactive_energy_lead_total_export;
//    float32_t     apparent_energy_total_import;
//    float32_t     apparent_energy_total_export;

    float32_t     freq;
    float32_t     bypass_current_irms;
    EM_LINE       selected_line;
    uint8_t       acc_mode;
    uint8_t       start_dummy_power;
    uint8_t       magnet_tamper;
    float32_t     dummy_current;
    
	
} EM_INST_READ_PARAMS;

/******************************************************************************
Macro definitions
******************************************************************************/

/******************************************************************************
Variable Externs
******************************************************************************/
extern EM_INST_READ_PARAMS g_inst_read_params;

/******************************************************************************
Functions Prototypes
******************************************************************************/
void Metrology_Read_CallBack(void);

#endif /* _INST_READ_H */
