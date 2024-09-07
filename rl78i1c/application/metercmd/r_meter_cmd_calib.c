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
* File Name    : r_meter_cmd_calib.c
* Version      : 1.00
* Device(s)    :
* Tool-Chain   :
* H/W Platform :
* Description  : Meter CMD - Auto calibration
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
***********************************************************************************************************************/
/* Driver */
#include "r_cg_macrodriver.h"   /* Macro Driver Definitions */
#include "r_cg_userdefine.h"    /* CG User Define */
#include "r_cg_dsadc.h"         /* DSADC Driver */
#include "r_cg_rtc.h"
#include "r_cg_wdt.h"           /* WDT Driver */

#include "typedef.h"            /* GSCE Standard Typedef */

#include <stdlib.h>
#include <string.h>

/* Middleware */
#include "em_core.h"

/* Wrapper */
#include "wrp_app_mcu.h"
#include "wrp_em_sw_config.h"

/* Application */
#include "platform.h"
#include "r_meter_cmd.h"
#include "r_meter_cmd_share.h"
#include "config_storage.h"
//#include "storage.h"

#if (defined METER_ENABLE_PROPRIETARY_METER_COMMAND) && (METER_ENABLE_PROPRIETARY_METER_COMMAND == TRUE)

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
#define EXPECTED_CALIB_ARGS_LENGTH                  (   sizeof(uint8_t) +\
                                                        sizeof(uint8_t) +\
                                                        sizeof(float32_t) +\
                                                        sizeof(float32_t) +\
                                                        sizeof(float32_t) +\
                                                        sizeof(uint8_t)\
)
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
* Function Name: r_meter_cmd_send_calib_output
* Description  : Encode and send the calibration output
* Arguments    : MeterCmdFrameInfo * p_frame_info
*              : EM_CALIB_OUTPUT * p_output
*              : uint8_t result
* Return Value : None
***********************************************************************************************************************/
void r_meter_cmd_send_calib_output(MeterCmdFrameInfo * p_frame_info, EM_CALIB_OUTPUT * p_output, uint8_t result)
{
    /* Prepare reply buffer */
    p_frame_info->ret = METER_CMD_OK;

    R_METER_CMD_EncodeUInt8ToBuffer(&p_frame_info->data_buffer, result);
    R_METER_CMD_EncodeFloat32ToBuffer(&p_frame_info->data_buffer, p_output->fs);
    R_METER_CMD_EncodeFloat32ToBuffer(&p_frame_info->data_buffer, p_output->gain);
    R_METER_CMD_EncodeFloat32ToBuffer(&p_frame_info->data_buffer, p_output->gain1);
    R_METER_CMD_EncodeFloat32ToBuffer(&p_frame_info->data_buffer, p_output->vcoeff);
    R_METER_CMD_EncodeFloat32ToBuffer(&p_frame_info->data_buffer, p_output->icoeff);
    R_METER_CMD_EncodeFloat32ToBuffer(&p_frame_info->data_buffer, p_output->pcoeff);
    R_METER_CMD_EncodeFloat32ToBuffer(&p_frame_info->data_buffer, p_output->angle_error);
    R_METER_CMD_EncodeFloat32ToBuffer(&p_frame_info->data_buffer, p_output->angle_error1);
    R_METER_CMD_EncodeUInt8ToBuffer(&p_frame_info->data_buffer, p_output->step);
    /* Send reply buffer */
    R_METER_CMD_PackAndSendResFrame(p_frame_info);
}

