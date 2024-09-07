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
* File Name    : r_meter_cmd_platform.c
* Version      : 1.00
* Device(s)    :
* Tool-Chain   :
* H/W Platform :
* Description  : Meter CMD - Platform information access
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
***********************************************************************************************************************/
/* Driver */
#include "r_cg_macrodriver.h"   /* Macro Driver Definitions */
#include "r_cg_dsadc.h"         /* DSADC Driver */
#include "r_cg_sau.h"
#include "r_cg_wdt.h"           /* WDT Driver */

#include "typedef.h"            /* GSCE Standard Typedef */

#include <stdlib.h>
#include <string.h>

/* Middleware */
#include "em_core.h"

/* Wrapper */
#include "wrp_mcu.h"
#include "wrp_em_sw_config.h"

/* Application */
#include "r_meter_cmd.h"
#include "r_meter_cmd_share.h"

//#include "format.h"
#include "config_format.h"
//#include "r_dlms_format.h"

#if (defined METER_ENABLE_PROPRIETARY_METER_COMMAND) && (METER_ENABLE_PROPRIETARY_METER_COMMAND == TRUE)

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/
typedef struct tagMeterCmdPlatformItemInfo
{
    uint32_t address;
    uint8_t one_element_size;
    uint8_t num_of_element;
    uint8_t length;
    uint8_t type;
    uint8_t mem_type;
    uint8_t data_type;
} MeterCmdPlatformItemInfo;

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/

#define METER_CMD_PLATFORM_ACCESS_OK                            (0x01)
#define METER_CMD_PLATFORM_ACCESS_ERROR     	                (0x02)
#define METER_CMD_PLATFORM_ACCESS_ERROR_UNKNOWN_TYPE            (0x03)

/*******************
 * Type code *******
*******************/
/* Group 1 */
#define METER_CMD_PLATFORM_TYPE_SAMPLING_FREQUENCY              (0x00)
#define METER_CMD_PLATFORM_TYPE_METER_CONSTANT                  (0x01)
#define METER_CMD_PLATFORM_TYPE_PULSE_ON_TIME                   (0x02)
#define METER_CMD_PLATFORM_TYPE_RTC_COMPENSATION                (0x03)
#define METER_CMD_PLATFORM_TYPE_ADC_GAIN_ARRAY                  (0x04)

/* Group 2 */
#define METER_CMD_PLATFORM_TYPE_PHASE1_VRMS                     (0x10)
#define METER_CMD_PLATFORM_TYPE_PHASE1_IRMS                     (0x11)
#define METER_CMD_PLATFORM_TYPE_PHASE1_ACTIVE_POWER_COEFF       (0x12)
#define METER_CMD_PLATFORM_TYPE_PHASE1_REACTIVE_POWER_COEFF     (0x13)
#define METER_CMD_PLATFORM_TYPE_PHASE1_APPARENT_POWER_COEFF     (0x14)
#define METER_CMD_PLATFORM_TYPE_PHASE1_PHASE_SHIFT_ANGLE_ARRAY  (0x15)
#define METER_CMD_PLATFORM_TYPE_PHASE1_SW_GAIN_ARRAY            (0x16)

/* Group 3 */
#define METER_CMD_PLATFORM_TYPE_PHASE2_VRMS                     (0x20)
#define METER_CMD_PLATFORM_TYPE_PHASE2_IRMS                     (0x21)
#define METER_CMD_PLATFORM_TYPE_PHASE2_ACTIVE_POWER_COEFF       (0x22)
#define METER_CMD_PLATFORM_TYPE_PHASE2_REACTIVE_POWER_COEFF     (0x23)
#define METER_CMD_PLATFORM_TYPE_PHASE2_APPARENT_POWER_COEFF     (0x24)
#define METER_CMD_PLATFORM_TYPE_PHASE2_PHASE_SHIFT_ANGLE_ARRAY  (0x25)
#define METER_CMD_PLATFORM_TYPE_PHASE2_SW_GAIN_ARRAY            (0x26)

/* Group 3 */
#define METER_CMD_PLATFORM_TYPE_PHASE3_VRMS                     (0x30)
#define METER_CMD_PLATFORM_TYPE_PHASE3_IRMS                     (0x31)
#define METER_CMD_PLATFORM_TYPE_PHASE3_ACTIVE_POWER_COEFF       (0x32)
#define METER_CMD_PLATFORM_TYPE_PHASE3_REACTIVE_POWER_COEFF     (0x33)
#define METER_CMD_PLATFORM_TYPE_PHASE3_APPARENT_POWER_COEFF     (0x34)
#define METER_CMD_PLATFORM_TYPE_PHASE3_PHASE_SHIFT_ANGLE_ARRAY  (0x35)
#define METER_CMD_PLATFORM_TYPE_PHASE3_SW_GAIN_ARRAY            (0x36)

