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
* File Name    : config_storage.c
* Version      : 1.00
* Device(s)    : RL78/I1C
* Tool-Chain   : CCRL
* H/W Platform : RL78/I1C Energy Meter Platform
* Description  : Storage Source File
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
***********************************************************************************************************************/
/* Driver */
#include "r_cg_macrodriver.h"

/* Library */
#include <string.h>             /* Compiler standard library */

/* MW */
#include "em_core.h"            /* EM Core */
#include "dataflash.h"          /* Logical Volume Manager */

/* WRP */
#include "wrp_em_sw_config.h"   /* EM Wrapper SW Config */
#include "wrp_em_adc.h"

/* Application */
#include "platform.h"
#include "config_format.h"      /* Storage Format Header */
#include "config_storage.h"     /* Storage Header File */

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/
/* Storage Module State */
typedef enum tagStorageState
{
    CONFIG_STATE_UNINITIALIZED = 0,        /* Uninitialized */
    CONFIG_STATE_INITIALIZED               /* Initialized */
    
} CONFIG_STATE;

typedef struct tagConfigStorageCrcData
{
    EM_CALIBRATION  * p_calibration;
    uint8_t         * p_driver_adc_gain0_list;
    uint8_t         * p_driver_adc_gain1_list;
    
} config_crc_data;
/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
#define CONFIG_STORAGE_INTERVAL_COUNT           (60)

/***********************************************************************************************************************
Imported global variables and functions (from other files)
***********************************************************************************************************************/

/***********************************************************************************************************************
Exported global variables and functions (to be accessed by other files)
***********************************************************************************************************************/
/* Configuration for the dataflash CRC checking area */
const uint16_t g_dataflash_crc_block_addr = CONFIG_STORAGE_CALIB_ADDR;
const uint16_t g_dataflash_crc_block_size = CONFIG_STORAGE_CALIB_SIZE;

/* Dataflash format flag, which may use in user app */
uint8_t g_DATAFLASH_FormatTrigger =  FALSE;

/***********************************************************************************************************************
Private global variables and functions
***********************************************************************************************************************/
/* Variables */

#ifndef __DEBUG
static
#endif
CONFIG_STATE                g_config_storage_state = CONFIG_STATE_UNINITIALIZED;    /* Storage State */

/* CRC interval checking calibration data in RAM */
#ifndef __DEBUG
static
#endif
uint8_t                     g_config_storage_interval_flag;                         /* Storage interval checking CRC for RAM */

#ifndef __DEBUG
static
#endif
uint8_t                     g_config_disable_CRC_checking;                         /* Enable CRC checking */

/* Sub-functions (internal used for module) */
CFS_FUNC static uint8_t CONFIG_ReadStorageHeader(void);   
CFS_FUNC static uint8_t CONFIG_UpdateStorageHeader(void);

/***********************************************************************************************************************
* Function Name    : static uint8_t CONFIG_ReadStorageHeader(void)
* Description      : Read & Check Storage Header code of config(data flash)
* Arguments        : None
* Functions Called : DATAFLASH_Read()
* Return Value     : Execution Status
*                  :    CONFIG_OK              Normal end, Storage is formatted
*                  :    CONFIG_ERROR           Storage Header error
*                  :    CONFIG_NOT_FORMATTED   Storage device is not formatted
***********************************************************************************************************************/
CFS_FUNC static uint8_t CONFIG_ReadStorageHeader(void)
{
    uint32_t config_storage_header;
    /* Read Storage header OK? */
    if (DATAFLASH_Read(CONFIG_STORAGE_HEADER_HEADER_CODE_ADDR,
                       (uint8_t *)&config_storage_header,
                       CONFIG_STORAGE_HEADER_HEADER_CODE_SIZE) != DATAFLASH_OK)
    {
        return CONFIG_ERROR;   /* Read error */
    }


    /* Is the usage = sum of all item? */
    if (config_storage_header != CONFIG_STORAGE_HEADER_CODE_MASK)
    {
        return CONFIG_NOT_FORMATTED;   /* Not formatted */ 
    }

    /* Success */
    return CONFIG_OK;
}

