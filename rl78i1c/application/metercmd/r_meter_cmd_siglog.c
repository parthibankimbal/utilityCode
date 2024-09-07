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
* File Name    : r_meter_cmd_siglog.c
* Version      : 1.00
* Device(s)    :
* Tool-Chain   :
* H/W Platform :
* Description  : Meter CMD - Signal logging
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

#if (defined METER_ENABLE_PROPRIETARY_METER_COMMAND) && (METER_ENABLE_PROPRIETARY_METER_COMMAND == TRUE)

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/
typedef struct tagMeterCmdSigLogInfo
{
    uint8_t enabled;
    uint8_t num_of_signals;
    uint16_t signal_selection;
    uint16_t sets_of_sample;

    uint16_t backup_sps;
    uint16_t backup_sdr;
} MeterCmdSigLogInfo;

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
/* Information used for communication only */
#define EXPECTED_SIGLOG_ARGS_LENGTH                 (   sizeof(uint16_t) +\
                                                        sizeof(uint16_t) \
)

#define METER_CMD_SIGLOG_RET_OK                         (0x01)
#define METER_CMD_SIGLOG_ERROR_MAX_SIGNALS              (0x02)
#define METER_CMD_SIGLOG_ERROR_INVALID_SIGNALS          (0x03)
#define METER_CMD_SIGLOG_ERROR_ZERO_ARGUMENTS           (0x04)

#define ELEVATED_UART_BAUD_RATE                         (1500000)

#if (METER_CMD_SIGLOG_SUPPORTED_NUM_OF_SIGNALS > 9)
#error "Invalid supported number of signals, the current elevated baudrate support 9 channel only
#endif

#define METER_CMD_SIGLOG_UART_mn_TX_CH0                 00
#define METER_CMD_SIGLOG_UART_mn_TX_CH1                 02
#define METER_CMD_SIGLOG_UART_mn_TX_CH2                 10
#define METER_CMD_SIGLOG_UART_mn_TX_CH3                 12

#define METER_CMD_SIGLOG_UART_m_CH0                     0
#define METER_CMD_SIGLOG_UART_m_CH1                     0
#define METER_CMD_SIGLOG_UART_m_CH2                     1
#define METER_CMD_SIGLOG_UART_m_CH3                     1

#define METER_CMD_SIGLOG_UART_SEND(byte)                {\
    JOIN(TXD, METER_CMD_SIGLOG_UART_CHANNEL) = byte;\
}
#define METER_CMD_SIGLOG_WAIT_UNTIL_SENT()              {\
    while(JOIN(SSR, JOIN(METER_CMD_SIGLOG_UART_mn_TX_CH, METER_CMD_SIGLOG_UART_CHANNEL)) & 0x0040);\
}
#define METER_CMD_SIGLOG_UART_START()                   {\
    JOIN(JOIN(R_UART, METER_CMD_SIGLOG_UART_CHANNEL), _Start());\
}
#define METER_CMD_SIGLOG_UART_STOP()                    {\
    JOIN(JOIN(R_UART, METER_CMD_SIGLOG_UART_CHANNEL), _Stop());\
}

