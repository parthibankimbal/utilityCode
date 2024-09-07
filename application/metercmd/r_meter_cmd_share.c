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
* File Name    : r_meter_cmd_share.c
* Version      : 1.00
* Device(s)    :
* Tool-Chain   :
* H/W Platform :
* Description  : Meter CMD - Share functions
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
***********************************************************************************************************************/
/* Driver */
#include "typedef.h"            /* GSCE Standard Typedef */
#include <stdlib.h>
#include <string.h>

/* Wrapper */
#include "wrp_user_ext.h"

/* Application */
#include "r_meter_cmd_share.h"

#if (defined METER_ENABLE_PROPRIETARY_METER_COMMAND) && (METER_ENABLE_PROPRIETARY_METER_COMMAND == TRUE)

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
#define METER_CMD_DECODE_BUFFER_GENERIC_4BYTE(p, data)     {\
                                                                *(((uint8_t *)&data) + 0) = *(p->data + current_decoded_length + 3);     \
                                                                *(((uint8_t *)&data) + 1) = *(p->data + current_decoded_length + 2);     \
                                                                *(((uint8_t *)&data) + 2) = *(p->data + current_decoded_length + 1);     \
                                                                *(((uint8_t *)&data) + 3) = *(p->data + current_decoded_length + 0);     \
}


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
* Function Name: r_meter_cmd_encode_decode_memory_failed
* Description  : Process when memory encode or decode failed
*              : Current action: restart mcu
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
static void r_meter_cmd_encode_decode_memory_failed(void)
{
    /* Fatal error in encoding or decoding function, force meter reset */
    WRP_EXT_SoftReset();
}

/***********************************************************************************************************************
* Function Name: r_meter_cmd_buffer_encode_length_update
* Description  : Update the length for encoding
* Arguments    : MeterCmdDataBuffer * p_data_buffer
*              : uint8_t length
* Return Value : None
***********************************************************************************************************************/
static void r_meter_cmd_buffer_encode_length_update(MeterCmdDataBuffer * p_data_buffer, uint8_t length)
{
    uint8_t updated_length = p_data_buffer->length + length;

    if (updated_length < p_data_buffer->length) {
        r_meter_cmd_encode_decode_memory_failed();
    }
    else {
        p_data_buffer->length = updated_length;
    }
}

/***********************************************************************************************************************
* Function Name: r_meter_cmd_buffer_decode_length_update
* Description  : Update the length for decoding
* Arguments    : MeterCmdDataBuffer * p_data_buffer
*              : uint8_t length
* Return Value : None
***********************************************************************************************************************/
static void r_meter_cmd_buffer_decode_length_update(MeterCmdDataBuffer * p_data_buffer, uint8_t length)
{
    uint8_t updated_length = p_data_buffer->decoded_length + length;

    if (updated_length > p_data_buffer->length ||
        updated_length < p_data_buffer->decoded_length)
    {
        r_meter_cmd_encode_decode_memory_failed();
    }
    else {
        p_data_buffer->decoded_length = updated_length;
    }
}

/***********************************************************************************************************************
* Function Name: R_METER_CMD_EncodeUInt8ToBuffer
* Description  : Encode a uint8_t variable to buffer
* Arguments    : MeterCmdDataBuffer * p_data_buffer
*              : uint8_t data
* Return Value : None
***********************************************************************************************************************/
void R_METER_CMD_EncodeUInt8ToBuffer(MeterCmdDataBuffer * p_data_buffer, uint8_t data)
{
    uint8_t current_length = p_data_buffer->length;

    r_meter_cmd_buffer_encode_length_update(p_data_buffer, 1);

    *(p_data_buffer->data + current_length) = data;
}