/***********************************************************************************************************************
* Function Name    : static uint8_t CONFIG_UpdateStorageHeader(void)
* Description      : header code to Storage Header
* Arguments        : None
* Functions Called : DATAFLASH_Write()
* Return Value     : Execution Status
*                  :    CONFIG_OK      Normal end, update success
*                  :    CONFIG_ERROR   Write error
***********************************************************************************************************************/
CFS_FUNC static uint8_t CONFIG_UpdateStorageHeader(void)
{
    uint32_t config_storage_header = CONFIG_STORAGE_HEADER_CODE_MASK;

    /* Is write Storage OK? */
    if (DATAFLASH_Write(CONFIG_STORAGE_HEADER_HEADER_CODE_ADDR,
                        (uint8_t *)&config_storage_header,
                        CONFIG_STORAGE_HEADER_HEADER_CODE_SIZE) != DATAFLASH_OK)
    {
        return CONFIG_ERROR;   /* Write error */
    }
    
    /* Success */
    return CONFIG_OK;
}

/***********************************************************************************************************************
* Function Name    : static uint8_t CONFIG_CalculateCRC16(void)
* Description      : Calculate the CRC16 based on required input data
* Arguments        : config_crc_data * p_crc_data: pointer to structure of data to calculate CRC
* Functions Called : TBD
* Return Value     : uint16_t: Calculated CRC value
***********************************************************************************************************************/
#ifndef __DEBUG
static 
#endif
CFS_FUNC uint16_t CONFIG_CalculateCRC16(config_crc_data * p_crc_data)
{
    uint16_t crc_value;
    
    if (p_crc_data != NULL)
    {
        if ( (p_crc_data->p_calibration != NULL) &&
            (p_crc_data->p_driver_adc_gain0_list != NULL) &&
            (p_crc_data->p_driver_adc_gain1_list != NULL) &&
            (1)
        )
        {
            
            /* Clear old CRC to start calc. CRC for calib */
            R_CRC_Clear();
            
            /* Calibration data (no pointer) */
            R_CRC_Calculate(
                (uint8_t *)p_crc_data->p_calibration,
                CONFIG_STORAGE_CALIB_NO_ARRAY_SIZE
            );
            
            /* SW Phase */
            R_CRC_Calculate(
                (uint8_t *)p_crc_data->p_calibration->sw_phase_correction.i1_phase_degrees,
                CONFIG_STORAGE_CALIB_SW_DEGREE_I1_ARRAY_SIZE
            );
            R_CRC_Calculate(
                (uint8_t *)p_crc_data->p_calibration->sw_phase_correction.i2_phase_degrees,
                CONFIG_STORAGE_CALIB_SW_DEGREE_I2_ARRAY_SIZE
            );
            
            /* SW Gain */
            
            R_CRC_Calculate(
              (uint8_t *)p_crc_data->p_calibration->sw_gain.i1_gain_values,
              CONFIG_STORAGE_CALIB_SW_GAIN_I1_ARRAY_SIZE
            );
            R_CRC_Calculate(
              (uint8_t *)p_crc_data->p_calibration->sw_gain.i2_gain_values,
              CONFIG_STORAGE_CALIB_SW_GAIN_I2_ARRAY_SIZE
            );
            
            /* ADC Gain */
            
            R_CRC_Calculate(
                (uint8_t *)p_crc_data->p_driver_adc_gain0_list,
                CONFIG_STORAGE_CALIB_DRIVER_ADC_GAIN0_SIZE
            );
            R_CRC_Calculate(
                (uint8_t *)p_crc_data->p_driver_adc_gain1_list,
                CONFIG_STORAGE_CALIB_DRIVER_ADC_GAIN1_SIZE
            );
            
            crc_value = R_CRC_GetResult();
        }
    }
    
    return crc_value;
}

