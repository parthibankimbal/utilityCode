/***********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only 
* intended for use with Renesas products. No other uses are authorized. This 
* software is owned by Renesas Electronics Corporation and is protected under 
* all applicable laws, including copyright laws.
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING 
* THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT 
* LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE 
* AND NON-INFRINGEMENT.  ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.
* TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS 
* ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE 
* FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR 
* ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE 
* BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software 
* and to discontinue the availability of this software.  By using this software, 
* you agree to the additional terms and conditions found by accessing the 
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2011, 2012 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/

/***********************************************************************************************************************
* File Name    : rtc_compensation.c
* Version      : 
* Device(s)    : RL78/I1C
* Tool-Chain   : CA78K0R
* Description  : This file implements source file for RTC temperature compensation.
* Creation Date: 
***********************************************************************************************************************/

/***********************************************************************************************************************
Pragma directive
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
/* Driver layer */
#include "r_cg_macrodriver.h"
#include "r_cg_12adc.h"
#include "r_cg_tmps.h"
#include "r_cg_rtc.h"
#include "r_cg_wdt.h"

/* Code Standard */
#include "typedef.h"            /* GSCE Standard Typedef */

/* MW, WRP layer */
#include "wrp_app_mcu.h"

/* Application layer */
#include "rtc_compensation.h"

//float PPM_Calculated = 0;
//float PPM_Ajdusted = 0;
//float RTC_COMPENSATION_PPM_OFFSET       =      (56.5f);
/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
/* Start temperature of crystal profile */
#define RTC_COMPENSATION_START_TEMP             (25)

/* Default setting of temperature sensor */
//#define TMPS_VBGR                               (3.25f)                 /* Band-gap internal reference voltage default: 1.50V */
//float TMPS_VBGR               =                 3.14f;                 /* Band-gap internal reference voltage default: 1.50V */
float TMPS_VBGR               =                 2.00f; //3.14f;                 /* Band-gap internal reference voltage default: 1.50V */
#define ADC_STEPS				(4095.0f)		/* Total number of steps for 12bit SAR ADC */	

/* Define max value of RTC minute counter */
#if (RTC_CONST_PERIOD == 500)
    #define RTC_COMP_ONE_MINUTE_COUNT    120
#elif (RTC_CONST_PERIOD == 1000)
    #define RTC_COMP_ONE_MINUTE_COUNT    60
#else
    #error "RTC constant period must be 0.5s or 1s"
#endif

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Imported global variables and functions (from other files)
***********************************************************************************************************************/

/* Imported from rtc_crystal_profile.c */
extern const rtc_freq_error_t   g_rtc_crystal_profile[];
extern const uint16_t           g_crystal_profile_size;

/***********************************************************************************************************************
Exported global variables and functions (to be accessed by other files)
***********************************************************************************************************************/
unsigned int 	g_comp_flag_ad_conversion_end = 0;
uint16_t temps_buffer = 0;

/***********************************************************************************************************************
Private global variables and functions
***********************************************************************************************************************/

/* Variables */
static volatile uint8_t                 g_comp_flag_start = 0;
static float                    g_rtc_calibration_value = 0;

/* Private function prototypes */
static float32_t RTC_COMP_LookupFreqErr(int16_t temp);                                              /* Lookup frequency error from measured temperature */
static float32_t RTC_COMP_Interpolate(int16_t temp);                                                /* Interpolate to get freq error at unspecific temperature */
static uint16_t RTC_COMP_GetSensorADCValue(void);                                                   /* Read ADC value */
static float32_t RTC_COMP_GetSensorVoltageValue(uint16_t temps_counter);                            /* Convert from ADC value to voltage value */
/*static*/ float32_t RTC_COMP_GetTemperature(uint8_t *out_mode, uint16_t *out_counter, float32_t * out_vs);    /* Get the current temperature reading */