/***********************************************************************************************************************
* Function Name: R_METER_CMD_ProcessCmdCalibration
* Description  : Entry function to process for calibration command
* Arguments    : MeterCmdDataBuffer * p_req_buffer
*              : MeterCmdFrameInfo * p_frame_info
* Return Value : uint8_t: 0: if ok, others: error
***********************************************************************************************************************/
// uint8_t R_METER_CMD_ProcessCmdCalibration(MeterCmdDataBuffer * p_req_buffer, MeterCmdFrameInfo * p_frame_info, uint8_t is_unlock_access)
// {
//     /* Length: args */
//     /* Arguments layout
//      * 2byte: Cycle
//      * 2byte: Cycle phase
//      * 4byte: Imax
//      * 4byte: Reference Voltage
//      * 4byte: Reference Current
//      * 1byte: Wire
//     */
// #define R_CALIB_PROCESS_CALIBRATION_PROTECTED                   28
//     EM_CALIB_ARGS calib_args;
//     EM_CALIBRATION calib;
//     EM_CALIB_OUTPUT calib_output;
//     EM_CALIB_WORK calib_work;
//     dsad_reg_setting_t adc_gain_phase;
//     uint8_t rlt;
//     uint8_t w;
//     float32_t angle_list[EM_GAIN_PHASE_NUM_LEVEL_MAX];
//     float32_t gain_list[EM_GAIN_PHASE_NUM_LEVEL_MAX];

//     if (is_unlock_access == 0)
//     {
//       rlt = R_CALIB_PROCESS_CALIBRATION_PROTECTED;
//       R_METER_CMD_PackAndSendErrorCode(p_frame_info, METER_CMD_ERROR_EXECUTION_FAILED);
//     }
//     else
//       {
//       if (meter_calibration_init_callback != 0)
//       {
//         meter_calibration_init_callback();
//       }
//       /* Check parameter length */
//       METER_CMD_CHECK_EXPECTED_LENGTH(EXPECTED_CALIB_ARGS_LENGTH);
  
//       /* Get calibration parameters */
//       calib_args.cycle = R_METER_CMD_DecodeBufferToUInt8(p_req_buffer);
//       calib_args.cycle_angle = R_METER_CMD_DecodeBufferToUInt8(p_req_buffer);
//       calib_args.imax = R_METER_CMD_DecodeBufferToFloat32(p_req_buffer);
//       calib_args.v = R_METER_CMD_DecodeBufferToFloat32(p_req_buffer);
//       calib_args.i = R_METER_CMD_DecodeBufferToFloat32(p_req_buffer);
//       w = (EM_LINE)R_METER_CMD_DecodeBufferToUInt8(p_req_buffer);
//       if (w == 0)
//       {
//           calib_args.line_i = EM_LINE_PHASE;
//       }
//       else
//       {
//           calib_args.line_i = EM_LINE_NEUTRAL;
//       }
  
//       /* Fixed parameters */
//       calib_args.line_v = EM_LINE_PHASE;
//       calib_args.rtc_period = 500;
//       calib_args.max_gvalue = 64;
//       calib_args.stable_ndelay = 10;
  
//       /* Set imax=0 to let calib module do in dual gain
//       */
//   #if (EM_SW_PROPERTY_ADC_GAIN_PHASE_NUMBER_LEVEL != 1)
//       if (calib_args.line_i == EM_LINE_PHASE)
//       {
//           calib_args.imax = 0.0f;
//       }
//   #endif /* EM_SW_PROPERTY_ADC_GAIN_PHASE_NUMBER_LEVEL != 1*/
  
//   #if (EM_SW_PROPERTY_ADC_GAIN_NEUTRAL_NUMBER_LEVEL != 1)
//       if (calib_args.line_i == EM_LINE_NEUTRAL)
//       {
//           calib_args.imax = 0.0f;
//       }
//   #endif /* EM_SW_PROPERTY_ADC_GAIN_NEUTRAL_NUMBER_LEVEL != 1 */
  
//       calib = EM_GetCalibInfo();
  
//       /* Backup phase and reg settings */
//       R_DSADC_GetGainAndPhase(&adc_gain_phase);
  
//       /* Initiate calibration */
//       rlt = EM_CalibInitiate(&calib_args, &calib_work, &calib_output);
  
//       if (rlt != EM_OK) {
//           r_meter_cmd_send_calib_output(p_frame_info, &calib_output, rlt);
//       }
//       else {
//           /* Calibrating */
//           while (TRUE)
//           {
//               R_WDT_Restart();
  
//               rlt = EM_CalibRun();
  
