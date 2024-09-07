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
* File Name    : credit.c
* Version      : 1.00
* Device(s)    : RL78/I1C
* Tool-Chain   : CA78K0R
* H/W Platform : RL78/I1C Energy Meter Platform
* Description  : EM Display Application Layer APIs
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
***********************************************************************************************************************/
#include "SingleWireOp.h"
/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
#define SAMPLE_COUNT  40
#define SAMPLE_NO_11		1
/***********************************************************************************************************************
Imported global variables and functions (from other files)
***********************************************************************************************************************/
void kick_watchdog(void);
/******************************************************************************
Exported global variables and functions (to be accessed by other files)
******************************************************************************/
int16_t i16PhBuf[SAMPLE_COUNT];
int16_t i16NeuBuf[SAMPLE_COUNT];
#ifdef SAMPLE_NO_01
int16_t i16PhBufOffset[SAMPLE_COUNT] = {   -23,    -22,    -19,    -19,    -19,    -21,    -22,    -17,    -16,    -20,    -23,    -19,    -19,    -18,    -20,    -18,    -17,    -20,    -18,    -18,    -19,    -19,    -16,    -15,    -13,    -14,    -16,    -16,    -17,    -18,    -19,    -16,    -16,    -16,    -16,    -19,    -15,    -15,    -18,    -15};
int16_t i16NeuBufOffset[SAMPLE_COUNT] = {   100,    101,    100,     97,     98,     99,     98,     97,     98,     94,     94,     93,     91,     94,     92,     91,     90,     90,     89,     88,     88,     86,     86,     86,     84,     85,     81,     82,     83,     81,     80,     79,     80,     79,     78,     79,     77,     77,     75,     76};
#elif SAMPLE_NO_02
int16_t i16PhBufOffset[SAMPLE_COUNT] = {  -124,   -121,   -124,   -119,   -119,   -122,   -123,   -118,   -115,   -118,   -117,   -118,   -112,   -113,   -115,   -111,   -108,   -113,   -113,   -108,   -106,   -107,   -106,   -106,   -107,   -105,   -103,   -102,   -100,    -99,    -99,    -98,    -99,    -97,    -93,    -94,    -94,    -92,    -87,    -94};
int16_t i16NeuBufOffset[SAMPLE_COUNT] = {     0,      0,     -2,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,      0,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1};
#elif SAMPLE_NO_03
int16_t i16PhBufOffset[SAMPLE_COUNT] = {    18,     16,     15,     16,     19,     18,     14,     17,     19,     19,     15,     13,     13,     10,     13,     16,     12,     15,     12,     11,     12,     15,     14,     15,     15,     14,     12,     15,     11,     10,     16,     15,     15,     13,     13,     12,     17,     14,      6,      9};
int16_t i16NeuBufOffset[SAMPLE_COUNT] = {   137,    138,    135,    134,    133,    133,    134,    129,    128,    126,    127,    124,    124,    125,    125,    121,    121,    120,    117,    117,    115,    115,    114,    116,    117,    113,    112,    111,    109,    108,    106,    107,    106,    106,    104,    104,    104,    103,    102,     98};
#elif SAMPLE_NO_04
int16_t i16PhBufOffset[SAMPLE_COUNT] = {    41,     44,     46,     44,     45,     43,     46,     42,     42,     44,     44,     38,     41,     40,     37,     39,     37,     39,     36,     37,     37,     40,     40,     35,     35,     33,     34,     36,     36,     33,     32,     30,     33,     36,     29,     30,     30,     30,     37,     32};
int16_t i16NeuBufOffset[SAMPLE_COUNT] = {    94,     92,     92,     92,     91,     92,     88,     88,     89,     88,     89,     87,     88,     85,     84,     84,     84,     83,     83,     82,     80,     80,     80,     81,     79,     81,     81,     78,     76,     75,     75,     75,     74,     74,     74,     70,     72,     70,     70,     71};
#elif SAMPLE_NO_05
int16_t i16PhBufOffset[SAMPLE_COUNT] = {  -153,   -149,   -150,   -149,   -149,   -144,   -141,   -142,   -138,   -134,   -139,   -140,   -136,   -136,   -138,   -134,   -130,   -127,   -125,   -129,   -126,   -124,   -126,   -122,   -123,   -120,   -119,   -119,   -117,   -119,   -116,   -119,   -114,   -111,   -114,   -113,   -107,   -110,   -111,   -104};
int16_t i16NeuBufOffset[SAMPLE_COUNT] = {    51,     51,     51,     51,     51,     50,     48,     47,     50,     48,     46,     47,     48,     48,     48,     47,     47,     45,     43,     45,     42,     43,     45,     43,     44,     42,     41,     42,     39,     40,     40,     40,     40,     40,     39,     38,     38,     40,     39,     39};
#elif SAMPLE_NO_06
int16_t i16PhBufOffset[SAMPLE_COUNT] = {    56,     55,     59,     58,     56,     57,     54,     56,     51,     51,     55,     58,     56,     49,     50,     49,     52,     50,     49,     49,     42,     42,     47,     48,     49,     46,     46,     48,     43,     47,     48,     42,     45,     42,     45,     46,     37,     44,     44,     40};
int16_t i16NeuBufOffset[SAMPLE_COUNT] = {    80,     80,     78,     77,     78,     77,     77,     76,     75,     75,     74,     72,     75,     72,     71,     71,     71,     71,     71,     71,     69,     68,     68,     67,     67,     66,     66,     67,     64,     62,     63,     63,     64,     63,     60,     62,     62,     61,     59,     57};
#elif SAMPLE_NO_07
int16_t i16PhBufOffset[SAMPLE_COUNT] = {    75,     75,     74,     74,     69,     74,     71,     73,     73,     69,     68,     68,     66,     65,     63,     60,     64,     64,     65,     65,     67,     64,     61,     60,     59,     65,     62,     63,     59,     60,     55,     57,     59,     56,     58,     58,     60,     61,     54,     54};
int16_t i16NeuBufOffset[SAMPLE_COUNT] = {    55,     56,     56,     55,     54,     56,     56,     56,     53,     53,     55,     53,     53,     53,     52,     52,     52,     50,     48,     50,     49,     48,     48,     48,     48,     46,     47,     48,     48,     46,     44,     45,     44,     44,     45,     44,     44,     42,     43,     41};
#elif SAMPLE_NO_08
int16_t i16PhBufOffset[SAMPLE_COUNT] = {    82,     80,     79,     80,     78,     75,     74,     77,     79,     73,     72,     74,     78,     76,     72,     72,     75,     74,     67,     73,     71,     67,     65,     67,     73,     69,     64,     68,     64,     63,     62,     60,     62,     62,     61,     65,     61,     56,     60,     62};
int16_t i16NeuBufOffset[SAMPLE_COUNT] = {    79,     80,     78,     75,     76,     77,     76,     74,     73,     73,     72,     73,     71,     71,     71,     69,     70,     68,     68,     68,     69,     68,     65,     64,     65,     65,     63,     63,     63,     62,     59,     60,     61,     61,     60,     58,     59,     57,     58,     59};
#elif SAMPLE_NO_09
int16_t i16PhBufOffset[SAMPLE_COUNT] = {     2,      3,      2,     -1,      0,      2,      4,      1,      4,      1,      1,      1,      0,      5,      0,     -1,      3,      3,      3,      0,      0,      6,      4,      2,      1,      3,      2,      2,      4,      1,      0,     -1,      1,      3,      0,     -1,      4,      2,      2,      1};
int16_t i16NeuBufOffset[SAMPLE_COUNT] = {   157,    156,    154,    153,    152,    152,    151,    150,    148,    148,    147,    146,    144,    143,    140,    138,    138,    138,    138,    136,    136,    136,    130,    129,    132,    130,    129,    127,    125,    126,    124,    122,    124,    122,    120,    120,    118,    117,    117,    119};
#elif SAMPLE_NO_10
int16_t i16PhBufOffset[SAMPLE_COUNT] = {   -48,    -46,    -49,    -47,    -44,    -42,    -42,    -40,    -42,    -44,    -41,    -41,    -39,    -41,    -43,    -42,    -39,    -41,    -41,    -40,    -41,    -43,    -41,    -39,    -37,    -43,    -40,    -36,    -43,    -37,    -33,    -37,    -36,    -35,    -36,    -40,    -38,    -35,    -38,    -36};
int16_t i16NeuBufOffset[SAMPLE_COUNT] = {   119,    121,    120,    118,    119,    118,    115,    116,    115,    116,    112,    112,    112,    110,    110,    106,    107,    106,    105,    105,    103,    103,    103,    103,     99,     99,     99,    101,     98,     97,     98,     96,     95,     96,     92,     93,     93,     90,     90,     90};
#elif SAMPLE_NO_11
int16_t i16PhBufOffset[SAMPLE_COUNT] = {    30,     29,     33,     33,     30,     28,     28,     28,     24,     30,     28,     28,     28,     27,     28,     23,     26,     24,     25,     26,     25,     23,     24,     25,     22,     23,     25,     23,     23,     23,     23,     23,     25,     23,     24,     21,     23,     25,     20,     18};
int16_t i16NeuBufOffset[SAMPLE_COUNT] = {    -4,     -1,     -2,     -2,     -1,     -1,     -1,     -2,     -4,     -4,      0,      0,      1,     -2,     -1,      1,      0,      0,     -1,     -1,     -2,     -3,     -3,     -3,     -4,     -1,     -1,     -3,     -2,     -3,     -4,     -1,     -4,     -3,     -3,     -3,     -2,     -4,     -2,     -1};
#endif
uint16_t sampleCount = 0;
uint8_t dataReady = 0; 
uint8_t g_u8SingleWireTimeUpdate = 0;
uint32_t g_u32SleepTime = 0;
uint8_t g_uSingleWirePulsesEnable = 0;
/******************************************************************************
Private global variables and functions
******************************************************************************/