uint8_t isInvalidFloat(float value) {
    union {
        float f;
        uint32_t i;
    } u;
    
    uint32_t exponent;
    uint32_t fraction;
    
    u.f = value;
    
    exponent = (u.i & 0x7F800000) >> 23;
    fraction = u.i & 0x007FFFFF;

    return (uint8_t)(((exponent == 0xFF) && (fraction != 0)) || ((exponent == 0xFF) && (fraction == 0)));
}
/***********************************************************************************************************************
* Function Name: void RTC_COMP_calibrate(float value)
* Description  : RTC crystals have different resonating frequency due to tolerance in the manufacturing.
* This routine is use to calibrated the RTC module. Deviation is measured by an external instrument and then fed to this function.
* Arguments    : float value = deviation measured by the instrument.
* Return Value : None
***********************************************************************************************************************/
void RTC_COMP_calib(float value, uint8_t add)
{
    g_comp_flag_start = 1;
    RTC_COMPENSATION_MEM_Read(CONFIG_STORAGE_USER_INFO_RTC_CALIBRATION_VALUE_ADDR, (uint8_t*)&g_rtc_calibration_value, CONFIG_STORAGE_USER_INFO_RTC_CALIBRATION_VALUE_SIZE);
    if((g_rtc_calibration_value > 200.0) || (g_rtc_calibration_value < - 200.0) || isInvalidFloat(g_rtc_calibration_value))
    {
      g_rtc_calibration_value = 0;
    }
    if(add)
    {
      g_rtc_calibration_value += value;
    }
    else
    {
      value -= RTC_COMPENSATION_PPM_OFFSET;
      g_rtc_calibration_value = value;
    }
    if ((g_rtc_calibration_value + RTC_COMPENSATION_PPM_OFFSET) > 190.0)
    {
        g_rtc_calibration_value = 190.0 - RTC_COMPENSATION_PPM_OFFSET;
    }
    else if ((g_rtc_calibration_value + RTC_COMPENSATION_PPM_OFFSET) < -190.0)
    {
        g_rtc_calibration_value = -190.0 - RTC_COMPENSATION_PPM_OFFSET;
    }
    RTC_COMPENSATION_MEM_Write(CONFIG_STORAGE_USER_INFO_RTC_CALIBRATION_VALUE_ADDR, (uint8_t*)&g_rtc_calibration_value, CONFIG_STORAGE_USER_INFO_RTC_CALIBRATION_VALUE_SIZE);
}

/***********************************************************************************************************************
* Function Name: float get_RTC_COMP_value(void)
* Description  : RTC crystals have different resonating frequency due to tolerance in the manufacturing.
* This routine is used to get the compensation value in PPM. 
* Arguments    : None.
* Return Value : float compensation value
***********************************************************************************************************************/
float get_RTC_COMP_value(void)
{
    return g_rtc_calibration_value;
}

/***********************************************************************************************************************
* Function Name: static float32_t RTC_COMP_LookupFreqErr(int16_t temp)
* Description  : Get the frequency error from measured temperature
* Arguments    : temp : measure temperature
* Return Value : frequency error
***********************************************************************************************************************/
static float32_t RTC_COMP_LookupFreqErr(int16_t temp)
{
    uint16_t                i;
    const rtc_freq_error_t  *p_max;
    const rtc_freq_error_t  *p_min;

    /* Calculate temperature value will be used to scan */
    if (temp < RTC_COMPENSATION_START_TEMP)
    {
        temp = 2 * RTC_COMPENSATION_START_TEMP - temp;
    }
    
    /* Get max, min input temperature */
    p_min = &g_rtc_crystal_profile[0];
    p_max = &g_rtc_crystal_profile[g_crystal_profile_size - 1]; 
    
    /* Check threshold temperature */
    if (temp >= (p_max->temperature))
    {
        return p_max->ppm;
    }
    else if (temp <= (p_min->temperature))
    {
        return p_min->ppm;
    }
    
    /* 
     * Scan temperature and choose frequency error 
     */
    
    /* Scan to choose the same temperature */
    for (i = 0; i < g_crystal_profile_size; i++)
    {
        if (temp == g_rtc_crystal_profile[i].temperature)
        {
            return g_rtc_crystal_profile[i].ppm;
        }
    }
    
    /* No the same temperature in profile, do interpolation */
    return RTC_COMP_Interpolate(temp);
}

