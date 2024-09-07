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
* File Name    : r_meter_cmd_share.h
* Version      : 1.00
* Device(s)    :
* Tool-Chain   :
* H/W Platform :
* Description  : Meter CMD: Share definitions and function prototypes
***********************************************************************************************************************/

#ifndef __R_METER_CMD_SHARE_H
#define __R_METER_CMD_SHARE_H

/***********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
***********************************************************************************************************************/
#include "compiler.h"
#include "em_type.h"
#include "platform.h"

/* Wrapper */
#include "wrp_user_ext.h"

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/
typedef enum tagMeterCmdCode
{
    METER_CMD_CODE_AUTHENTICATION = 0,
    METER_CMD_CODE_CALIBRATION,
    METER_CMD_CODE_MEMORY_FORMAT = 0x03,
    METER_CMD_CODE_SIGLOG,
    METER_CMD_CODE_OCMEM_ACCESS,
    METER_CMD_CODE_PLATFORM_ACCESS,
    METER_CMD_CODE_UNLOCK_ACCESS,
} MeterCmdCode_t;

typedef enum tagMeterCmdRet
{
    METER_CMD_OK = 0x00,
    METER_CMD_ERROR_UNEXPECTED_FRAME_TYPE,
    METER_CMD_ERROR_UNSUPPORTED_COMMAND_CODE,
    METER_CMD_ERROR_INCORRECT_ARGUMENT_LENGTH,
    METER_CMD_ERROR_UNKNOWN_COMMAND_INPUT,
    METER_CMD_ERROR_EXECUTION_FAILED,
    METER_CMD_ERROR_AUTHENTICATION_FAILED,
    METER_CMD_ERROR_APPLICATION_LOCKED,
    METER_CMD_ERROR_UNLOCK_FAILED,
} MeterCmdRet_t;

typedef struct tagMeterCmdDataBuffer
{
    uint8_t * data;
    uint8_t length;
    uint8_t decoded_length;
} MeterCmdDataBuffer;

typedef struct tagMeterCmdSendBuffer
{
    uint8_t * data;
    uint8_t size;
}MeterCmdRawBuffer;

typedef enum tagFrameType
{
    FRAME_TYPE_REQUEST = 0x01,
    FRAME_TYPE_RESPONSE = 0x03,
    FRAME_TYPE_NOTIFICATION = 0x05,
} MeterCmdFrameType_t;

typedef struct tagMeterCmdFrameInfo
{
    MeterCmdFrameType_t frame_type;
    MeterCmdCode_t code;
    MeterCmdRet_t ret;
    MeterCmdRawBuffer raw_buffer;
    MeterCmdDataBuffer data_buffer;
    uint16_t crc;
} MeterCmdFrameInfo;

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
#define METER_CMD_DATA_TYPE_UINT8                           (1)
#define METER_CMD_DATA_TYPE_UINT16                          (2)
#define METER_CMD_DATA_TYPE_UINT32                          (3)
#define METER_CMD_DATA_TYPE_FLOAT32                         (4)
#define METER_CMD_DATA_TYPE_ARRAY_UINT8                     (5)
#define METER_CMD_DATA_TYPE_ARRAY_FLOAT32                   (6)

#define METER_CMD_MEM_TYPE_EEPROM                           (0x01)
#define METER_CMD_MEM_TYPE_DATAFLASH                        (0x02)
#define METER_CMD_MEM_TYPE_INVOKING_ACTION                  (0x08)      /* Memory area for invoking simple action from meter */

/* Communication format */
#define METER_CMD_FIRST_CODE_SIZE                           (1)
#define METER_CMD_FRAME_TYPE_SIZE                           (1)
#define METER_CMD_MESSAGE_LENGTH_SIZE                       (1)
#define METER_CMD_CRC_SIZE                                  (sizeof(uint16_t))
#define METER_CMD_FINAL_CODE_SIZE                           (1)

#define METER_CMD_HEADER_SIZE                               (METER_CMD_FIRST_CODE_SIZE + METER_CMD_FRAME_TYPE_SIZE)
#define METER_CMD_FOOTER_SIZE                               (METER_CMD_FINAL_CODE_SIZE)
#define METER_CMD_HEADER_FOOTER_SIZE                        (METER_CMD_HEADER_SIZE + METER_CMD_FOOTER_SIZE)
#define METER_CMD_COMMAND_CODE_SIZE                         (1)
#define METER_CMD_RESP_FRAME_RET_SIZE                       (1)

