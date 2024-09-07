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
* File Name    : r_meter_cmd_ocmem.c
* Version      : 1.00
* Device(s)    :
* Tool-Chain   :
* H/W Platform :
* Description  : Meter CMD - On chip memory access
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
***********************************************************************************************************************/
/* Driver */
#include "r_cg_macrodriver.h"   /* Macro Driver Definitions */
#include "r_cg_userdefine.h"    /* CG User Define */
#include "r_cg_rtc.h"           /* RTC Driver */
#include "r_cg_wdt.h"           /* WDT Driver */

#include "typedef.h"            /* GSCE Standard Typedef */

#include <stdlib.h>
#include <string.h>

/* Middleware */

/* Wrapper */

/* Application */
#include "r_meter_cmd.h"
#include "r_meter_cmd_share.h"
#include "eeprom.h"
#include "dataflash.h"
//#include "bl_serialflash.h"
//#include "em_display.h"                 /* LCD Display Pannel */
#include "HardwareTest.h"

#if (defined METER_ENABLE_PROPRIETARY_METER_COMMAND) && (METER_ENABLE_PROPRIETARY_METER_COMMAND == TRUE)

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/
typedef uint8_t(*FUNC_MemRead)(uint32_t addr, uint8_t *buf, uint16_t size);
typedef uint8_t(*FUNC_MemWrite)(uint32_t addr, uint8_t *buf, uint16_t size);

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/

#define METER_CMD_MEMORY_ACCESS_OK                              (0x01)
#define METER_CMD_MEMORY_ACCESS_ERROR                           (0x02)
#define METER_CMD_MEMORY_ACCESS_ERROR_MAX_LENGTH                (0x03)
#define METER_CMD_MEMORY_ACCESS_ERROR_OUT_OF_PC                 (0x04)
#define METER_CMD_MEMORY_ACCESS_ERROR_UNSUPPORTED_TYPE          (0x05)

#define METER_CMD_OCMEM_MAX_PC_SOLVABLE_ADDRESS                 (0x000FFFFF)
#define METER_CMD_MEMORY_ACCESS_MAX_LENGTH                      (128)

#define METER_CMD_MEMORY_ACCESS_TYPE_ONCHIP                     (0x01)
#define METER_CMD_MEMORY_ACCESS_TYPE_EEPROM                     (0x02)
#define METER_CMD_MEMORY_ACCESS_TYPE_DATAFLASH                  (0x03)
#define METER_CMD_MEMORY_ACCESS_TYPE_SERIALFLASH                (0x04)
#define METER_CMD_MEMORY_ACCESS_TYPE_INSTANTPARAM               (0x05)

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
* Function Name: r_meter_cmd_on_chip_memory_read
* Description  : Read on chip memory
* Arguments    : uint32_t addr
*              : uint8_t* buf
*              : uint16_t size
* Return Value : uint8_t: follow format of eeprom.h
***********************************************************************************************************************/
static uint8_t r_meter_cmd_on_chip_memory_read(uint32_t addr, uint8_t* buf, uint16_t size)
{
    uint16_t i;
    uint16_t value;
    if (addr + size > METER_CMD_OCMEM_MAX_PC_SOLVABLE_ADDRESS) {
        /* Out of 20bit PC size */
        return 3;
    }
    /* For RL78 device, SFR size is only 8bit and 16bit size */
    if (size == 1) {
        /* Access using 8bit instruction */
        buf[0] = *((FAR_PTR uint8_t *)(addr));
    }
    else if (size == 2) {
        /* Access using 16bit instruction */
        value = *((FAR_PTR uint16_t *)(addr));
        buf[0] = *(((uint8_t *)&value) + 0);
        buf[1] = *(((uint8_t *)&value) + 1);
    }
    else {
        /* Access > SFR size, so consider ROM or RAM */
        for (i = 0; i < size; i++)
        {
            buf[i] = *((FAR_PTR uint8_t *)(addr + i));
        }
    }
    return 0;
}
/***********************************************************************************************************************
* Function Name: r_meter_cmd_on_chip_memory_write
* Description  : Write on chip memory
* Arguments    : uint32_t addr
*              : uint8_t* buf
*              : uint16_t size
* Return Value : uint8_t: follow format of eeprom.h
***********************************************************************************************************************/
static uint8_t r_meter_cmd_on_chip_memory_write(uint32_t addr, uint8_t* buf, uint16_t size)
{
    uint16_t i;
    uint16_t value;
    if (addr + size > METER_CMD_OCMEM_MAX_PC_SOLVABLE_ADDRESS) {
        /* Out of 20bit PC size */
        return 3;
    }
    /* For RL78 device, SFR size is only 8bit and 16bit size */
    if (size == 1) {
        /* Access using 8bit instruction */
        *((FAR_PTR uint8_t *)(addr)) = buf[0];
    }
    else if (size == 2) {
        /* Access using 16bit instruction */
        *(((uint8_t *)&value) + 0) = buf[0];
        *(((uint8_t *)&value) + 1) = buf[1];
        *((FAR_PTR uint16_t *)(addr)) = value;
    }
    else {
        /* Access > SFR size, so consider ROM or RAM */
        for (i = 0; i < size; i++)
        {
            *((FAR_PTR uint8_t *)(addr + i)) = buf[i];
        }
    }
    return 0;
}