/***********************************************************************************************************************
* Function Name: static float32_t RTC_COMP_Interpolate(int16_t temp)
* Description  : Do interpolation to get ppm value
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
static float32_t RTC_COMP_Interpolate(int16_t temp)
{
    float32_t               ppm = 0;
    uint16_t                i;
    const rtc_freq_error_t  *temp1 = NULL;  /* Near point 1, used to linear interpolate */
    const rtc_freq_error_t  *temp2 = NULL;  /* Near point 2, used to linear interpolate */
    
    /* Scan positive branch until meet the temperature larger than measured temperature */
    for (i = 1; i < g_crystal_profile_size; i++)
    {
        if (g_rtc_crystal_profile[i].temperature >= temp)
        {
            /* Get temp1 value */
            temp1 = &g_rtc_crystal_profile[i - 1];
            
            /* Get temp2 value */
            temp2 = &g_rtc_crystal_profile[i];
            
            break;
        }
    }
    
    /* Apply linear interpolation formula */
    if (temp1 != NULL && temp2 != NULL)
    {
        ppm = temp1->ppm + (temp2->ppm - temp1->ppm) * (temp - temp1->temperature) / (temp2->temperature - temp1->temperature);
    }
    
    return ppm;
}

/***********************************************************************************************************************
* Function Name: void RTC_COMP_Init(void)
* Description  : Initialize rtc compensation
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void RTC_COMP_Init(void)
{
    g_comp_flag_start = 1;
    RTC_COMPENSATION_MEM_Read(CONFIG_STORAGE_USER_INFO_RTC_CALIBRATION_VALUE_ADDR, (uint8_t*)&g_rtc_calibration_value, CONFIG_STORAGE_USER_INFO_RTC_CALIBRATION_VALUE_SIZE);
    if ((g_rtc_calibration_value > 200.0) || (g_rtc_calibration_value < -200.0) || isInvalidFloat(g_rtc_calibration_value))
    {
        g_rtc_calibration_value = 0;
    }
}

/***********************************************************************************************************************
* Function Name: static uint16_t RTC_COMP_GetSensorVoltageValue(void)
* Description  : Get measured voltage from Internal Sensor
* Arguments    : mode : Operation mode of Internal Sensor
* Return Value : uint16_t : voltage value
***********************************************************************************************************************/
static uint16_t RTC_COMP_GetSensorADCValue(void)
{
    uint16_t    temps_counter;
    float32_t   vs;
    uint8_t     i;
    uint8_t     timeout = 10;
    
    g_comp_flag_ad_conversion_end = 0;
    R_12ADC_Start();
	
    //Wait for ADC stabilization
    MCU_Delay(50);
    
    /* Read the temperature result counter then convert */
    while((g_comp_flag_ad_conversion_end == 0) && (timeout != 0))
    {
      R_WDT_Restart();
      timeout--;
      MCU_Delay(50);
    }
    
    /* Read the temperature result counter then convert */
    R_12ADC_Get_ValueResult(ADTEMPERSENSOR, &temps_counter);
    
    return temps_counter;
}


/***********************************************************************************************************************
* Function Name: static float32_t RTC_COMP_GetSensorVoltageValue(void)
* Description  : Get measured voltage from Internal Sensor
* Arguments    : mode : Operation mode of Internal Sensor
* Return Value : float32_t : voltage value
***********************************************************************************************************************/
static float32_t RTC_COMP_GetSensorVoltageValue(uint16_t temps_counter)
{
    return (( (float32_t)temps_counter * TMPS_VBGR ) / ADC_STEPS);
}