#define METER_CMD_LENGTH_REQ_FRAME_MIN_SIZE                 (\
    METER_CMD_MESSAGE_LENGTH_SIZE + \
    METER_CMD_CRC_SIZE +    \
    METER_CMD_COMMAND_CODE_SIZE \
)
#define METER_CMD_LENGTH_RES_FRAME_MIN_SIZE                 (\
    METER_CMD_MESSAGE_LENGTH_SIZE + \
    METER_CMD_CRC_SIZE + \
    METER_CMD_COMMAND_CODE_SIZE + \
    METER_CMD_RESP_FRAME_RET_SIZE \
)

#define METER_CMD_LENGTH_MAX_SIZE                           (255)

#define METER_CMD_REQ_FRAME_DATA_MAX_SIZE                   (\
    METER_CMD_LENGTH_MAX_SIZE - METER_CMD_LENGTH_REQ_FRAME_MIN_SIZE\
)

#define METER_CMD_RES_FRAME_DATA_MAX_SIZE                   (\
    METER_CMD_LENGTH_MAX_SIZE - METER_CMD_LENGTH_RES_FRAME_MIN_SIZE\
)

#define METER_CMD_SEND_BUFFER_MAX_SIZE                      (METER_CMD_HEADER_FOOTER_SIZE + METER_CMD_LENGTH_MAX_SIZE)
#define METER_CMD_GET_FRAME_CRC(rcv_buffer, length)         ((uint16_t)(*(rcv_buffer + length - 2)) | (*(rcv_buffer + length - 3) << 8))

#define METER_CMD_GET_REQ_FRAME_INFO(buffer, req_len, frame, req_info)      {\
    req_info.raw_buffer.data = buffer;\
    req_info.raw_buffer.size = req_len;\
    req_info.crc = METER_CMD_GET_FRAME_CRC(buffer, req_len);\
    req_info.frame_type = frame;\
    req_info.ret = METER_CMD_OK; /* Just to make the representation clearer, no use on request frame */ \
    req_info.code = (MeterCmdCode_t)*(buffer + METER_CMD_HEADER_SIZE + METER_CMD_MESSAGE_LENGTH_SIZE);\
    req_info.data_buffer.data = (buffer + METER_CMD_HEADER_SIZE + METER_CMD_MESSAGE_LENGTH_SIZE + METER_CMD_COMMAND_CODE_SIZE);\
    req_info.data_buffer.length = req_len - METER_CMD_LENGTH_REQ_FRAME_MIN_SIZE - METER_CMD_HEADER_FOOTER_SIZE;\
    req_info.data_buffer.decoded_length = 0;\
}

#define METER_CMD_INITIATE_RES_FRAME_INFO(req_info, res_buffer, res_info)   {\
    res_info.code = req_info.code;                                                                              \
    res_info.frame_type = FRAME_TYPE_RESPONSE;                                                                  \
    res_info.raw_buffer.data = res_buffer;                                                                      \
    res_info.data_buffer.data = res_buffer +                                                                    \
        METER_CMD_HEADER_SIZE +                                                                                 \
        METER_CMD_MESSAGE_LENGTH_SIZE +                                                                         \
        METER_CMD_COMMAND_CODE_SIZE +                                                                           \
        METER_CMD_RESP_FRAME_RET_SIZE;                                                                          \
    res_info.data_buffer.length = 0;                                                                            \
    res_info.data_buffer.decoded_length = 0;                                                                    \
}

#define METER_CMD_RESET_DATA_BUFFER(data_buffer) {\
    data_buffer.length = 0;\
    data_buffer.decoded_length = 0;\
}

#define METER_CMD_CHECK_EXPECTED_LENGTH(expected_length) {\
    if (p_req_buffer->length != expected_length) {                                                  \
        /* Expected length incorrect */                                                             \
        R_METER_CMD_PackAndSendErrorCode(p_frame_info, METER_CMD_ERROR_INCORRECT_ARGUMENT_LENGTH);  \
        return 0;                                                                                   \
    }                                                                                               \
}