/***********************************************************************************************************************
* Function Name: r_meter_cmd_process_onchip_memory_access_request
* Description  : Handler for on-chip memory access request
* Arguments    : MeterCmdDataBuffer * p_req_buffer
*              : MeterCmdFrameInfo * p_frame_info
*              : uint8_t is_read
*              : uint8_t mem_type
*              : uint32_t address
*              : uint8_t length
* Return Value : uint8_t: 0: if ok, others: error
***********************************************************************************************************************/
static uint8_t r_meter_cmd_process_onchip_memory_access_request(MeterCmdDataBuffer * p_req_buffer, MeterCmdFrameInfo * p_frame_info, uint8_t is_read, uint8_t mem_type, uint32_t address, uint8_t length, uint8_t is_unlock_access)
{
    uint8_t buffer[METER_CMD_MEMORY_ACCESS_MAX_LENGTH];
    uint8_t backup_vrtc, is_continue, result;
    FUNC_MemRead read_func;
    FUNC_MemWrite write_func;

    /* Mark frame correct */
    p_frame_info->ret = METER_CMD_OK;

    /* Backup VRTCEN settings */
    backup_vrtc = VRTCEN;
    VRTCEN = 1;

    /* Recheck length of access request */
    if (length > METER_CMD_MEMORY_ACCESS_MAX_LENGTH) {
        R_METER_CMD_EncodeUInt8ToBuffer(&p_frame_info->data_buffer, METER_CMD_MEMORY_ACCESS_ERROR_MAX_LENGTH);
    }
    else if ((length == 0) || ((is_unlock_access == 0) && (is_read == 0)))
    {
        R_METER_CMD_EncodeUInt8ToBuffer(&p_frame_info->data_buffer, METER_CMD_MEMORY_ACCESS_ERROR);
    }
    else {
        if (mem_type == METER_CMD_MEMORY_ACCESS_TYPE_ONCHIP) {
            read_func = r_meter_cmd_on_chip_memory_read;
            write_func = r_meter_cmd_on_chip_memory_write;
            is_continue = TRUE;
        }
        else if (mem_type == METER_CMD_MEMORY_ACCESS_TYPE_EEPROM) {
            read_func = EPR_Read;
            write_func = EPR_Write;
            is_continue = TRUE;
        }
        else if (mem_type == METER_CMD_MEMORY_ACCESS_TYPE_DATAFLASH) {
            read_func = DATAFLASH_Read;
            write_func = DATAFLASH_Write;
            is_continue = TRUE;
        }
        else if (mem_type == METER_CMD_MEMORY_ACCESS_TYPE_INSTANTPARAM) {
            read_func = getInstantParam;
            write_func = setInstantParam;
            is_continue = TRUE;
        }
//        else if (mem_type == METER_CMD_MEMORY_ACCESS_TYPE_SERIALFLASH) {
//            read_func = BL_SFL_DeviceRead;
//            /* Unsupport write operation */
//            if (is_read == 0) {
//                R_METER_CMD_EncodeUInt8ToBuffer(&p_frame_info->data_buffer, METER_CMD_MEMORY_ACCESS_ERROR);
//                is_continue = FALSE;
//            }
//        }
        else {
            R_METER_CMD_EncodeUInt8ToBuffer(&p_frame_info->data_buffer, METER_CMD_MEMORY_ACCESS_ERROR_UNSUPPORTED_TYPE);
            is_continue = FALSE;
        }

        if (is_continue == TRUE)
        {
            if (is_read) {
                result = read_func(address, buffer, length);
            }
            else {
                /* Decode buffer */
                R_METER_CMD_DecodeBufferToArrayUInt8(p_req_buffer, buffer, length);
                result = write_func(address, buffer, length);
            }

            if (result != 0)
            {
                if (result == 3) {
                    R_METER_CMD_EncodeUInt8ToBuffer(&p_frame_info->data_buffer, METER_CMD_MEMORY_ACCESS_ERROR_MAX_LENGTH);
                }
                else {
                    R_METER_CMD_EncodeUInt8ToBuffer(&p_frame_info->data_buffer, METER_CMD_MEMORY_ACCESS_ERROR);
                }
            }
            else {
                R_METER_CMD_EncodeUInt8ToBuffer(&p_frame_info->data_buffer, METER_CMD_MEMORY_ACCESS_OK);

                if (is_read) {
                    R_METER_CMD_EncodeArrayUInt8ToBuffer(&p_frame_info->data_buffer, buffer, length);
                }
            }
        }
    }

    /* Restore VRTCEN settings */
    VRTCEN = backup_vrtc;

    R_METER_CMD_PackAndSendResFrame(p_frame_info);

    return 0;
}

/***********************************************************************************************************************
* Function Name: R_METER_CMD_ProcessCmdOcMemAccess
* Description  : Entry function to process for on-chip memory access command
* Arguments    : MeterCmdDataBuffer * p_req_buffer
*              : MeterCmdFrameInfo * p_frame_info
* Return Value : uint8_t: 0: if ok, others: error
***********************************************************************************************************************/
uint8_t R_METER_CMD_ProcessCmdOcMemAccess(MeterCmdDataBuffer * p_req_buffer, MeterCmdFrameInfo * p_frame_info, uint8_t is_unlock_access)
{
    /* Length: args */
    /* Arguments layout
     * 1byte: Read or WriteN
     * 1byte: Memory Type
     * 4byte: Address
     * 1byte: Length
    */
    uint8_t is_read, length, mem_type;
    uint32_t address;

    /* Check parameter length */
    METER_CMD_ASSERT_TRUE_SEND_ERROR_CODE_RET_VALUE(
        (p_req_buffer->length < 6),
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

    /* Get memory type parameters */
    mem_type = R_METER_CMD_DecodeBufferToUInt8(p_req_buffer);

    /* Get address parameters */
    address = R_METER_CMD_DecodeBufferToUInt32(p_req_buffer);

    /* Get length parameters */
    length = R_METER_CMD_DecodeBufferToUInt8(p_req_buffer);

    /* Process the request */
    return r_meter_cmd_process_onchip_memory_access_request(p_req_buffer, p_frame_info, is_read, mem_type, address, length, is_unlock_access);
}

#endif