/***********************************************************************************************************************
* Function Name : RTC_COMP_GetTemperature
* Interface     : float32_t RTC_COMP_GetTemperature(uint8_t *out_mode, uint16_t *out_counter, float32_t * out_vs)
* Description   : Get the temperature from temperature sensor module
* Arguments     : None
* Return Value  : float32_t: Acquired temperature value
***********************************************************************************************************************/
/*static*/ float32_t RTC_COMP_GetTemperature(uint8_t *out_mode, uint16_t *out_counter, float32_t * out_vs)
{
    float32_t   temps_degree;
    uint16_t    temps_counter;
    float32_t   vs;
    uint8_t     mode;
    uint8_t     selected_mode;
    /* 
     * Get the reading
     */
    R_TMPS_Start();
    R_TMPS_SetMode(TMPS_MODE2);
    
     /* Read ADC result register */
    temps_counter = RTC_COMP_GetSensorADCValue();
	
    g_comp_flag_ad_conversion_end = 0;
    
    /* Stop the TMPS to conserve power */
    R_TMPS_Stop();
    
    /* Convert to voltage value */
    vs = RTC_COMP_GetSensorVoltageValue(temps_counter);
    
    /* Get the temperature with following formula 
     *                     (Vs - v1)     (V)
     * degree.C = 1000 * ----------------------- + t1
     *                      (Slope)   (mv/deg.c)
    */
    temps_degree = (1000.0f * ((vs - TMPS_MODE2_V1) / TMPS_MODE2_SLOPE)) + TMPS_MODE2_T1;
    
    
    /* Output additional information */
    if (out_mode != NULL)
    {
        *out_mode = TMPS_MODE2;
    }
    
    if (out_counter != NULL)
    {
        *out_counter = temps_counter;
    }
    
    if (out_vs != NULL)
    {
        *out_vs = vs;
    }
    
    /* Return the read temperature */
    return temps_degree;
}


/***********************************************************************************************************************
* Function Name: void RTC_COMP_PollingProcessing(void)
* Description  : RTC compensation polling processing
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void RTC_COMP_PollingProcessing(void)
{
    uint8_t     mode;
    uint16_t    temps_counter;
    float32_t   vs;
    float32_t   ppm = 0;
    float32_t   temps_degree;
    
    /* Elapsed a period of compensation? */
    if (g_comp_flag_start)
    {
        /* 
         * STEP 1: Get the temperature value
         */
        temps_degree = RTC_COMP_GetTemperature(&mode, &temps_counter, &vs);
        
        /*
         * STEP 2: Look up ppm value on crystal profile,
         * based the the temperature has been read
         */
        //TODO: Rakesh
        //ppm = RTC_COMP_LookupFreqErr((int16_t)temps_degree);
        //ppm = (temps_degree-25)*(temps_degree-25)*(CRYSTAL_CURVE_COFF);
        //PPM_Calculated = ppm;
        /*
         * STEP 3: Adding center frequency error offset,
         * based on user manual calibration input
         */
        ppm += RTC_COMPENSATION_PPM_OFFSET;
        ppm += g_rtc_calibration_value;
        
        /*
         * STEP 4: Choose dynamic range, based on ppm has been calculated
         * then do compensation on RCR2 and RADJ register
         */
        //PPM_Ajdusted = ppm;
        R_RTC_Compensate(ppm);
        
        /* Clear as ack */
        g_comp_flag_start = 0;
    }
}

/***********************************************************************************************************************
* Function Name: void RTC_COMP_ConstInterruptCallback(void)
* Description  : RTC Const Period Callback for minute counting
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void RTC_COMP_ConstInterruptCallback(void)
{
    static uint16_t second_counter = 0;
    static uint16_t minute_counter = 0;
    
    second_counter++;
    if (second_counter >= RTC_COMP_ONE_MINUTE_COUNT)
    {    
        //g_comp_flag_start = 1;   
        minute_counter++;
        if (minute_counter >= RTC_COMPENSATION_PERIOD)
        {
            /* Mark to trigger a start to do compensation */
            //EI();
            g_comp_flag_start = 1;
            
            /* Next counting */
            minute_counter = 0;
        }
        
        /* Next counting */
        second_counter = 0;
    }   
}