/***********************************************************************************************************************
* Function Name: R_METER_CMD_EncodeUInt16ToBuffer
* Description  : Encode a uint16_t variable to buffer
* Arguments    : MeterCmdDataBuffer * p_data_buffer
*              : uint16_t data
* Return Value : None
***********************************************************************************************************************/
void R_METER_CMD_EncodeUInt16ToBuffer(MeterCmdDataBuffer * p_data_buffer, uint16_t data)
{
    uint8_t current_length = p_data_buffer->length;

    r_meter_cmd_buffer_encode_length_update(p_data_buffer, 2);

    *(p_data_buffer->data + current_length + 0) = *(((uint8_t *)&data) + 1);
    *(p_data_buffer->data + current_length + 1) = *(((uint8_t *)&data) + 0);
}

/***********************************************************************************************************************
* Function Name: R_METER_CMD_EncodeUInt32ToBuffer
* Description  : Encode a uint32_t variable to buffer
* Arguments    : MeterCmdDataBuffer * p_data_buffer
*              : uint32_t data
* Return Value : None
***********************************************************************************************************************/
void R_METER_CMD_EncodeUInt32ToBuffer(MeterCmdDataBuffer * p_data_buffer, uint32_t data)
{
    uint8_t current_length = p_data_buffer->length;

    r_meter_cmd_buffer_encode_length_update(p_data_buffer, 4);

    *(p_data_buffer->data + current_length + 0) = *(((uint8_t *)&data) + 3);
    *(p_data_buffer->data + current_length + 1) = *(((uint8_t *)&data) + 2);
    *(p_data_buffer->data + current_length + 2) = *(((uint8_t *)&data) + 1);
    *(p_data_buffer->data + current_length + 3) = *(((uint8_t *)&data) + 0);
}

/***********************************************************************************************************************
* Function Name: R_METER_CMD_EncodeFloat32ToBuffer
* Description  : Encode a float32_t variable to buffer
* Arguments    : MeterCmdDataBuffer * p_data_buffer
*              : float32_t data
* Return Value : None
***********************************************************************************************************************/
void R_METER_CMD_EncodeFloat32ToBuffer(MeterCmdDataBuffer * p_data_buffer, float32_t data)
{
    uint8_t current_length = p_data_buffer->length;

    r_meter_cmd_buffer_encode_length_update(p_data_buffer, 4);

    *(p_data_buffer->data + current_length + 0) = *(((uint8_t *)&data) + 3);
    *(p_data_buffer->data + current_length + 1) = *(((uint8_t *)&data) + 2);
    *(p_data_buffer->data + current_length + 2) = *(((uint8_t *)&data) + 1);
    *(p_data_buffer->data + current_length + 3) = *(((uint8_t *)&data) + 0);
}

/***********************************************************************************************************************
* Function Name: R_METER_CMD_EncodeArrayUInt8ToBuffer
* Description  : Encode a uint8_t array to buffer
* Arguments    : MeterCmdDataBuffer * p_data_buffer
*              : uint8_t * p_data
*              : uint8_t length
* Return Value : None
***********************************************************************************************************************/
void R_METER_CMD_EncodeArrayUInt8ToBuffer(MeterCmdDataBuffer * p_data_buffer, uint8_t * p_data, uint8_t length)
{
    uint8_t i;
    uint8_t current_length = p_data_buffer->length;

    r_meter_cmd_buffer_encode_length_update(p_data_buffer, length);

    for (i = 0; i < length; i++)
    {
        *(p_data_buffer->data + current_length + i) = *(p_data + i);
    }
}

/***********************************************************************************************************************
* Function Name: R_METER_CMD_EncodeArrayFloat32ToBuffer
* Description  : Encode a float32_t array to buffer
* Arguments    : MeterCmdDataBuffer * p_data_buffer
*              : float32_t * p_data
*              : uint8_t length
* Return Value : None
***********************************************************************************************************************/
void R_METER_CMD_EncodeArrayFloat32ToBuffer(MeterCmdDataBuffer * p_data_buffer, float32_t * p_data, uint8_t length)
{
    while (length > 0) {
        R_METER_CMD_EncodeFloat32ToBuffer(p_data_buffer, *p_data++);
        length--;
    }
}