/***********************************************************************************************************************
* Function Name: R_DSADC_Create
* Description  : This function initializes the DSAD converter.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_DSADC_Create2(void)
{
    DSADCK = 0U;
    DSADRES = 1U;   /* reset DSAD converter */
    DSADRES = 0U;   /* reset release of DSAD converter */
    DSADCEN = 1U;   /* enables input clock supply */
    DSAMK = 1U;     /* disable INTDSAD interrupt */
    DSAIF = 0U;     /* clear INTDSAD interrupt flag */
    /* Set INTDSAD low priority */
    DSAPR1 = 0U;
    DSAPR0 = 0U;
    /* Set INTDSADDEC low priority */
    DSADECPR1 = 1U;
    DSADECPR0 = 1U;
    DSADMR = _8000_DSAD_SAMPLING_FREQUENCY_1 | _4000_DSAD_RESOLUTION_16BIT;
    DSADGCR0 = _40_DSAD_CH1_PGAGAIN_16;
    DSADGCR1 = _00_DSAD_CH2_PGAGAIN_1;
    DSADHPFCR = _C0_DSAD_CUTOFF_FREQUENCY_3 | _00_DSAD_CH2_HIGHPASS_FILTER_ENABLE | _00_DSAD_CH1_HIGHPASS_FILTER_ENABLE;
    DSADPHCR1 = _0000_DSAD_PHCR1_VALUE;
    DSADPHCR2 = _0000_DSAD_PHCR2_VALUE;
}
/***********************************************************************************************************************
* Function Name: R_DSADC_Start
* Description  : This function starts the DSAD converter.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_DSADC_Start2(void)
{
    DSAIF = 0U;     /* clear INTDSAD interrupt flag */
    DSAMK = 0U;     /* enable INTDSAD interrupt */
    DSADMR |= _0004_DSAD_CH2_OPERATION | _0002_DSAD_CH1_OPERATION;
}
/***********************************************************************************************************************
* Function Name: R_DSADC_Stop
* Description  : This function stops the DSAD converter.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_DSADC_Stop2(void)
{
    DSADMR &= (uint16_t)~(_0004_DSAD_CH2_OPERATION | _0002_DSAD_CH1_OPERATION);
    DSAMK = 1U;     /* disable INTDSAD interrupt */
    DSAIF = 0U;     /* clear INTDSAD interrupt flag */
}