/***********************************************************************************************************************
* Function Name    : uint8_t CONFIG_Init(uint8_t is_checking)
* Description      : Storage Initialization
* Arguments        : None
* Functions Called : DATAFLASH_Init()
*                  : DATAFLASH_VerifyStatus()
*                  : CONFIG_ReadStorageHeader()
* Return Value     : Execution Status
*                  :    CONFIG_OK               Normal end
*                  :    CONFIG_ERROR            Error occurred
*                  :                            Read/Write error or data validity error
*                  :    CONFIG_NOT_FORMATTED    No valid bank found
***********************************************************************************************************************/
CFS_FUNC uint8_t CONFIG_Init(uint8_t is_checking)
{
    uint8_t     status;
    
    g_config_storage_state = CONFIG_STATE_UNINITIALIZED;   /* Uninitialized */
    
    if (is_checking)
    {
        /* Driver Initialization */
        if (DATAFLASH_Init() != DATAFLASH_OK)
        {
            return CONFIG_ERROR;
        }
    }
    
    /* Successful, at here STORAGE module is initial success */
    g_config_storage_state = CONFIG_STATE_INITIALIZED;     /* Initialized */
    
    /* Enable CRC checking */
    g_config_disable_CRC_checking = 0;

    
    if (is_checking)
    {
        /* Check data format */
        status = DATAFLASH_VerifyStatus();
        if (status == DATAFLASH_ERROR)
        {
            return CONFIG_ERROR;                            /* Error occurred when read Storage Header */
        }
        else if (status == DATAFLASH_NOT_FORMAT)
        {
            return CONFIG_NOT_FORMATTED;                    /* Storage data is not formatted: both bank invalid */
        }
    }
    
    /* Update and check Storage format */
    status = CONFIG_ReadStorageHeader();
    if (status == CONFIG_ERROR)
    {
        return CONFIG_ERROR;                            /* Error occurred when read Storage Header */
    }
    else if (status == CONFIG_NOT_FORMATTED)
    {
        /* Raise flag format of data flash which will extern and may use in app user */
        g_DATAFLASH_FormatTrigger = TRUE;
        return CONFIG_NOT_FORMATTED;                    /* Storage data is not formatted: wrong storage header content */
    }
    
    return CONFIG_OK;                                   /* OK, And storage is formatted */
}
/***********************************************************************************************************************
* Function Name    : uint8_t CONFIG_BackupCalib(EM_CALIBRATION * calib, dsad_reg_setting_t regs)
* Description      : Storage Backup (from EM Core to Storage)
* Note             : When CONFIG_ERROR is returned, maybe some data on selected item
*                  : (of [selection] parameter) have been stored to Storage
* Arguments        : EM_CALIBRATION calib - pointer for calibration
*                    dsad_reg_setting_t regs - value of adc register to back up to storage
* Return Value     : Execution Status
*                  :    CONFIG_OK      Backup successfull
*                  :    CONFIG_ERROR   Storage device is not initialized or not formatted
*                  :                    Or, error occurred when write Storage
*                  :                    Or, selection = 0 (CONFIG_ITEM_NONE)
***********************************************************************************************************************/
CFS_FUNC static uint8_t CONFIG_BackupCalib(EM_CALIBRATION calib, dsad_reg_setting_t regs)
{
    uint8_t i;
    uint8_t driver_adc_gain0;
    uint8_t driver_adc_gain1;
    int16_t driver_timer_offset;
    float32_t i1_phase_degrees_temp[EM_GAIN_PHASE_NUM_LEVEL_MAX];
    float32_t i2_phase_degrees_temp[EM_GAIN_PHASE_NUM_LEVEL_MAX];
    float32_t i1_gain_values_temp[EM_GAIN_PHASE_NUM_LEVEL_MAX];
    float32_t i2_gain_values_temp[EM_GAIN_PHASE_NUM_LEVEL_MAX];

    /* Pass data to temporary for phase degree */
    for ( i = 0; i < EM_GAIN_PHASE_NUM_LEVEL_MAX; i++ )
    {
        i1_phase_degrees_temp[i] = calib.sw_phase_correction.i1_phase_degrees[i];
        i2_phase_degrees_temp[i] = calib.sw_phase_correction.i2_phase_degrees[i];

        i1_gain_values_temp[i]   = calib.sw_gain.i1_gain_values[i];
        i2_gain_values_temp[i]   = calib.sw_gain.i2_gain_values[i];
    }

    /* Clear the temp driver gain list */
    memset(&driver_adc_gain0, 0, sizeof(driver_adc_gain0));
    memset(&driver_adc_gain1, 0, sizeof(driver_adc_gain1));

    /* Store calib to Storage */
    if (DATAFLASH_Write(CONFIG_STORAGE_CALIB_ADDR,
                        (uint8_t *)&calib,
                        CONFIG_STORAGE_CALIB_NO_ARRAY_SIZE) != DATAFLASH_OK)
    {
        return CONFIG_ERROR; /* Write error */
    }
    R_WDT_Restart();
    /* 
         * Store SW Phase Correction array 
        */
    /* I1 */
    if (DATAFLASH_Write(CONFIG_STORAGE_CALIB_SW_DEGREE_I1_ARRAY_ADDR,
                        (uint8_t *)&i1_phase_degrees_temp,
                        CONFIG_STORAGE_CALIB_SW_DEGREE_I1_ARRAY_SIZE) != DATAFLASH_OK)
    {
        return CONFIG_ERROR; /* Write error */
    }

    /* I2 */
    if (DATAFLASH_Write(CONFIG_STORAGE_CALIB_SW_DEGREE_I2_ARRAY_ADDR,
                        (uint8_t *)&i2_phase_degrees_temp,
                        CONFIG_STORAGE_CALIB_SW_DEGREE_I2_ARRAY_SIZE) != DATAFLASH_OK)
    {
        return CONFIG_ERROR; /* Write error */
    }

    R_WDT_Restart();

    /* 
    * Store SW Gain I Array 
    */
    /* I1 */
    if (DATAFLASH_Write(CONFIG_STORAGE_CALIB_SW_GAIN_I1_ARRAY_ADDR,
                        (uint8_t *)i1_gain_values_temp,
                        CONFIG_STORAGE_CALIB_SW_GAIN_I1_ARRAY_SIZE) != DATAFLASH_OK)
    {
        return CONFIG_ERROR; /* Write error */
    }
    /* I2 */
    if (DATAFLASH_Write(CONFIG_STORAGE_CALIB_SW_GAIN_I2_ARRAY_ADDR,
                        (uint8_t *)i2_gain_values_temp,
                        CONFIG_STORAGE_CALIB_SW_GAIN_I2_ARRAY_SIZE) != DATAFLASH_OK)
    {
        return CONFIG_ERROR; /* Write error */
    }
    R_WDT_Restart();
    /* 
    * Store Driver ADC Gain Array 
    */

    /* When using dual gain, ignore current ADC Gain, 
    * Using default value only (same as when backup, GAIN_X1)
    */
#if (EM_SW_PROPERTY_ADC_GAIN_PHASE_NUMBER_LEVEL != 1)
    R_DSADC_ClearGainSetting(&regs, EM_ADC_DRIVER_CHANNEL_PHASE);
#endif /* EM_SW_PROPERTY_ADC_GAIN_PHASE_NUMBER_LEVEL != 1 */

#if (EM_SW_PROPERTY_ADC_GAIN_NEUTRAL_NUMBER_LEVEL != 1)
    R_DSADC_ClearGainSetting(&regs, EM_ADC_DRIVER_CHANNEL_NEUTRAL);
#endif /* EM_SW_PROPERTY_ADC_GAIN_NEUTRAL_NUMBER_LEVEL != 1 */

    driver_adc_gain0 = regs.gain0;
    driver_adc_gain1 = regs.gain1;

    if (DATAFLASH_Write(CONFIG_STORAGE_CALIB_DRIVER_ADC_GAIN0_ADDR,
                        (uint8_t *)&driver_adc_gain0,
                        CONFIG_STORAGE_CALIB_DRIVER_ADC_GAIN0_SIZE) != DATAFLASH_OK)
    {
        return CONFIG_ERROR; /* Write error */
    }
    if (DATAFLASH_Write(CONFIG_STORAGE_CALIB_DRIVER_ADC_GAIN1_ADDR,
                        (uint8_t *)&driver_adc_gain1,
                        CONFIG_STORAGE_CALIB_DRIVER_ADC_GAIN1_SIZE) != DATAFLASH_OK)
    {
        return CONFIG_ERROR; /* Write error */
    }
    R_WDT_Restart();

    /* Success */
    return CONFIG_OK;
}
/***********************************************************************************************************************
* Function Name    : convert_default_gain_reg_value
* Description      : to convert default value to register value
* Arguments        : dsad_channel_t channel - channel of adc gain
*                    dsad_gain_t gain - gain set value
*                    uint8_t * preg0 - pointer of variable that contain value of gain 0
*                    uint8_t * preg1 - pointer of variable that contain value of gain 1
* Return Value     : void
***********************************************************************************************************************/
static void convert_default_gain_reg_value(dsad_channel_t channel, dsad_gain_t gain, uint8_t * preg0, uint8_t * preg1)
{
        /* Set the gain */
        switch(channel)
        {
            case DSADCHANNEL0:  /* Used as Current channel */
                *preg0 &= 0xF0;
                *preg0 |= gain;
                break;
            case DSADCHANNEL1:  /* Used as Voltage channel */
                *preg0 &= 0x0F;
                *preg0 |= (uint8_t)(gain << 4);
                break;
            case DSADCHANNEL2:  /* Used as Current channel */
                *preg1 &= 0xF0;
                *preg1 |= gain;
                break;
            case DSADCHANNEL3:  /* Used as Current channel */
                *preg1 &= 0x0F;
                *preg1 |= (uint8_t)(gain << 4);
                break;
            default:
            break;
        }
}