#define METER_CMD_ASSERT_TRUE_SEND_ERROR_CODE_RET_VALUE(cond, code, value)      {                   \
    if ((cond) == TRUE) {                                                                           \
        R_METER_CMD_PackAndSendErrorCode(p_frame_info, code);                                       \
        return value;                                                                               \
    }\
}

#define METER_CMD_ALLOC_BUFFER(size)                                (WRP_EXT_Heap_Malloc((size)))
#define METER_CMD_FREE_BUFFER(x)                                    {if ( (x) != NULL ) { WRP_EXT_Heap_Free(x); x = NULL; } }

#define MTER_CMD_DECODE_BUFFER_FUNC_PROTOTYPE(frame_type, name)     frame_type name(uint8_t ** pp_rcv_buffer)
#define METER_CMD_PROCESS_FUNC_PROTOTYPE(name)                      uint8_t name(MeterCmdDataBuffer * p_req_buffer, MeterCmdFrameInfo * p_frame_info, uint8_t is_unlock_access)

/***********************************************************************************************************************
Variable Externs
***********************************************************************************************************************/

/***********************************************************************************************************************
Functions Prototypes
***********************************************************************************************************************/
/* Command process function */
METER_CMD_PROCESS_FUNC_PROTOTYPE(METER_CMD_ProcessCmdAuthentication);
METER_CMD_PROCESS_FUNC_PROTOTYPE(R_METER_CMD_ProcessCmdCalibration);
METER_CMD_PROCESS_FUNC_PROTOTYPE(R_METER_CMD_ProcessCmdMemoryFormat);
METER_CMD_PROCESS_FUNC_PROTOTYPE(R_METER_CMD_ProcessCmdSigLog);
METER_CMD_PROCESS_FUNC_PROTOTYPE(R_METER_CMD_ProcessCmdOcMemAccess);
METER_CMD_PROCESS_FUNC_PROTOTYPE(METER_CMD_ProcessCmdPlatformAccess);

/* Common frame handling */
void R_METER_CMD_PackAndSendResFrame(MeterCmdFrameInfo * p_frame_info);
void R_METER_CMD_PackAndSendErrorCode(MeterCmdFrameInfo * p_frame_info, MeterCmdRet_t ret);

/* Common buffer encode function */
void R_METER_CMD_EncodeUInt8ToBuffer(MeterCmdDataBuffer * p_data_buffer, uint8_t data);
void R_METER_CMD_EncodeUInt16ToBuffer(MeterCmdDataBuffer * p_data_buffer, uint16_t data);
void R_METER_CMD_EncodeUInt32ToBuffer(MeterCmdDataBuffer * p_data_buffer, uint32_t data);
void R_METER_CMD_EncodeFloat32ToBuffer(MeterCmdDataBuffer * p_data_buffer, float32_t data);
void R_METER_CMD_EncodeArrayUInt8ToBuffer(MeterCmdDataBuffer * p_data_buffer, uint8_t * p_data, uint8_t length);
void R_METER_CMD_EncodeDataTypeToBuffer(MeterCmdDataBuffer * p_data_buffer, void * p_data, uint8_t data_type, uint8_t array_length);

/* Common buffer decode function */
uint8_t R_METER_CMD_DecodeBufferToUInt8(MeterCmdDataBuffer * p_data_buffer);
uint16_t R_METER_CMD_DecodeBufferToUInt16(MeterCmdDataBuffer * p_data_buffer);
uint32_t R_METER_CMD_DecodeBufferToUInt32(MeterCmdDataBuffer * p_data_buffer);
float32_t R_METER_CMD_DecodeBufferToFloat32(MeterCmdDataBuffer * p_data_buffer);
void R_METER_CMD_DecodeBufferToArrayUInt8(MeterCmdDataBuffer * p_data_buffer, uint8_t * p_data, uint8_t length);
void R_METER_CMD_DecodeBufferToDataType(MeterCmdDataBuffer * p_data_buffer, void * p_data, uint8_t data_type, uint8_t array_length);

/* Export function from SigLog module */
void R_METER_CMD_SIGLOG_Reset(void);
uint8_t R_METER_CMD_SIGLOG_IsLogging(void);
void R_METER_CMD_SIGLOG_ADC_InterruptCallBack(NEAR_PTR EM_SAMPLES * p_samples);

#endif /* __R_METER_CMD_SHARE_H */