/***********************************************************************************************************************
* Function Name: R_METER_CMD_EncodeDataTypeToBuffer
* Description  : Encode a custom buffer with specified type to buffer
* Arguments    : MeterCmdDataBuffer * p_data_buffer
*              : void * p_data
*              : uint8_t data_type
*              : uint8_t array_length
* Return Value : None
***********************************************************************************************************************/
void R_METER_CMD_EncodeDataTypeToBuffer(MeterCmdDataBuffer * p_data_buffer, void * p_data, uint8_t data_type, uint8_t array_length)
{
    switch (data_type)
    {
    case METER_CMD_DATA_TYPE_UINT8:
        R_METER_CMD_EncodeUInt8ToBuffer(p_data_buffer, *((uint8_t *)p_data));
        break;
    case METER_CMD_DATA_TYPE_UINT16:
        R_METER_CMD_EncodeUInt16ToBuffer(p_data_buffer, *((uint16_t *)p_data));
        break;
    case METER_CMD_DATA_TYPE_UINT32:
        R_METER_CMD_EncodeUInt32ToBuffer(p_data_buffer, *((uint32_t *)p_data));
        break;
    case METER_CMD_DATA_TYPE_FLOAT32:
        R_METER_CMD_EncodeFloat32ToBuffer(p_data_buffer, *((float32_t *)p_data));
        break;
    case METER_CMD_DATA_TYPE_ARRAY_UINT8:
        R_METER_CMD_EncodeArrayUInt8ToBuffer(p_data_buffer, p_data, array_length);
        break;
    case METER_CMD_DATA_TYPE_ARRAY_FLOAT32:
        R_METER_CMD_EncodeArrayFloat32ToBuffer(p_data_buffer, p_data, array_length);
        break;
    default:
        break;
    }
}

/***********************************************************************************************************************
* Function Name: R_METER_CMD_DecodeBufferToUInt8
* Description  : Decode buffer data to uint8_t
* Arguments    : MeterCmdDataBuffer * p_data_buffer
* Return Value : uint8_t
***********************************************************************************************************************/
uint8_t R_METER_CMD_DecodeBufferToUInt8(MeterCmdDataBuffer * p_data_buffer)
{
    uint8_t data;
    uint8_t current_decoded_length = p_data_buffer->decoded_length;

    r_meter_cmd_buffer_decode_length_update(p_data_buffer, 1);

    data = *(p_data_buffer->data + current_decoded_length);

    return data;
}

/***********************************************************************************************************************
* Function Name: R_METER_CMD_DecodeBufferToUInt16
* Description  : Decode buffer data to uint16_t
* Arguments    : MeterCmdDataBuffer * p_data_buffer
* Return Value : uint16_t
***********************************************************************************************************************/
uint16_t R_METER_CMD_DecodeBufferToUInt16(MeterCmdDataBuffer * p_data_buffer)
{
    uint16_t data;
    uint8_t current_decoded_length = p_data_buffer->decoded_length;

    r_meter_cmd_buffer_decode_length_update(p_data_buffer, 2);

    *(((uint8_t *)&data) + 0) = *(p_data_buffer->data + current_decoded_length + 1);
    *(((uint8_t *)&data) + 1) = *(p_data_buffer->data + current_decoded_length + 0);

    return data;
}

/***********************************************************************************************************************
* Function Name: R_METER_CMD_DecodeBufferToUInt32
* Description  : Decode buffer data to uint32_t
* Arguments    : MeterCmdDataBuffer * p_data_buffer
* Return Value : uint32_t
***********************************************************************************************************************/
uint32_t R_METER_CMD_DecodeBufferToUInt32(MeterCmdDataBuffer * p_data_buffer)
{
    uint32_t data;
    uint8_t current_decoded_length = p_data_buffer->decoded_length;

    r_meter_cmd_buffer_decode_length_update(p_data_buffer, 4);

    METER_CMD_DECODE_BUFFER_GENERIC_4BYTE(p_data_buffer, data);

    return data;
}