/***********************************************************************************************************************
* Function Name    : uint8_t CONFIG_Format(void)
* Description      : Storage Format Storage Device to store data
* Arguments        : None
* Functions Called : DATAFLASH_BankErase()
                   : DATAFLASH_Write()
* Return Value     : Execution Status
*                  :    CONFIG_OK          Formatted successfull, device is ready to store data
*                  :    CONFIG_ERROR       Storage device is not initialized
*                  :                        Or, error occurred when write data to Storage
***********************************************************************************************************************/
CFS_FUNC uint8_t CONFIG_Format(void)
{
    uint16_t    count;
    uint32_t    config_addr;
    dsad_reg_setting_t  regs;

    memset(&regs, 0, sizeof(dsad_reg_setting_t));

    /* Check storage state, only call when initial successfull */
    if (g_config_storage_state != CONFIG_STATE_INITIALIZED)
    {
        return CONFIG_ERROR;
    }
    
    /* Format the dataflash */
    DATAFLASH_BankErase(0);
    DATAFLASH_BankErase(1);

    /* Write initial Storage Header */
    if (CONFIG_UpdateStorageHeader() != CONFIG_OK)
    {
        return CONFIG_ERROR;
    }

    /* Convert default gain value for phase */
#if(EM_ADC_CURRENT_DRIVER_CHANNEL_OPTIMIZATION_PHASE == 0 || EM_ADC_CURRENT_DRIVER_CHANNEL_OPTIMIZATION_PHASE == 1)
    convert_default_gain_reg_value(EM_ADC_DRIVER_CHANNEL_PHASE, R_DSADC_GetGainEnumValue(EM_CALIB_DEFAULT_PHASE_GAIN), &regs.gain0, &regs.gain1);
#endif

    /* Convert default gain value for neutral */
#if(EM_ADC_CURRENT_DRIVER_CHANNEL_OPTIMIZATION_NEUTRAL == 0 || EM_ADC_CURRENT_DRIVER_CHANNEL_OPTIMIZATION_NEUTRAL == 1)
    convert_default_gain_reg_value(EM_ADC_DRIVER_CHANNEL_NEUTRAL, R_DSADC_GetGainEnumValue(EM_CALIB_DEFAULT_NEUTRAL_GAIN), &regs.gain0, &regs.gain1);
#endif

    /* Calibration default */
    if (CONFIG_BackupCalib(g_EM_DefaultCalibration, regs) != CONFIG_OK)
    {
        return CONFIG_ERROR; /* Storage Header error (not formatted) */
    }

    return CONFIG_OK; /* Formatted */
}

