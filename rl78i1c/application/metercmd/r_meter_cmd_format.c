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
* File Name    : r_meter_cmd_format.c
* Version      : 1.00
* Device(s)    :
* Tool-Chain   :
* H/W Platform :
* Description  : Meter CMD - External memory format
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
***********************************************************************************************************************/
/* Driver */
#include "r_cg_macrodriver.h"   /* Macro Driver Definitions */
#include "r_cg_userdefine.h"    /* CG User Define */
#include "r_cg_wdt.h"           /* WDT Driver */

#include "typedef.h"            /* GSCE Standard Typedef */

#include <stdlib.h>
#include <string.h>

/* Application */
#include "r_meter_cmd.h"
#include "r_meter_cmd_share.h"
#include "eeprom.h"
#include "dataflash.h"
#include "eeprom_storage.h"
#include "HardwareTest.h"
#include "config_storage.h"
//#include "storage.h"

#include "wrp_user_ext.h"
//#include "r_dlms_format.h"
//#include "event.h"

#if (defined METER_ENABLE_PROPRIETARY_METER_COMMAND) && (METER_ENABLE_PROPRIETARY_METER_COMMAND == TRUE)

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
#define METER_CMD_FORMAT_EXPECTED_ARGS_LENGTH       (2)

#define METER_CMD_FORMAT_RET_OK                     (0x01)
#define METER_CMD_FORMAT_ERROR_UNSUPPORTED_AREA     (0x02)

#define METER_CMD_FORMAT_ACTION_IMAGE_BACKUP        (0x01)
#define METER_CMD_FORMAT_ACTION_MEM_TEST_I2C        (0x05)
#define METER_CMD_FORMAT_ACTION_MEM_TEST_SPI        (0x06)
#define METER_CMD_FORMAT_ACTION_RELAY_CONNECT       (0x07)
#define METER_CMD_FORMAT_ACTION_RELAY_DISCONNECT    (0x08)
#define METER_CMD_FORMAT_ACTION_RF_IO_LOW_CHECK     (0x09)
#define METER_CMD_FORMAT_ACTION_RF_IO_HIGH_CHECK    (0x0A)
#define METER_CMD_FORMAT_ACTION_RF_IO_LOW_CHECK_ONBOARD     (0x0B)
#define METER_CMD_FORMAT_ACTION_RF_IO_HIGH_CHECK_ONBOARD    (0x0C)

/***********************************************************************************************************************
Imported global variables and functions (from other files)
***********************************************************************************************************************/
//TODO: Rakesh sept 2022 check rf test case
/*extern void relayTest(uint8_t byConnectState);
extern uint8_t TestMem(uint8_t byMemType);
extern uint8_t rfPinsCheck(uint8_t byInput);
extern uint8_t rfPinsCheck_OnBoard(uint8_t byInput);
*/
/***********************************************************************************************************************
Exported global variables and functions (to be accessed by other files)
***********************************************************************************************************************/

/***********************************************************************************************************************
Private global variables and functions
***********************************************************************************************************************/

/***********************************************************************************************************************
* Function Name: r_meter_cmd_invoke_action
* Description  : Handler for invoking action (plain action without data)
* Arguments    : uint8_t action_id
* Return Value : uint8_t: 0: if ok, others: error
***********************************************************************************************************************/
uint8_t r_meter_cmd_invoke_action(uint8_t action_id)
{
    uint8_t result = TRUE;

    switch (action_id)
    {
    case METER_CMD_FORMAT_ACTION_IMAGE_BACKUP:
        //EVENT_BackupImage();
        break;
    case METER_CMD_FORMAT_ACTION_MEM_TEST_I2C:
        result = TestMem(0);                                //50 seconds
        break;
    case METER_CMD_FORMAT_ACTION_MEM_TEST_SPI:
        //result = TestMem(1);                                //150 seconds
        break;
    case METER_CMD_FORMAT_ACTION_RELAY_CONNECT:
        relayTest(0);
        break;
    case METER_CMD_FORMAT_ACTION_RELAY_DISCONNECT:
        relayTest(1);
        break;
    case METER_CMD_FORMAT_ACTION_RF_IO_LOW_CHECK:
        result = rfPinsCheck(0);
        break;
    case METER_CMD_FORMAT_ACTION_RF_IO_HIGH_CHECK:
        result = rfPinsCheck(1);
        break;
    case METER_CMD_FORMAT_ACTION_RF_IO_LOW_CHECK_ONBOARD:
        result = rfPinsCheck_OnBoard(0);//0 for low
        break;
    case METER_CMD_FORMAT_ACTION_RF_IO_HIGH_CHECK_ONBOARD:
        result = rfPinsCheck_OnBoard(1);//1 for high
        break;
    default:
        /* Command not supported, mark as false */
        result = FALSE;
        break;
    }

    return result;
}