/* Group 4 */
#define METER_CMD_PLATFORM_TYPE_NEUTRAL_IRMS                    (0x41)
#define METER_CMD_PLATFORM_TYPE_NEUTRAL_ACTIVE_POWER_COEFF      (0x42)
#define METER_CMD_PLATFORM_TYPE_NEUTRAL_REACTIVE_POWER_COEFF    (0x43)
#define METER_CMD_PLATFORM_TYPE_NEUTRAL_APPARENT_POWER_COEFF    (0x44)
#define METER_CMD_PLATFORM_TYPE_NEUTRAL_PHASE_SHIFT_ANGLE_ARRAY (0x45)
#define METER_CMD_PLATFORM_TYPE_NEUTRAL_SW_GAIN_ARRAY           (0x46)

/* Group 5 */
#define METER_CMD_PLATFORM_SERIAL_NUMBER                        (0x70)
#define METER_CMD_PLATFORM_MANUFACTURED_YEAR                    (0x71)

/***********************************************************************************************************************
Imported global variables and functions (from other files)
***********************************************************************************************************************/

/***********************************************************************************************************************
Exported global variables and functions (to be accessed by other files)
***********************************************************************************************************************/

/***********************************************************************************************************************
Private global variables and functions
***********************************************************************************************************************/
const MeterCmdPlatformItemInfo g_meter_cmd_platform_item_info[] = {
    {
        CONFIG_STORAGE_CALIB_SAMPLING_FREQUENCY_ADDR            ,
        sizeof(float32_t)                                       ,
        1                                                       ,
        CONFIG_STORAGE_CALIB_SAMPLING_FREQUENCY_SIZE            ,
        METER_CMD_PLATFORM_TYPE_SAMPLING_FREQUENCY              ,
        METER_CMD_MEM_TYPE_DATAFLASH                            ,
        METER_CMD_DATA_TYPE_FLOAT32                             ,
    },
    {
        CONFIG_STORAGE_CALIB_METER_CONSTANT_ADDR                ,
        sizeof(uint16_t)                                        ,
        1                                                       ,
        CONFIG_STORAGE_CALIB_METER_CONSTANT_SIZE                ,
        METER_CMD_PLATFORM_TYPE_METER_CONSTANT                  ,
        METER_CMD_MEM_TYPE_DATAFLASH                            ,
        METER_CMD_DATA_TYPE_UINT16                              ,
    },
    {
        CONFIG_STORAGE_CALIB_PULSE_ON_TIME_ADDR                 ,
        sizeof(uint16_t)                                        ,
        1                                                       ,
        CONFIG_STORAGE_CALIB_PULSE_ON_TIME_SIZE                 ,
        METER_CMD_PLATFORM_TYPE_PULSE_ON_TIME                   ,
        METER_CMD_MEM_TYPE_DATAFLASH                            ,
        METER_CMD_DATA_TYPE_UINT16                              ,
    },
    {
        CONFIG_STORAGE_CALIB_RTC_COMP_OFFSET_ADDR               ,
        sizeof(float32_t)                                       ,
        1                                                       ,
        CONFIG_STORAGE_CALIB_RTC_COMP_OFFSET_SIZE               ,
        METER_CMD_PLATFORM_TYPE_RTC_COMPENSATION                ,
        METER_CMD_MEM_TYPE_DATAFLASH                            ,
        METER_CMD_DATA_TYPE_FLOAT32                             ,
    },
    {
        CONFIG_STORAGE_CALIB_DRIVER_ADC_GAIN0_ADDR              ,
        sizeof(uint8_t)                                         ,
        2                                                       ,
        CONFIG_STORAGE_CALIB_DRIVER_ADC_GAIN0_SIZE * 2          ,
        METER_CMD_PLATFORM_TYPE_ADC_GAIN_ARRAY                  ,
        METER_CMD_MEM_TYPE_DATAFLASH                            ,
        METER_CMD_DATA_TYPE_ARRAY_UINT8                         ,
    },
    {
        CONFIG_STORAGE_CALIB_COEFF_VRMS_ADDR                    ,
        sizeof(float32_t)                                       ,
        1                                                       ,
        CONFIG_STORAGE_CALIB_COEFF_VRMS_SIZE                    ,
        METER_CMD_PLATFORM_TYPE_PHASE1_VRMS                     ,
        METER_CMD_MEM_TYPE_DATAFLASH                            ,
        METER_CMD_DATA_TYPE_FLOAT32                             ,
    },
    {
        CONFIG_STORAGE_CALIB_COEFF_I1RMS_ADDR                   ,
        sizeof(float32_t)                                       ,
        1                                                       ,
        CONFIG_STORAGE_CALIB_COEFF_I1RMS_SIZE                   ,
        METER_CMD_PLATFORM_TYPE_PHASE1_IRMS                     ,
        METER_CMD_MEM_TYPE_DATAFLASH                            ,
        METER_CMD_DATA_TYPE_FLOAT32                             ,
    },
    {
        CONFIG_STORAGE_CALIB_COEFF_ACTIVE_POWER_ADDR            ,
        sizeof(float32_t)                                       ,
        1                                                       ,
        CONFIG_STORAGE_CALIB_COEFF_ACTIVE_POWER_SIZE            ,
        METER_CMD_PLATFORM_TYPE_PHASE1_ACTIVE_POWER_COEFF       ,
        METER_CMD_MEM_TYPE_DATAFLASH                            ,
        METER_CMD_DATA_TYPE_FLOAT32                             ,
    },
    {
        CONFIG_STORAGE_CALIB_COEFF_REACTIVE_POWER_ADDR          ,
        sizeof(float32_t)                                       ,
        1                                                       ,
        CONFIG_STORAGE_CALIB_COEFF_REACTIVE_POWER_SIZE          ,
        METER_CMD_PLATFORM_TYPE_PHASE1_REACTIVE_POWER_COEFF     ,
        METER_CMD_MEM_TYPE_DATAFLASH                            ,
        METER_CMD_DATA_TYPE_FLOAT32                             ,
    },
    {
        CONFIG_STORAGE_CALIB_COEFF_APPARENT_POWER_ADDR          ,
        sizeof(float32_t)                                       ,
        1                                                       ,
        CONFIG_STORAGE_CALIB_COEFF_APPARENT_POWER_SIZE          ,
        METER_CMD_PLATFORM_TYPE_PHASE1_APPARENT_POWER_COEFF     ,
        METER_CMD_MEM_TYPE_DATAFLASH                            ,
        METER_CMD_DATA_TYPE_FLOAT32                             ,
    },
    {
        CONFIG_STORAGE_CALIB_SW_DEGREE_I1_ARRAY_ADDR            ,
        sizeof(float32_t)                                       ,
        EM_GAIN_PHASE_NUM_LEVEL_MAX                             ,
        CONFIG_STORAGE_CALIB_SW_DEGREE_I1_ARRAY_SIZE            ,
        METER_CMD_PLATFORM_TYPE_PHASE1_PHASE_SHIFT_ANGLE_ARRAY  ,
        METER_CMD_MEM_TYPE_DATAFLASH                            ,
        METER_CMD_DATA_TYPE_ARRAY_FLOAT32                       ,
    },
    {
        CONFIG_STORAGE_CALIB_SW_GAIN_I1_ARRAY_ADDR              ,
        sizeof(float32_t)                                       ,
        EM_GAIN_PHASE_NUM_LEVEL_MAX                             ,
        CONFIG_STORAGE_CALIB_SW_GAIN_I1_ARRAY_SIZE              ,
        METER_CMD_PLATFORM_TYPE_PHASE1_SW_GAIN_ARRAY            ,
        METER_CMD_MEM_TYPE_DATAFLASH                            ,
        METER_CMD_DATA_TYPE_ARRAY_FLOAT32                       ,
    },
    {
        CONFIG_STORAGE_CALIB_COEFF_I2RMS_ADDR                   ,
        sizeof(float32_t)                                       ,
        1                                                       ,
        CONFIG_STORAGE_CALIB_COEFF_I2RMS_SIZE                   ,
        METER_CMD_PLATFORM_TYPE_NEUTRAL_IRMS                    ,
        METER_CMD_MEM_TYPE_DATAFLASH                            ,
        METER_CMD_DATA_TYPE_FLOAT32                             ,
    },
    {
        CONFIG_STORAGE_CALIB_COEFF_ACTIVE_POWER2_ADDR           ,
        sizeof(float32_t)                                       ,
        1                                                       ,
        CONFIG_STORAGE_CALIB_COEFF_ACTIVE_POWER2_SIZE           ,
        METER_CMD_PLATFORM_TYPE_NEUTRAL_ACTIVE_POWER_COEFF      ,
        METER_CMD_MEM_TYPE_DATAFLASH                            ,
        METER_CMD_DATA_TYPE_FLOAT32                             ,
    },
    {
        CONFIG_STORAGE_CALIB_COEFF_REACTIVE_POWER2_ADDR         ,
        sizeof(float32_t)                                       ,
        1                                                       ,
        CONFIG_STORAGE_CALIB_COEFF_REACTIVE_POWER2_SIZE         ,
        METER_CMD_PLATFORM_TYPE_NEUTRAL_REACTIVE_POWER_COEFF    ,
        METER_CMD_MEM_TYPE_DATAFLASH                            ,
        METER_CMD_DATA_TYPE_FLOAT32                             ,
    },
    {
        CONFIG_STORAGE_CALIB_COEFF_APPARENT_POWER2_ADDR         ,
        sizeof(float32_t)                                       ,
        1                                                       ,
        CONFIG_STORAGE_CALIB_COEFF_APPARENT_POWER2_SIZE         ,
        METER_CMD_PLATFORM_TYPE_NEUTRAL_APPARENT_POWER_COEFF    ,
        METER_CMD_MEM_TYPE_DATAFLASH                            ,
        METER_CMD_DATA_TYPE_FLOAT32                             ,
    },
    {
        CONFIG_STORAGE_CALIB_SW_DEGREE_I2_ARRAY_ADDR            ,
        sizeof(float32_t)                                       ,
        EM_GAIN_NEUTRAL_NUM_LEVEL_MAX                           ,
        CONFIG_STORAGE_CALIB_SW_DEGREE_I2_ARRAY_SIZE            ,
        METER_CMD_PLATFORM_TYPE_NEUTRAL_PHASE_SHIFT_ANGLE_ARRAY ,
        METER_CMD_MEM_TYPE_DATAFLASH                            ,
        METER_CMD_DATA_TYPE_ARRAY_FLOAT32                       ,
    },
    {
        CONFIG_STORAGE_CALIB_SW_GAIN_I2_ARRAY_ADDR              ,
        sizeof(float32_t)                                       ,
        EM_GAIN_NEUTRAL_NUM_LEVEL_MAX                           ,
        CONFIG_STORAGE_CALIB_SW_GAIN_I2_ARRAY_SIZE              ,
        METER_CMD_PLATFORM_TYPE_NEUTRAL_SW_GAIN_ARRAY           ,
        METER_CMD_MEM_TYPE_DATAFLASH                            ,
        METER_CMD_DATA_TYPE_ARRAY_FLOAT32                       ,
    },
    {
        STORAGE_EEPROM_DLMS_METER_SERIAL_NUMBER_ADDR            ,
        sizeof(uint8_t)                                         ,
        16                                                      ,
        STORAGE_EEPROM_DLMS_METER_SERIAL_NUMBER_SIZE            ,
        METER_CMD_PLATFORM_SERIAL_NUMBER                        ,
        METER_CMD_MEM_TYPE_EEPROM                               ,
        METER_CMD_DATA_TYPE_ARRAY_UINT8                         ,
    },
    {
        STORAGE_EEPROM_DLMS_MANUFATURE_YEAR_ADDR                ,
        sizeof(uint8_t)                                         ,
        4                                                       ,
        STORAGE_EEPROM_DLMS_MANUFATURE_YEAR_SIZE                ,
        METER_CMD_PLATFORM_MANUFACTURED_YEAR                    ,
        METER_CMD_MEM_TYPE_EEPROM                               ,
        METER_CMD_DATA_TYPE_ARRAY_UINT8                         ,
    },
};