/***********************************************************************************************************************
* Function Name    : uint8_t CONFIG_Backup(uint8_t selection)
* Description      : Storage Backup (from EM Core to Storage)
* Note             : When CONFIG_ERROR is returned, maybe some data on selected item
*                  : (of [selection] parameter) have been stored to Storage
* Arguments        : uint8_t selection: Backup item selection
*                  :      CONFIG_ITEM_CONFIG       Select EM Configuration
*                  :      CONFIG_ITEM_CALIB        Select EM Calibration
*                  :      CONFIG_ITEM_ALL          Select all above items
* Functions Called : CONFIG_ReadStorageHeader()
*                  : DATAFLASH_Write()
*                  : EM_GetConfig()
*                  : EM_GetCalibInfo()
*                  : CONFIG_UpdateStorageHeader()
* Return Value     : Execution Status
*                  :    CONFIG_OK      Backup successfull
*                  :    CONFIG_ERROR   Storage device is not initialized or not formatted
*                  :                    Or, error occurred when write Storage
*                  :                    Or, selection = 0 (CONFIG_ITEM_NONE)
***********************************************************************************************************************/
CFS_FUNC uint8_t CONFIG_Backup(uint8_t selection)
{
    uint8_t                 status;
    
    /* Check parameter */
    if (selection == CONFIG_ITEM_NONE)
    {
        return CONFIG_ERROR;   /* Params error */
    }
    
    /* Check Storage State */
    if (g_config_storage_state != CONFIG_STATE_INITIALIZED)
    {
        return CONFIG_ERROR;   /* Device is not initialized */
    }
    
    /* Get Storage Header */
    if (CONFIG_ReadStorageHeader() != CONFIG_OK)
    {
        return CONFIG_ERROR;   /* Storage Header error (not formatted) */
    }

    /* Is CONFIG_ITEM_CALIB selected? */
    if ((selection & CONFIG_ITEM_CALIB) != 0)
    {
        EM_CALIBRATION      calib;
        dsad_reg_setting_t  regs;
        
        /* Get the gain first */
        R_DSADC_GetGainAndPhase(&regs);
        /* Get EM Core calib */
        calib = EM_GetCalibInfo();

        if (CONFIG_BackupCalib(calib, regs) != CONFIG_OK)
        {
            return CONFIG_ERROR; /* Storage Header error (not formatted) */
        }

    }

    /* Update Storage Header */
    if (CONFIG_UpdateStorageHeader() != CONFIG_OK)
    {
        return CONFIG_ERROR;   /* Write error */
    }
    
    /* Success */
    return CONFIG_OK;
}