#define METER_CMD_SIGLOG_UART_BACKUP_BAUDRATE_SETTING() {\
    g_meter_cmd_siglog_info.backup_sps = JOIN(SPS, JOIN(METER_CMD_SIGLOG_UART_m_CH, METER_CMD_SIGLOG_UART_CHANNEL));\
    g_meter_cmd_siglog_info.backup_sdr = JOIN(SDR, JOIN(METER_CMD_SIGLOG_UART_mn_TX_CH, METER_CMD_SIGLOG_UART_CHANNEL));\
}
#define METER_CMD_SIGLOG_UART_APPLY_1_5MBAUD() {\
    if (JOIN(SMR, JOIN(METER_CMD_SIGLOG_UART_mn_TX_CH, METER_CMD_SIGLOG_UART_CHANNEL)) & 0x8000) {                                      \
        JOIN(SPS, JOIN(METER_CMD_SIGLOG_UART_m_CH, METER_CMD_SIGLOG_UART_CHANNEL)) &= 0xFF0F;                                           \
        JOIN(SPS, JOIN(METER_CMD_SIGLOG_UART_m_CH, METER_CMD_SIGLOG_UART_CHANNEL)) |= (METER_CMD_SIGLOG_SPS_DIVIDER_SPS_CLK_6M << 4);   \
    }                                                                                                                                   \
    else {                                                                                                                              \
        JOIN(SPS, JOIN(METER_CMD_SIGLOG_UART_m_CH, METER_CMD_SIGLOG_UART_CHANNEL)) &= 0xFFF0;                                           \
        JOIN(SPS, JOIN(METER_CMD_SIGLOG_UART_m_CH, METER_CMD_SIGLOG_UART_CHANNEL)) |= METER_CMD_SIGLOG_SPS_DIVIDER_SPS_CLK_6M;          \
    }                                                                                                                                   \
    JOIN(SDR, JOIN(METER_CMD_SIGLOG_UART_mn_TX_CH, METER_CMD_SIGLOG_UART_CHANNEL)) = 0x0200;                                            \
}
#define METER_CMD_SIGLOG_UART_DISABLE_INTERRUPT() {\
    JOIN(STMK, METER_CMD_SIGLOG_UART_CHANNEL) = 1U;\
}
#define METER_CMD_SIGLOG_UART_RESTORE_BAUDRATE_SETTING() {\
    JOIN(SPS, JOIN(METER_CMD_SIGLOG_UART_m_CH, METER_CMD_SIGLOG_UART_CHANNEL)) = g_meter_cmd_siglog_info.backup_sps;\
    JOIN(SDR, JOIN(METER_CMD_SIGLOG_UART_mn_TX_CH, METER_CMD_SIGLOG_UART_CHANNEL)) = g_meter_cmd_siglog_info.backup_sdr;\
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
static volatile MeterCmdSigLogInfo g_meter_cmd_siglog_info;

/***********************************************************************************************************************
* Function Name: r_meter_cmd_send_signal_uart_sync
* Description  : Send a 4 byte buffer to uart in synchronous mode
* Arguments    : uint8_t * buffer: send buffer, need to be 4 byte
* Return Value : None
***********************************************************************************************************************/
void r_meter_cmd_send_signal_uart_sync(uint8_t * buffer)
{
    /* Send 4 byte synchronously (one signal) */
    METER_CMD_SIGLOG_UART_SEND((*buffer++));
    METER_CMD_SIGLOG_WAIT_UNTIL_SENT();
    METER_CMD_SIGLOG_UART_SEND((*buffer++));
    METER_CMD_SIGLOG_WAIT_UNTIL_SENT();
    METER_CMD_SIGLOG_UART_SEND((*buffer++));
    METER_CMD_SIGLOG_WAIT_UNTIL_SENT();
    METER_CMD_SIGLOG_UART_SEND((*buffer++));
    METER_CMD_SIGLOG_WAIT_UNTIL_SENT();
}

/***********************************************************************************************************************
* Function Name: r_meter_cmd_siglog_driver_start
* Description  : Prepare the signal logging uart driver
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void r_meter_cmd_siglog_driver_start(void)
{
    METER_CMD_SIGLOG_UART_STOP();

    /* Backup and apply new baudrate settings */
    METER_CMD_SIGLOG_UART_BACKUP_BAUDRATE_SETTING();
    METER_CMD_SIGLOG_UART_APPLY_1_5MBAUD();

    METER_CMD_SIGLOG_UART_START();
    METER_CMD_SIGLOG_UART_DISABLE_INTERRUPT();
}

/***********************************************************************************************************************
* Function Name: r_meter_cmd_siglog_driver_stop
* Description  : Restore the state of uart driver before doing signal logging
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void r_meter_cmd_siglog_driver_stop(void)
{
    METER_CMD_SIGLOG_UART_STOP();

    /* Restore backed-up baudrate settings */
    METER_CMD_SIGLOG_UART_RESTORE_BAUDRATE_SETTING();

    METER_CMD_SIGLOG_UART_START();
}

/***********************************************************************************************************************
* Function Name: R_METER_CMD_SIGLOG_Reset
* Description  : Reset the state of signal logging
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_METER_CMD_SIGLOG_Reset(void)
{
    memset((uint8_t *)&g_meter_cmd_siglog_info, 0, sizeof(MeterCmdSigLogInfo));
}

/***********************************************************************************************************************
* Function Name: R_METER_CMD_SIGLOG_IsLogging
* Description  : Check whether signal logging is enabled or not
* Arguments    : None
* Return Value : uint8_t: TRUE or FALSE
***********************************************************************************************************************/
uint8_t R_METER_CMD_SIGLOG_IsLogging(void)
{
    return g_meter_cmd_siglog_info.enabled;
}