const uint16_t g_meter_cmd_platform_item_info_size = sizeof(g_meter_cmd_platform_item_info) / sizeof(MeterCmdPlatformItemInfo);

/***********************************************************************************************************************
* Function Name: r_meter_cmd_process_platform_info_access_request
* Description  : Handler for platform info access request
* Arguments    : MeterCmdDataBuffer * p_req_buffer
*              : MeterCmdFrameInfo * p_frame_info
*              : uint8_t is_read
*              : uint8_t type
* Return Value : uint8_t: 0 if OK, 1 for ERROR
***********************************************************************************************************************/
static uint8_t r_meter_cmd_process_platform_info_access_request(MeterCmdDataBuffer * p_req_buffer, MeterCmdFrameInfo * p_frame_info, uint8_t is_read, uint8_t type)
{
    uint8_t * p_alloc_buffer;
    uint8_t remaining_req_length, read_length;
    uint32_t access_address;
    uint16_t i;

    MeterCmdPlatformItemInfo item;

    /* Mark frame correct */
    p_frame_info->ret = METER_CMD_OK;

    /* Scan for platform info table index */
    for (i = 0; i < g_meter_cmd_platform_item_info_size; i++)
    {
        if (g_meter_cmd_platform_item_info[i].type == type) {
            break;
        }
    }

    if (i == g_meter_cmd_platform_item_info_size)
    {
        R_METER_CMD_EncodeUInt8ToBuffer(&p_frame_info->data_buffer, METER_CMD_PLATFORM_ACCESS_ERROR_UNKNOWN_TYPE);
        R_METER_CMD_PackAndSendResFrame(p_frame_info);
        return 1;
    }

    /* Derive additional information related to access item */
    item = g_meter_cmd_platform_item_info[i];
    remaining_req_length = p_req_buffer->length - p_req_buffer->decoded_length;
    access_address = item.address;

    /* Recheck length of access request */
    if (is_read) {
        METER_CMD_ASSERT_TRUE_SEND_ERROR_CODE_RET_VALUE(
            (remaining_req_length != 0),
            METER_CMD_ERROR_INCORRECT_ARGUMENT_LENGTH,
            1
        );
    }
    else {
        METER_CMD_ASSERT_TRUE_SEND_ERROR_CODE_RET_VALUE(
            (remaining_req_length != (item.length)),
            METER_CMD_ERROR_INCORRECT_ARGUMENT_LENGTH,
            1
        );
    }

    R_METER_CMD_EncodeUInt8ToBuffer(&p_frame_info->data_buffer, METER_CMD_PLATFORM_ACCESS_OK);

    /* All checking is done, start platform information access */
    if (is_read) {
        read_length = item.length;

        p_alloc_buffer = METER_CMD_ALLOC_BUFFER(read_length);

        /* Access memory */
        if (item.mem_type == METER_CMD_MEM_TYPE_EEPROM) {
            EPR_Read(access_address, p_alloc_buffer, read_length);
        }
        else {
            DATAFLASH_Read(access_address, p_alloc_buffer, read_length);
        }

        /* Encode and send frame */
        R_METER_CMD_EncodeDataTypeToBuffer(&p_frame_info->data_buffer, p_alloc_buffer, item.data_type, item.num_of_element);

        METER_CMD_FREE_BUFFER(p_alloc_buffer);
    }
    else {
        p_alloc_buffer = METER_CMD_ALLOC_BUFFER(remaining_req_length);

        /* Decode buffer */
        R_METER_CMD_DecodeBufferToDataType(p_req_buffer, p_alloc_buffer, item.data_type, remaining_req_length / item.one_element_size);

        /* Access memory */
        if (item.mem_type == METER_CMD_MEM_TYPE_EEPROM) {
            EPR_Write(access_address, p_alloc_buffer, remaining_req_length);
        }
        else {
            DATAFLASH_Write(access_address, p_alloc_buffer, remaining_req_length);
        }

        METER_CMD_FREE_BUFFER(p_alloc_buffer);
    }

    R_METER_CMD_PackAndSendResFrame(p_frame_info);
    return 0;
}