/***********************************************************************************************************************
* Function Name    : uint8_t CONFIG_Restore(uint8_t selection)
* Description      : Storage Restore (from Storage to EM Core)
* Arguments        : uint8_t selection: Restore item selection
*                  :      CONFIG_ITEM_CONFIG       Select EM Configuration
*                  :      CONFIG_ITEM_CALIB        Select EM Calibration
*                  :      CONFIG_ITEM_SYS_STATE    Select EM System State
*                  :      CONFIG_ITEM_RTC_TIME     Select RTC Time
*                  :      CONFIG_ITEM_ALL          Select all above items
* Functions Called : CONFIG_ReadStorageHeader()
*                  : DATAFLASH_Read()
*                  : EM_SetConfig()
*                  : EM_SetCalibInfo()
*                  : EM_SetState()
*                  : RTC_CounterSet()
* Return Value     : Execution Status
*                  :    CONFIG_OK                  Restore successfull
*                  :    CONFIG_ERROR_DATA_CORRUPT  Related data on Storage of selected item is corrupt
*                  :    CONFIG_ERROR               Storage device is not initialized or not formatted
*                  :                                Or, error occurred when read Storage,
*                  :                                Or, selection = 0 (CONFIG_ITEM_NONE)
***********************************************************************************************************************/
CFS_FUNC uint8_t CONFIG_Restore(uint8_t selection)
{
    uint8_t                 status;
    
    /* Check parameter */
    if (selection == CONFIG_ITEM_NONE)
    {
        return CONFIG_ERROR;   /* Params error */
    }
    
    /* Check Storage State */
    if (g_config_storage_state != CONFIG_STATE_INITIALIZED)
    {
        return CONFIG_ERROR;   /* Device is not initialized */
    }
    
    /* Get Storage Header */
    if (CONFIG_ReadStorageHeader() != CONFIG_OK)
    {
        return CONFIG_ERROR;   /* Storage Header error */
    }
    

    /* Is CONFIG_ITEM_CALIB selected? */
    if ((selection & CONFIG_ITEM_CALIB) != 0)
    {        

        EM_CALIBRATION calib;
        st_em_setting_t em_hold_setting_value;
        /* Assign pointer for phase degree in calib holder value */
        calib.sw_phase_correction.i1_phase_degrees = em_hold_setting_value.degree_list_i1;
        calib.sw_phase_correction.i2_phase_degrees = em_hold_setting_value.degree_list_i2;

        /* Assign pointer for gain in calib to holder value */
        calib.sw_gain.i1_gain_values = em_hold_setting_value.gain_list_i1;
        calib.sw_gain.i2_gain_values = em_hold_setting_value.gain_list_i2;
        /* Get calibration from storage */
        if (CONFIG_LoadEMCalib(&calib, &em_hold_setting_value.regs) != EM_OK)
        {
            return CONFIG_ERROR;
        }
        
        /* Init load data from storage for ADC driver */
        R_DSADC_SetGain(em_hold_setting_value.regs);

        /* Set to EM Core */
        if (EM_SetCalibInfo(&calib) != EM_OK)
        {
            return CONFIG_ERROR_DATA_CORRUPT; /* Data corrupt */
        }
    }
    else
    {
        return CONFIG_ERROR; /* Header not match */
    }

    /* Success */
    return CONFIG_OK;
}