//               /* Updating information to client */
//               if (rlt != EM_OK) {
//                   r_meter_cmd_send_calib_output(p_frame_info, &calib_output, rlt);
//               }
  
//               if (rlt != EM_CALIBRATING)
//               {
//                   break;
//               }
//           }
//       }
  
//       /* Restore gain and phase settings */
//       R_DSADC_SetGainAndPhase(adc_gain_phase);
  
//       /* Initialize the angle and gain list */
//       memset(&angle_list[0], 0, sizeof(angle_list));
//       memset(&gain_list[0], 0, sizeof(gain_list));
  
//       if (rlt == EM_OK)
//       {
//           calib.sampling_frequency = calib_output.fs;
//           calib.coeff.vrms = calib_output.vcoeff;
//           angle_list[0] = calib_output.angle_error;
//           gain_list[0] = calib_output.gain;
//           if (*((uint32_t *)&calib_args.imax) == 0)
//           {
//               angle_list[1] = calib_output.angle_error1;
//               gain_list[1] = calib_output.gain1;
//           }
//           if (calib_args.line_i == EM_LINE_PHASE)
//           {
//               calib.coeff.i1rms = calib_output.icoeff;
//               calib.coeff.active_power = calib_output.pcoeff;
//               calib.coeff.reactive_power = calib_output.pcoeff;
//               calib.coeff.apparent_power = calib_output.pcoeff;
//               calib.sw_phase_correction.i1_phase_degrees = angle_list;
//               calib.sw_gain.i1_gain_values = gain_list;
//           }
//   #ifdef METER_WRAPPER_ADC_COPY_NEUTRAL_SAMPLE
//           else
//           {
//               calib.coeff.i2rms = calib_output.icoeff;
//               calib.coeff.active_power2 = calib_output.pcoeff;
//               calib.coeff.reactive_power2 = calib_output.pcoeff;
//               calib.coeff.apparent_power2 = calib_output.pcoeff;
//               calib.sw_phase_correction.i2_phase_degrees = angle_list;
//               calib.sw_gain.i2_gain_values = gain_list;
//           }
//   #endif
  
//           {
//               /* Stop EM operation */
//               if (EM_Stop() != EM_OK)
//               {
//                   goto CALIBRATION_SETTING_FAILED;
//               }
  
//               /* Set calibrated data */
//               if (EM_SetCalibInfo(&calib) != EM_OK)
//               {
//                   goto CALIBRATION_SETTING_FAILED;
//               }
  
//               /* Driver ADC Gain (not set if using dual gain) */
//               if (*((uint32_t *)&calib_args.imax) != 0)
//               {
//                   if (calib_args.line_i == EM_LINE_PHASE) {
//                       R_DSADC_SetChannelGain(
//                           EM_ADC_DRIVER_CHANNEL_PHASE,
//                           R_DSADC_GetGainEnumValue((uint8_t)gain_list[0])
//                       );
//                   }
//   #ifdef METER_WRAPPER_ADC_COPY_NEUTRAL_SAMPLE
//                   else {
//                       R_DSADC_SetChannelGain(
//                           EM_ADC_DRIVER_CHANNEL_NEUTRAL,
//                           R_DSADC_GetGainEnumValue((uint8_t)gain_list[0])
//                       );
//                   }
//   #endif
//               }
  
//               /* Backup calibrated data into Storage Memory */
//               if (CONFIG_Backup(CONFIG_ITEM_CALIB) != CONFIG_OK)
//               {
//                   goto CALIBRATION_SETTING_FAILED;
//               }
  
//               /* Start EM operation */
//               if (EM_Start() != EM_OK)
//               {
//                   goto CALIBRATION_SETTING_FAILED;
//               }
//           }
  
//           r_meter_cmd_send_calib_output(p_frame_info, &calib_output, rlt);
//       }
//     }
//     return 0;

// CALIBRATION_SETTING_FAILED:
//     EM_Start();
//     R_METER_CMD_PackAndSendErrorCode(p_frame_info, METER_CMD_ERROR_EXECUTION_FAILED);