/***********************************************************************************************************************
* Function Name: R_DSADC_Set_OperationOn
* Description  : This function power-on control.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_DSADC_Set_OperationOn2(void)
{
    DSADMR |= _0400_DSAD_CH2_POWER_ON | _0200_DSAD_CH1_POWER_ON;
}

/***********************************************************************************************************************
* Function Name: R_DSADC_Set_OperationOff
* Description  : This function power-down control.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_DSADC_Set_OperationOff2(void)
{
    DSADMR &= (uint16_t)~(_0400_DSAD_CH2_POWER_ON | _0200_DSAD_CH1_POWER_ON);
}


void accumulate_square(st_sq_accumulate* accumulate, int64_t sample)
{
	/*
    union 
	{
		unsigned short aa[4];
		unsigned long bb[2];
		unsigned long long cc;
	}long1;

	long1.bb[0]=sample;
	
	MULC = 0x40;
	MUL32SL = long1.aa[0];
	MUL32SH = long1.aa[1];

        long1.bb[1]=sample;
	
	MULBL = long1.aa[2];  
	MULBH = long1.aa[3];
  	
	__nop();                                   
        __nop();
 	__nop();                                   
        __nop();                              
        __nop();
 	__nop();                                   
        __nop();                              
        __nop();
 	__nop();                                   
        __nop();                              
        __nop();
 	__nop();                                   
        __nop();
        
	long1.aa[0] = MULR0;
	long1.aa[1] = MULR1;
	long1.aa[2] = MULR2;
	long1.aa[3] = MULR3;
	//accumulate->accumulated += long1.cc;
	*/
	accumulate->accumulated += (sample * sample);
	accumulate->count++;
}