/***********************************************************************************************************************
* Function Name    : uint8_t CONFIG_LoadEMCalib(EM_CALIBRATION * p_calib, dsad_reg_setting_t * p_regs)
* Description      : load data for calibration configuration from storage
* Arguments        : EM_CALIBRATION * p_calib - pointer of calibration to get data from storage
*                    dsad_reg_setting_t * p_regs - pointer to get value of adc gain from storage
* Return Value     : Execution Status
*                  :    CONFIG_OK                  Restore successfull
*                  :    CONFIG_ERROR_DATA_CORRUPT  Related data on Storage of selected item is corrupt
*                  :    CONFIG_ERROR               Storage device is not initialized or not formatted
*                  :                                Or, error occurred when read Storage,
*                  :                                Or, selection = 0 (CONFIG_ITEM_NONE)
***********************************************************************************************************************/
CFS_FUNC uint8_t CONFIG_LoadEMCalib(EM_CALIBRATION *p_calib, dsad_reg_setting_t * p_regs)
{
    uint8_t i;

    /* Clear the temp driver gain list */
    for ( i = 0; i < EM_GAIN_PHASE_NUM_LEVEL_MAX; i++ )
    {
        p_calib->sw_phase_correction.i1_phase_degrees[i] = 0;
        p_calib->sw_phase_correction.i2_phase_degrees[i] = 0;

        p_calib->sw_gain.i1_gain_values[i] = 0;
        p_calib->sw_gain.i2_gain_values[i] = 0;
    }

    memset(&p_regs->gain0, 0, sizeof(uint8_t));
    memset(&p_regs->gain1, 0, sizeof(uint8_t));

    /* Get calib from Storage, accept sw phase & gain value list pointer */
    if (DATAFLASH_Read(CONFIG_STORAGE_CALIB_ADDR,
                       (uint8_t *)p_calib,
                       CONFIG_STORAGE_CALIB_NO_ARRAY_SIZE) != DATAFLASH_OK)
    {
        return CONFIG_ERROR; /* Read error */
    }

    /* 
    * Get SW Phase Correction array 
    */
    /* I1 */
    if (DATAFLASH_Read(CONFIG_STORAGE_CALIB_SW_DEGREE_I1_ARRAY_ADDR,
                       (uint8_t *)p_calib->sw_phase_correction.i1_phase_degrees,
                       CONFIG_STORAGE_CALIB_SW_DEGREE_I1_ARRAY_SIZE) != DATAFLASH_OK)
    {
        return CONFIG_ERROR; /* Read error */
    }

    /* I2 */
    if (DATAFLASH_Read(CONFIG_STORAGE_CALIB_SW_DEGREE_I2_ARRAY_ADDR,
                       (uint8_t *)p_calib->sw_phase_correction.i2_phase_degrees,
                       CONFIG_STORAGE_CALIB_SW_DEGREE_I2_ARRAY_SIZE) != DATAFLASH_OK)
    {
        return CONFIG_ERROR; /* Read error */
    }

    /* 
    * Get SW Gain array 
    */
    /* I1 */
    if (DATAFLASH_Read(CONFIG_STORAGE_CALIB_SW_GAIN_I1_ARRAY_ADDR,
                       (uint8_t *)p_calib->sw_gain.i1_gain_values,
                       CONFIG_STORAGE_CALIB_SW_GAIN_I1_ARRAY_SIZE) != DATAFLASH_OK)
    {
        return CONFIG_ERROR; /* Read error */
    }


    /* I2 */
    if (DATAFLASH_Read(CONFIG_STORAGE_CALIB_SW_GAIN_I2_ARRAY_ADDR,
                       (uint8_t *)p_calib->sw_gain.i2_gain_values,
                       CONFIG_STORAGE_CALIB_SW_GAIN_I2_ARRAY_SIZE) != DATAFLASH_OK)
    {
        return CONFIG_ERROR; /* Read error */
    }

    /* 
    * Get Driver ADC Gain Array 
    */
    /* Gain0 */
    if (DATAFLASH_Read(CONFIG_STORAGE_CALIB_DRIVER_ADC_GAIN0_ADDR,
                       (uint8_t *)&p_regs->gain0,
                       CONFIG_STORAGE_CALIB_DRIVER_ADC_GAIN0_SIZE) != DATAFLASH_OK)
    {
        return CONFIG_ERROR; /* Read error */
    }

    /* Gain1 */
    if (DATAFLASH_Read(CONFIG_STORAGE_CALIB_DRIVER_ADC_GAIN1_ADDR,
                       (uint8_t *)&p_regs->gain1,
                       CONFIG_STORAGE_CALIB_DRIVER_ADC_GAIN1_SIZE) != DATAFLASH_OK)
    {
        return CONFIG_ERROR; /* Read error */
    }

    /* Run internal CRC verify on dataflash, no need to get crc value */
    if (DATAFLASH_VerifyCRC(NULL) != DATAFLASH_OK)
    {
        return CONFIG_ERROR;
    }

    /* Success */
    return CONFIG_OK;
}
/******************************************************************************
* Function Name : CONFIG_EnableCRCChecking
* Interface     : void CONFIG_EnableCRCChecking(void)
* Description   : Enable to check CRC of storage
* Arguments     : None
* Return Value  : None
******************************************************************************/
CFS_FUNC void CONFIG_EnableCRCChecking(void)
{
    g_config_disable_CRC_checking = 0;
}