/***********************************************************************************************************************
* Function Name: r_meter_cmd_do_memory_format
* Description  : Handler for memory format request
* Arguments    : MeterCmdFrameInfo * p_frame_info
*              : uint8_t mem_type
*              : uint8_t area
* Return Value : uint8_t: 0: if ok, others: error
***********************************************************************************************************************/
uint8_t r_meter_cmd_do_memory_format(MeterCmdFrameInfo * p_frame_info, uint8_t mem_type, uint8_t area, uint8_t is_unlock_access)
{
    uint8_t result;
    uint8_t reset_buffer[4];

    /* EEPROM and DATAFLASH has a header to manage the memory format validity
    * Whole memory can be triggered by clearing that header code then restart for default memory using
    * NOTE: the reset is in send end callback
    */
    memset(reset_buffer, 0, 4);

    result = FALSE;

    /* Currently support all area of eeprom only */
    //if ((IS_CAL_JUMPER_ACTIVE()) && (mem_type & METER_CMD_MEM_TYPE_EEPROM))
    //TODO: Rakesh sept 2022 validate, if okay?.
    if ((is_unlock_access) && (mem_type & METER_CMD_MEM_TYPE_EEPROM))
    {
        if (area == 0xFF)
        {
            /* Register to application */
            EPR_Format_Init(MEMORY_FORMAT_COMMAND_STORAGE_LOC, MEMORY_FORMAT_COMMAND_STORAGE_SIZE, ERP_ERASE_TYPE_2);
            result = TRUE;
//            if (EPR_OK == EPR_Format(0, ERP_DEVICE_RESERVE_SPACE))
//            {
//                if(epr_init_after_full_erase_callback != 0)
//                {
//                    epr_init_after_full_erase_callback();
//                }
//                result = TRUE;
//            }
        }
    }

    //if ((IS_CAL_JUMPER_ACTIVE()) && (mem_type & METER_CMD_MEM_TYPE_DATAFLASH))
    ////TODO: Rakesh sept 2022 valid, if okay?
    if ((is_unlock_access) && (mem_type & METER_CMD_MEM_TYPE_DATAFLASH))
    {
        /* Currently support all area of data flash only */
        if (area == 0xFF)
        {
            /* Register to application */
            if (DATAFLASH_OK == DATAFLASH_Format())
            {
                result = TRUE;
            }
            EPR_Format_Init(MEMORY_FORMAT_COMMAND_STORAGE_LOC, MEMORY_FORMAT_COMMAND_STORAGE_SIZE, ERP_ERASE_TYPE_1);
            result = TRUE;
//            if (EPR_OK == EPR_Format(0, 0))
//            {
//                if(epr_init_after_full_erase_callback != 0)
//                {
//                    epr_init_after_full_erase_callback();
//                }
//                result = TRUE;
//            }
        }
    }

    if (mem_type & METER_CMD_MEM_TYPE_INVOKING_ACTION)
    {
        result = r_meter_cmd_invoke_action(area);
    }

    /* Prepare reply buffer */
    p_frame_info->ret = METER_CMD_OK;

    if (result == TRUE) {
        R_METER_CMD_EncodeUInt8ToBuffer(&p_frame_info->data_buffer, METER_CMD_FORMAT_RET_OK);
    }
    else {
        R_METER_CMD_EncodeUInt8ToBuffer(&p_frame_info->data_buffer, METER_CMD_FORMAT_ERROR_UNSUPPORTED_AREA);
    }

    /* Send reply buffer */
    R_METER_CMD_PackAndSendResFrame(p_frame_info);

    /* Post action after reply */
    if (result == TRUE)
    {
        /* Reset meter to properly format the memory */
        if ((mem_type & METER_CMD_MEM_TYPE_EEPROM) ||
            (mem_type & METER_CMD_MEM_TYPE_DATAFLASH))
        {
            WRP_EXT_SoftReset();
        }
        else
        {
            /* No process */
        }
    }

    return result;
}

/***********************************************************************************************************************
* Function Name: R_METER_CMD_ProcessCmdMemoryFormat
* Description  : Entry function to process for memory format command
* Arguments    : MeterCmdDataBuffer * p_req_buffer
*              : MeterCmdFrameInfo * p_frame_info
* Return Value : uint8_t: 0: if ok, others: error
***********************************************************************************************************************/
uint8_t R_METER_CMD_ProcessCmdMemoryFormat(MeterCmdDataBuffer * p_req_buffer, MeterCmdFrameInfo * p_frame_info, uint8_t is_unlock_access)
{
    /* Length: args */
    /* Arguments layout
     * 1byte: Memory frame_type
     * 1byte: Memory area mask
    */

    uint8_t mem_type;
    uint8_t area;

    /* Check parameter length */
    METER_CMD_CHECK_EXPECTED_LENGTH(METER_CMD_FORMAT_EXPECTED_ARGS_LENGTH);

    /* Decode parameters */
    mem_type = R_METER_CMD_DecodeBufferToUInt8(p_req_buffer);
    area = R_METER_CMD_DecodeBufferToUInt8(p_req_buffer);
    r_meter_cmd_do_memory_format(p_frame_info, mem_type, area, is_unlock_access);

    return 0;
}

#endif