/***********************************************************************************************************************
* Function Name    : uint8_t SingleWireCheck(void);
* Description      :
* Arguments        : None
* Functions Called : None
* Return Value     : status if single wire is present or not
***********************************************************************************************************************/
#define PH_CALIB_FACTOR  137.432687
#define NEU_CALIB_FACTOR  399.981859
#define SINGLE_WIRE_MIN_CURRENT     0.45
void SingleWireCheck(void)
{
    uint8_t u8SWCIndex = 0, u8SWCIndex2 = 60;
    st_sq_accumulate stAccumulatorPh = {0,0}, stAccumulatorNeu = {0,0};
    float fPhCurrent, fNeuCurrent;
    uint8_t cover_open = IS_COVER_OPEN();
    uint8_t rf_miss = IS_RF_LINK_MISSING();
    
    while (1)
    {
          kick_watchdog();
          EXTERNAL_WATCHDOG_PIN = 1;
          if (IS_SERIAL_NUMBER_UPDATED(0))
          {
              u8SWCIndex = 0;
              if (0 != u8SWCIndex2)
              {
                  u8SWCIndex2 -= 2;
                  u8SWCIndex = 5;
                  if (0 == u8SWCIndex2)
                  {
                      S_CAP_DISABLE();
                      EXTERNAL_WATCHDOG_PIN = 0;
                      RF_RST_ASSERT();
                  }
              }
              VRTCEN = 1;
              if (get_epoch() > g_u32SleepTime)
              {
                  if (0 != u8SWCIndex)
                  {
                      R_IT8Bit0_Channel0_Set_Interval(enTime_2s); 
                  }
                  else
                  {
                      R_IT8Bit0_Channel0_Set_Interval(enTime_60s);
                  }
              }
              else
              {
                  R_IT8Bit0_Channel0_Set_Interval(enTime_2s);
                  u8SWCIndex = 0;
              }
              VRTCEN = 0;
              R_IT8Bit0_Channel0_Start();
              EXTERNAL_WATCHDOG_PIN = 0;
          }
          else
          {
              S_CAP_DISABLE();
              RF_RST_ASSERT();
              EXTERNAL_WATCHDOG_PIN = 0;
              NOP();
              NOP();
              NOP();
          }
          STOP();                                                                                                                                                                  
          NOP();
          NOP();
          NOP();
          NOP();
          NOP();
          R_IT8Bit0_Channel0_Stop();
          
          if(IS_MAINS_PRESENT() /*|| (KEY_UP == KEY_PRESSED) || (KEY_DOWN == KEY_PRESSED) || (cover_open != IS_COVER_OPEN())*/ || (rf_miss != IS_RF_LINK_MISSING()) )//|| (st_tamper.tampers_bits.bits.single_wire == 1))
          {
            WDTE = 0x00;
          }
          else if (KEY_UP == KEY_PRESSED)
          {
              pushButton_Callback();
          }
          else if (5 == u8SWCIndex)
          {
              continue;
          }
          

          dataReady = 1;
          DSADCEN = 1;
          R_DSADC_Set_OperationOn2();
          R_DSADC_Start2();
          while(2 != dataReady);
          R_DSADC_Stop2();
          R_DSADC_Set_OperationOff2();
          DSADCEN = 0;
          dataReady = 0;
          
          //Process data here...
          stAccumulatorPh.accumulated = 0;
          stAccumulatorNeu.accumulated = 0;
          stAccumulatorPh.count = 0;
          stAccumulatorNeu.count = 0;
          for (u8SWCIndex = 0; u8SWCIndex < (SAMPLE_COUNT - 1); u8SWCIndex++)
          {
            accumulate_square(&stAccumulatorPh, i16PhBuf[u8SWCIndex] - i16PhBufOffset[u8SWCIndex]);
            accumulate_square(&stAccumulatorNeu, i16NeuBuf[u8SWCIndex] - i16NeuBufOffset[u8SWCIndex]);
          }
          fPhCurrent = sqrt(stAccumulatorPh.accumulated/stAccumulatorPh.count)
                        /PH_CALIB_FACTOR;
          fNeuCurrent = sqrt(stAccumulatorNeu.accumulated/stAccumulatorNeu.count)
                        /NEU_CALIB_FACTOR;

          //If current in any channel is present, then
          u8SWCIndex = 0;
          if ((fPhCurrent > SINGLE_WIRE_MIN_CURRENT) || (fNeuCurrent > SINGLE_WIRE_MIN_CURRENT) || (0 != g_u8SingleWireTimeUpdate))
          {
              EM_DisplaySequenceReset();
              //Hide Display
              break;
          }
          if (cover_open != IS_COVER_OPEN())
          {
              WDTE = 0x00;
          }
    }
}