/******************************************************************************
* Function Name : CONFIG_DisableCRCChecking
* Interface     : void CONFIG_DisableCRCChecking(void)
* Description   : Disable to check CRC of storage
* Arguments     : None
* Return Value  : None
******************************************************************************/
CFS_FUNC void CONFIG_DisableCRCChecking(void)
{
    g_config_disable_CRC_checking = 1;
}

/***********************************************************************************************************************
* Function Name : CONFIG_PollingProcessing
* Interface     : void CONFIG_PollingProcessing(void)
* Description   : Polling processing for storage
*               : Calculate CRC for calibration content on RAM
*               : Compare with CRC value on Storage
*               : If not matched, consider RAM corrupt, restore from dataflash
* Arguments     : None
* Return Value  : None
***********************************************************************************************************************/
CFS_FUNC void CONFIG_PollingProcessing(void)
{
    config_crc_data     crc_data;
    EM_CALIBRATION      calib;
    dsad_reg_setting_t  regs;
    uint8_t             driver_adc_gain0;
    uint8_t             driver_adc_gain1;
    uint16_t            ram_calculated_crc;
    uint16_t            memory_calculated_crc;
    
    if (g_config_disable_CRC_checking == 1)
    {
        return;
    }
    
    /* CRC check for the current calibration data */
    if (g_config_storage_interval_flag == 1)
    {
        /* ACK */
        g_config_storage_interval_flag = 0;

        if (g_config_storage_state == CONFIG_STATE_INITIALIZED)
        {
            calib = EM_GetCalibInfo();
            
            /* Clear the temp driver gain list */
            memset(&driver_adc_gain0, 0, sizeof(driver_adc_gain0));
            memset(&driver_adc_gain1, 0, sizeof(driver_adc_gain1));
                        
            /* 
             * ADC Gain Array 
            */
            /* Get the gain first */
            R_DSADC_GetGainAndPhase(&regs);
            
            /* When using dual gain, ignore current ADC Gain, 
             * Using default value only (same as when backup, GAIN_X1)
            */
            #if (EM_SW_PROPERTY_ADC_GAIN_PHASE_NUMBER_LEVEL != 1)
            R_DSADC_ClearGainSetting(&regs, EM_ADC_DRIVER_CHANNEL_PHASE);
            #endif /* EM_SW_PROPERTY_ADC_GAIN_PHASE_NUMBER_LEVEL != 1 */
            
            #if (EM_SW_PROPERTY_ADC_GAIN_NEUTRAL_NUMBER_LEVEL != 1)
            R_DSADC_ClearGainSetting(&regs, EM_ADC_DRIVER_CHANNEL_NEUTRAL);
            #endif /* EM_SW_PROPERTY_ADC_GAIN_NEUTRAL_NUMBER_LEVEL != 1 */
            
            driver_adc_gain0 = regs.gain0;
            driver_adc_gain1 = regs.gain1;
                        
            /* Calculate the CRC on RAM calibration data */
            crc_data.p_calibration = &calib;
            crc_data.p_driver_adc_gain0_list = (uint8_t *)&driver_adc_gain0;
            crc_data.p_driver_adc_gain1_list = (uint8_t *)&driver_adc_gain1;
            
            ram_calculated_crc = CONFIG_CalculateCRC16(&crc_data);
            
            /* Run internal CRC verify of dataflash and get the CRC result for comparison with RAM content
            *  This assume dataflash memory correct as it already passed in bios checking 
            */
            DATAFLASH_VerifyCRC(&memory_calculated_crc);
            
            /* Check matching of RAM and Memory data */
            if (memory_calculated_crc != ram_calculated_crc)
            {
                /* CRC not matched, this assume the RAM content wrong, will restore from memory */
                if (EM_Stop() != EM_OK)
                {
                    
                }

                if (CONFIG_Restore(CONFIG_ITEM_CALIB) != CONFIG_OK)
                {
                    
                }
                else
                {
                    
                }
                
                if (EM_Start() != EM_OK)
                {
                    
                }
            }
            else
            {
                
            }
            
        }
    }
}

/***********************************************************************************************************************
* Function Name : CONFIG_RtcCallback
* Interface     : void CONFIG_RtcCallback()
* Description   : RTC Callback to count time
* Arguments     : None
* Return Value  : None
***********************************************************************************************************************/
CFS_FUNC void CONFIG_RtcCallback(void)
{
#if (CONFIG_STORAGE_CRC_CHECKING_DISABLE == 0)    
    static uint16_t counter = 0;
    
    /* Check to signal the interval flag for CRC check */
    counter++;
    if (counter >= CONFIG_STORAGE_INTERVAL_COUNT)
    {
        g_config_storage_interval_flag = 1;
        counter = 0;
    }
#endif
}