//     return 1;
// }


uint8_t R_METER_CMD_ProcessCmdCalibration(MeterCmdDataBuffer * p_req_buffer, MeterCmdFrameInfo * p_frame_info, uint8_t is_unlock_access)
{
    /* Length: args */
    /* Arguments layout
     * 2byte: Cycle
     * 2byte: Cycle phase
     * 4byte: Imax
     * 4byte: Reference Voltage
     * 4byte: Reference Current
     * 1byte: Wire
    */
#define R_CALIB_PROCESS_CALIBRATION_PROTECTED                   28
    EM_CALIB_ARGS calib_args;
    EM_CALIBRATION calib;
    EM_CALIB_OUTPUT calib_output;
    EM_CALIB_WORK calib_work;
    dsad_reg_setting_t adc_gain_phase;
    uint8_t rlt;
    uint8_t w;
    float32_t angle_list[EM_GAIN_PHASE_NUM_LEVEL_MAX];
    float32_t gain_list[EM_GAIN_PHASE_NUM_LEVEL_MAX];

    if (is_unlock_access == 0)
    {
      rlt = R_CALIB_PROCESS_CALIBRATION_PROTECTED;
      R_METER_CMD_PackAndSendErrorCode(p_frame_info, METER_CMD_ERROR_EXECUTION_FAILED);
    }
    else
    {
      if (meter_calibration_init_callback != 0)
      {
        meter_calibration_init_callback();
      }
      /* Check parameter length */
      METER_CMD_CHECK_EXPECTED_LENGTH(EXPECTED_CALIB_ARGS_LENGTH);
  
      /* Get calibration parameters */
      calib_args.cycle = R_METER_CMD_DecodeBufferToUInt8(p_req_buffer);
      calib_args.cycle_angle = R_METER_CMD_DecodeBufferToUInt8(p_req_buffer);
      calib_args.imax = R_METER_CMD_DecodeBufferToFloat32(p_req_buffer);
      calib_args.v = R_METER_CMD_DecodeBufferToFloat32(p_req_buffer);
      calib_args.i = R_METER_CMD_DecodeBufferToFloat32(p_req_buffer);
      w = (EM_LINE)R_METER_CMD_DecodeBufferToUInt8(p_req_buffer);

      /* Fixed parameters */
      calib_args.line_v = EM_LINE_PHASE;
      calib_args.rtc_period = 500;
      calib_args.max_gvalue = 64;
      calib_args.stable_ndelay = 10;
  
      calib = EM_GetCalibInfo();  
  
      /* Set imax=0 to let calib module do in dual gain
      */
  #if (EM_SW_PROPERTY_ADC_GAIN_PHASE_NUMBER_LEVEL != 1)
      if (calib_args.line_i == EM_LINE_PHASE)
      {
          calib_args.imax = 0.0f;
      }
  #endif /* EM_SW_PROPERTY_ADC_GAIN_PHASE_NUMBER_LEVEL != 1*/
  
  #if (EM_SW_PROPERTY_ADC_GAIN_NEUTRAL_NUMBER_LEVEL != 1)
      if (calib_args.line_i == EM_LINE_NEUTRAL)
      {
          calib_args.imax = 0.0f;
      }
  #endif /* EM_SW_PROPERTY_ADC_GAIN_NEUTRAL_NUMBER_LEVEL != 1 */
  
      do
      {        
        if (w == 4)
        {
            calib_args.line_i = EM_LINE_PHASE;
            w--;
        }
        else if (w == 3)
        {
            calib_args.line_i = EM_LINE_NEUTRAL;
            w = 0;
        }
        else
        {
            calib_args.line_i = EM_LINE_PHASE;
            w = 0;
        }
        
        /* Backup phase and reg settings */
        R_DSADC_GetGainAndPhase(&adc_gain_phase);
    
        /* Initiate calibration */
        rlt = EM_CalibInitiate(&calib_args, &calib_work, &calib_output);
    
        if (rlt != EM_OK) {
            r_meter_cmd_send_calib_output(p_frame_info, &calib_output, rlt);
        }
        else {
            /* Calibrating */
            while (TRUE)
            {
                R_WDT_Restart();
    
                rlt = EM_CalibRun();
    
                /* Updating information to client */
                if (rlt != EM_OK) {
                    r_meter_cmd_send_calib_output(p_frame_info, &calib_output, rlt);
                }
    
                if (rlt != EM_CALIBRATING)
                {
                    break;
                }
            }
        }
    
        /* Restore gain and phase settings */
        R_DSADC_SetGainAndPhase(adc_gain_phase);
    
        /* Initialize the angle and gain list */
        memset(&angle_list[0], 0, sizeof(angle_list));
        memset(&gain_list[0], 0, sizeof(gain_list));
    
        if (rlt == EM_OK)
        {
            calib.sampling_frequency = calib_output.fs;
            calib.coeff.vrms = calib_output.vcoeff;
            angle_list[0] = calib_output.angle_error;
            gain_list[0] = calib_output.gain;
            if (*((uint32_t *)&calib_args.imax) == 0)
            {
                angle_list[1] = calib_output.angle_error1;
                gain_list[1] = calib_output.gain1;
            }
            if (calib_args.line_i == EM_LINE_PHASE)
            {
                calib.coeff.i1rms = calib_output.icoeff;
                calib.coeff.active_power = calib_output.pcoeff;
                calib.coeff.reactive_power = calib_output.pcoeff;
                calib.coeff.apparent_power = calib_output.pcoeff;
                calib.sw_phase_correction.i1_phase_degrees = angle_list;
                calib.sw_gain.i1_gain_values = gain_list;
            }
    #ifdef METER_WRAPPER_ADC_COPY_NEUTRAL_SAMPLE
            else
            {
                calib.coeff.i2rms = calib_output.icoeff;
                calib.coeff.active_power2 = calib_output.pcoeff;
                calib.coeff.reactive_power2 = calib_output.pcoeff;
                calib.coeff.apparent_power2 = calib_output.pcoeff;
                calib.sw_phase_correction.i2_phase_degrees = angle_list;
                calib.sw_gain.i2_gain_values = gain_list;
            }
    #endif
    
            {
                /* Stop EM operation */
                if (EM_Stop() != EM_OK)
                {
                    goto CALIBRATION_SETTING_FAILED;
                }
    
                /* Set calibrated data */
                if (EM_SetCalibInfo(&calib) != EM_OK)
                {
                    goto CALIBRATION_SETTING_FAILED;
                }
    
                /* Driver ADC Gain (not set if using dual gain) */
                if (*((uint32_t *)&calib_args.imax) != 0)
                {
                    if (calib_args.line_i == EM_LINE_PHASE) {
                        R_DSADC_SetChannelGain(
                            EM_ADC_DRIVER_CHANNEL_PHASE,
                            R_DSADC_GetGainEnumValue((uint8_t)gain_list[0])
                        );
                    }
    #ifdef METER_WRAPPER_ADC_COPY_NEUTRAL_SAMPLE
                    else {
                        R_DSADC_SetChannelGain(
                            EM_ADC_DRIVER_CHANNEL_NEUTRAL,
                            R_DSADC_GetGainEnumValue((uint8_t)gain_list[0])
                        );
                    }
    #endif
                }
    
                /* Backup calibrated data into Storage Memory */
                if (CONFIG_Backup(CONFIG_ITEM_CALIB) != CONFIG_OK)
                {
                    goto CALIBRATION_SETTING_FAILED;
                }
    
                /* Start EM operation */
                if (EM_Start() != EM_OK)
                {
                    goto CALIBRATION_SETTING_FAILED;
                }
            }    
            r_meter_cmd_send_calib_output(p_frame_info, &calib_output, rlt);
        }
      } while (w);
    }
    return 0;

CALIBRATION_SETTING_FAILED:
    EM_Start();
    R_METER_CMD_PackAndSendErrorCode(p_frame_info, METER_CMD_ERROR_EXECUTION_FAILED);

    return 1;
}


#endif