void set_SingleWire_Offset(void)
{
    kick_watchdog();
    peripheralShutdown();
    R_IT8Bit0_Channel0_Set_Interval(enTime_1s);
    R_IT8Bit0_Channel0_Start();
    STOP();
    NOP();
    NOP();
    NOP();
    NOP();
    NOP();
    R_IT8Bit0_Channel0_Stop();

    dataReady = 1;
    DSADCEN = 1;
    R_DSADC_Set_OperationOn2();
    R_DSADC_Start2();
    while (2 != dataReady);
    R_DSADC_Stop2();
    R_DSADC_Set_OperationOff2();
    DSADCEN = 0;
    dataReady = 0;

    memcpy(i16PhBufOffset, i16PhBuf, sizeof(i16PhBuf));
    memcpy(i16NeuBufOffset, i16NeuBuf, sizeof(i16NeuBuf));

    kick_watchdog();
    R_CGC_OperateAt6MHz();
    R_PORT_Create();
    //R_DSADC_Create();
    startup();
    EPR_Init();
    //Store to EEPROM
   write_page_eeprom(STORAGE_EEPROM_NO_LOAD_OFFSET_PHASE_ADDR, (uint8_t*)i16PhBufOffset, STORAGE_EEPROM_NO_LOAD_OFFSET_PHASE_SIZE);
   write_page_eeprom(STORAGE_EEPROM_NO_LOAD_OFFSET_NEUTRAL_ADDR, (uint8_t*)i16NeuBufOffset, STORAGE_EEPROM_NO_LOAD_OFFSET_NEUTRAL_SIZE);
   write_eeprom(STORAGE_EEPROM_NO_LOAD_OFFSET_FLAG_ADDR, 0xA5);
    WDTE = 0x00;
}