/***********************************************************************************************************************
* Function Name: R_METER_CMD_DecodeBufferToFloat32
* Description  : Decode buffer data to float32_t
* Arguments    : MeterCmdDataBuffer * p_data_buffer
* Return Value : float32_t
***********************************************************************************************************************/
float32_t R_METER_CMD_DecodeBufferToFloat32(MeterCmdDataBuffer * p_data_buffer)
{
    float32_t data;
    uint8_t current_decoded_length = p_data_buffer->decoded_length;

    r_meter_cmd_buffer_decode_length_update(p_data_buffer, 4);

    METER_CMD_DECODE_BUFFER_GENERIC_4BYTE(p_data_buffer, data);

    return data;
}

/***********************************************************************************************************************
* Function Name: R_METER_CMD_DecodeBufferToArrayUInt8
* Description  : Decode buffer data to array of uint8_t type
* Arguments    : MeterCmdDataBuffer * p_data_buffer
*              : uint8_t * p_data
*              : uint8_t length
* Return Value : None
***********************************************************************************************************************/
void R_METER_CMD_DecodeBufferToArrayUInt8(MeterCmdDataBuffer * p_data_buffer, uint8_t * p_data, uint8_t length)
{
    uint8_t i;
    uint8_t current_decoded_length = p_data_buffer->decoded_length;

    r_meter_cmd_buffer_decode_length_update(p_data_buffer, length);

    for (i = 0; i < length; i++)
    {
        *(p_data + i) = *(p_data_buffer->data + current_decoded_length + i);
    }
}

/***********************************************************************************************************************
* Function Name: R_METER_CMD_DecodeBufferToArrayFloat32
* Description  : Decode buffer data to array of float32_t type
* Arguments    : MeterCmdDataBuffer * p_data_buffer
*              : float32_t * p_data
*              : uint8_t length
* Return Value : None
***********************************************************************************************************************/
void R_METER_CMD_DecodeBufferToArrayFloat32(MeterCmdDataBuffer * p_data_buffer, float32_t * p_data, uint8_t length)
{
    while (length > 0) {
        *p_data++ = R_METER_CMD_DecodeBufferToFloat32(p_data_buffer);
        length--;
    }
}

/***********************************************************************************************************************
* Function Name: R_METER_CMD_DecodeBufferToDataType
* Description  : Decode buffer data to output buffer with custom type
* Arguments    : MeterCmdDataBuffer * p_data_buffer
*              : void * p_data
*              : uint8_t data_type
*              : uint8_t length
* Return Value : None
***********************************************************************************************************************/
void R_METER_CMD_DecodeBufferToDataType(MeterCmdDataBuffer * p_data_buffer, void * p_data, uint8_t data_type, uint8_t array_length)
{
    switch (data_type)
    {
    case METER_CMD_DATA_TYPE_UINT8:
        *((uint8_t *)p_data) = R_METER_CMD_DecodeBufferToUInt8(p_data_buffer);
        break;
    case METER_CMD_DATA_TYPE_UINT16:
        *((uint16_t *)p_data) = R_METER_CMD_DecodeBufferToUInt16(p_data_buffer);
        break;
    case METER_CMD_DATA_TYPE_UINT32:
        *((uint32_t *)p_data) = R_METER_CMD_DecodeBufferToUInt32(p_data_buffer);
        break;
    case METER_CMD_DATA_TYPE_FLOAT32:
        *((float32_t *)p_data) = R_METER_CMD_DecodeBufferToFloat32(p_data_buffer);
        break;
    case METER_CMD_DATA_TYPE_ARRAY_UINT8:
        R_METER_CMD_DecodeBufferToArrayUInt8(p_data_buffer, p_data, array_length);
        break;
    case METER_CMD_DATA_TYPE_ARRAY_FLOAT32:
        R_METER_CMD_DecodeBufferToArrayFloat32(p_data_buffer, p_data, array_length);
        break;
    default:
        break;
    }
}

#endif