/***********************************************************************************************************************
* Function Name: R_METER_CMD_SIGLOG_ADC_InterruptCallBack
* Description  : Signal logging ADC Interrupt end processing to send signal over uart line
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_METER_CMD_SIGLOG_ADC_InterruptCallBack(NEAR_PTR EM_SAMPLES * p_samples)
{
    R_WDT_Restart();

#if (METER_CMD_SIGLOG_SUPPORTED_NUM_OF_SIGNALS >= 1)
    if (g_meter_cmd_siglog_info.signal_selection & 0x0001)
    {
        r_meter_cmd_send_signal_uart_sync((uint8_t *)&METER_CMD_SIGLOG_VALUE_SIGNAL1);
    }
#endif

#if (METER_CMD_SIGLOG_SUPPORTED_NUM_OF_SIGNALS >= 2)
    if (g_meter_cmd_siglog_info.signal_selection & 0x0002)
    {
        r_meter_cmd_send_signal_uart_sync((uint8_t *)&METER_CMD_SIGLOG_VALUE_SIGNAL2);
    }
#endif

#if (METER_CMD_SIGLOG_SUPPORTED_NUM_OF_SIGNALS >= 3)
    if (g_meter_cmd_siglog_info.signal_selection & 0x0004)
    {
        r_meter_cmd_send_signal_uart_sync((uint8_t *)&METER_CMD_SIGLOG_VALUE_SIGNAL3);
    }
#endif

#if (METER_CMD_SIGLOG_SUPPORTED_NUM_OF_SIGNALS >= 4)
    if (g_meter_cmd_siglog_info.signal_selection & 0x0008)
    {
        r_meter_cmd_send_signal_uart_sync((uint8_t *)&METER_CMD_SIGLOG_VALUE_SIGNAL4);
    }
#endif

#if (METER_CMD_SIGLOG_SUPPORTED_NUM_OF_SIGNALS >= 5)
    if (g_meter_cmd_siglog_info.signal_selection & 0x0010)
    {
        r_meter_cmd_send_signal_uart_sync((uint8_t *)&METER_CMD_SIGLOG_VALUE_SIGNAL5);
    }
#endif

#if (METER_CMD_SIGLOG_SUPPORTED_NUM_OF_SIGNALS >= 6)
    if (g_meter_cmd_siglog_info.signal_selection & 0x0020)
    {
        r_meter_cmd_send_signal_uart_sync((uint8_t *)&METER_CMD_SIGLOG_VALUE_SIGNAL6);
    }
#endif

#if (METER_CMD_SIGLOG_SUPPORTED_NUM_OF_SIGNALS >= 7)
    if (g_meter_cmd_siglog_info.signal_selection & 0x0040)
    {
        r_meter_cmd_send_signal_uart_sync((uint8_t *)&METER_CMD_SIGLOG_VALUE_SIGNAL7);
    }
#endif

#if (METER_CMD_SIGLOG_SUPPORTED_NUM_OF_SIGNALS >= 8)
    if (g_meter_cmd_siglog_info.signal_selection & 0x0080)
    {
        r_meter_cmd_send_signal_uart_sync((uint8_t *)&METER_CMD_SIGLOG_VALUE_SIGNAL8);
    }
#endif

#if (METER_CMD_SIGLOG_SUPPORTED_NUM_OF_SIGNALS >= 9)
    if (g_meter_cmd_siglog_info.signal_selection & 0x0100)
    {
        r_meter_cmd_send_signal_uart_sync((uint8_t *)&METER_CMD_SIGLOG_VALUE_SIGNAL9);
    }
#endif

    g_meter_cmd_siglog_info.sets_of_sample--;
    if (g_meter_cmd_siglog_info.sets_of_sample == 0)
    {
        g_meter_cmd_siglog_info.enabled = FALSE;
    }
}

/***********************************************************************************************************************
* Function Name: R_METER_CMD_ProcessCmdSigLog
* Description  : Entry function to process for signal logging command
* Arguments    : MeterCmdDataBuffer * p_req_buffer
*              : MeterCmdFrameInfo * p_frame_info
* Return Value : uint8_t: 0: if ok, others: error
***********************************************************************************************************************/
uint8_t R_METER_CMD_ProcessCmdSigLog(MeterCmdDataBuffer * p_req_buffer, MeterCmdFrameInfo * p_frame_info, uint8_t is_unlock_access)
{
    /* Length: args */
    /* Arguments layout
     * 1byte: number of signals to log
     * 2byte: sets of samples
    */
    uint8_t is_param_good = FALSE;
    uint16_t i, temp;

    /* Check parameter length */
    METER_CMD_CHECK_EXPECTED_LENGTH(EXPECTED_SIGLOG_ARGS_LENGTH);

    /* Get calibration parameters */
    g_meter_cmd_siglog_info.signal_selection = R_METER_CMD_DecodeBufferToUInt16(p_req_buffer);
    g_meter_cmd_siglog_info.sets_of_sample = R_METER_CMD_DecodeBufferToUInt16(p_req_buffer);

    temp = 1;
    g_meter_cmd_siglog_info.num_of_signals = 0;
    for (i = 0; i < 16; i++)
    {
        if (temp & g_meter_cmd_siglog_info.signal_selection) {
            g_meter_cmd_siglog_info.num_of_signals++;
        }
        temp <<= 1;
    }

    temp = 1;
    for (i = 0; i < (METER_CMD_SIGLOG_SUPPORTED_NUM_OF_SIGNALS - 1); i++)
    {
        temp |= (temp << 1);
    }

    p_frame_info->ret = METER_CMD_OK;
    if (g_meter_cmd_siglog_info.num_of_signals > METER_CMD_SIGLOG_SUPPORTED_NUM_OF_SIGNALS) {
        R_METER_CMD_EncodeUInt8ToBuffer(&p_frame_info->data_buffer, METER_CMD_SIGLOG_ERROR_MAX_SIGNALS);
    }
    else if ((g_meter_cmd_siglog_info.signal_selection & (~temp)) != 0) {
        R_METER_CMD_EncodeUInt8ToBuffer(&p_frame_info->data_buffer, METER_CMD_SIGLOG_ERROR_INVALID_SIGNALS);
    }
    else if ((g_meter_cmd_siglog_info.num_of_signals == 0) ||
        (g_meter_cmd_siglog_info.sets_of_sample == 0)) {
        R_METER_CMD_EncodeUInt8ToBuffer(&p_frame_info->data_buffer, METER_CMD_SIGLOG_ERROR_ZERO_ARGUMENTS);
    }
    else {
        is_param_good = TRUE;
        R_METER_CMD_EncodeUInt8ToBuffer(&p_frame_info->data_buffer, METER_CMD_SIGLOG_RET_OK);
    }

    /* Encode max number of signals */
    R_METER_CMD_EncodeUInt8ToBuffer(&p_frame_info->data_buffer, METER_CMD_SIGLOG_SUPPORTED_NUM_OF_SIGNALS);

    /* Encode sampling rate */
    R_METER_CMD_EncodeUInt16ToBuffer(&p_frame_info->data_buffer, 3906);

    /* Encode elevated baudrate */
    R_METER_CMD_EncodeUInt32ToBuffer(&p_frame_info->data_buffer, ELEVATED_UART_BAUD_RATE);

    /* Send reply buffer */
    R_METER_CMD_PackAndSendResFrame(p_frame_info);

    /* Raise signal if parameter is correct */
    if (is_param_good == TRUE)
    {
        /* Reconfigure the driver for transfer */
        r_meter_cmd_siglog_driver_start();

        /* Delay around 500ms */
        for (i = 0; i < 10; i++)
        {
            MCU_Delay(10000);
            R_WDT_Restart();
        }

        /* Raise signal for sampling */
        g_meter_cmd_siglog_info.enabled = TRUE;

        /* Lock application, wait until finish sending signal */
        while (g_meter_cmd_siglog_info.enabled == TRUE)
        {
            R_WDT_Restart();
        }

        /* Restart application uart driver */
        r_meter_cmd_siglog_driver_stop();
    }

    return 0;
}

#endif