uint8_t verify_SingleWire_Offset(void)
{
    uint8_t u8SWCIndex = 0, u8SWCIndex2 = 60;
    st_sq_accumulate stAccumulatorPh = { 0,0 }, stAccumulatorNeu = { 0,0 };
    float fPhCurrent, fNeuCurrent;
    kick_watchdog();
    peripheralShutdown();
    R_IT8Bit0_Channel0_Set_Interval(enTime_1s);
    R_IT8Bit0_Channel0_Start();
    STOP();
    NOP();
    NOP();
    NOP();
    NOP();
    NOP();
    R_IT8Bit0_Channel0_Stop();

    dataReady = 1;
    DSADCEN = 1;
    R_DSADC_Set_OperationOn2();
    R_DSADC_Start2();
    while (2 != dataReady);
    R_DSADC_Stop2();
    R_DSADC_Set_OperationOff2();
    DSADCEN = 0;
    dataReady = 0;

    //Process data here...
    stAccumulatorPh.accumulated = 0;
    stAccumulatorNeu.accumulated = 0;
    stAccumulatorPh.count = 0;
    stAccumulatorNeu.count = 0;
    for (u8SWCIndex = 0; u8SWCIndex < (SAMPLE_COUNT - 1); u8SWCIndex++)
    {
        accumulate_square(&stAccumulatorPh, i16PhBuf[u8SWCIndex] - i16PhBufOffset[u8SWCIndex]);
        accumulate_square(&stAccumulatorNeu, i16NeuBuf[u8SWCIndex] - i16NeuBufOffset[u8SWCIndex]);
    }
    fPhCurrent = sqrt(stAccumulatorPh.accumulated / stAccumulatorPh.count)
        / PH_CALIB_FACTOR;
    fNeuCurrent = sqrt(stAccumulatorNeu.accumulated / stAccumulatorNeu.count)
        / NEU_CALIB_FACTOR;

    kick_watchdog();
    R_CGC_OperateAt6MHz();
    R_PORT_Create();
    R_DSADC_Create();
    startup();
    OPTICAL_POWER_ENABLE();
    EPR_Init();

    if ((fPhCurrent > 0.1) || (fNeuCurrent > 0.1))
    {
        return 1;
    }
    return 0;
}

void pushButton_Callback(void)
{
    VRTCEN = 1;
    g_u32SleepTime = get_epoch() + SINGLE_WIRE_FAST_SCAN_MAX_TIME;       //For 24 hrs, it will wakeup each 2 seconds
    g_u8SingleWireTimeUpdate = 1;
}

void keyPressEnableDisplayAndPulses(void)
{
    singleWireEnablePulses();
    singleWireEnableDisplay();
    g_uSingleWirePulsesEnable = 1;
}

void singleWireEnablePulses(void)
{
    EM_PULSE_Init();
}
void singleWireDisablePulses(void)
{
    EM_PULSE_DeInit();
}
void singleWireEnableDisplay(void)
{
    if (0 == R_LCD_Status())
    {
        EM_DisplaySequenceReset();
    }
    R_LCD_Start();
}
void singleWireDisableDisplay(void)
{
    R_LCD_Stop();
}