/***********************************************************************************************************************
* Function Name: METER_CMD_ProcessCmdPlatformAccess
* Description  : Entry function to process for platform access command
* Arguments    : MeterCmdDataBuffer * p_req_buffer
*              : MeterCmdFrameInfo * p_frame_info
* Return Value : uint8_t: 0: if ok, others: error
***********************************************************************************************************************/
uint8_t METER_CMD_ProcessCmdPlatformAccess(MeterCmdDataBuffer * p_req_buffer, MeterCmdFrameInfo * p_frame_info, uint8_t is_unlock_access)
{
    /* Length: args */
    /* Arguments layout
     * 1byte: Read or WriteN
     * 2byte: sets of samples
    */
    uint8_t is_read, type;

    /* Check parameter length */
    METER_CMD_ASSERT_TRUE_SEND_ERROR_CODE_RET_VALUE(
        (p_req_buffer->length < 2),
        METER_CMD_ERROR_INCORRECT_ARGUMENT_LENGTH,
        1
    );

    /* Get and check access value */
    is_read = R_METER_CMD_DecodeBufferToUInt8(p_req_buffer);

    METER_CMD_ASSERT_TRUE_SEND_ERROR_CODE_RET_VALUE(
        (is_read != 0 && is_read != 1),
        METER_CMD_ERROR_UNKNOWN_COMMAND_INPUT,
        1
    );

    /* Get type and index parameters */
    type = R_METER_CMD_DecodeBufferToUInt8(p_req_buffer);

    /* Process the request */
    return r_meter_cmd_process_platform_info_access_request(p_req_buffer, p_frame_info, is_read, type);
}

#endif