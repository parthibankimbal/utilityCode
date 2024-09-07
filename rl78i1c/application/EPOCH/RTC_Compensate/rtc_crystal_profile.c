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
* File Name    : rtc_crystal_profile.c
* Version      : 1.00
* Device(s)    : RL78/I1C
* Tool-Chain   : CA78K0R
* H/W Platform : RL78/I1C Energy Meter Platform
* Description  : This file declare crystal temperature profile.
***********************************************************************************************************************/

/***********************************************************************************************************************
Pragma directive
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "typedef.h"
#include "rtc_compensation.h"

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Imported global variables and functions (from other files)
***********************************************************************************************************************/

/***********************************************************************************************************************
Exported global variables and functions (to be accessed by other files)
***********************************************************************************************************************/
/*
 * CAUTION FOR USER TO FORM RTC TEMPERATURE
 *    The temperature range should be in range [25 to MAX]
 *    The temperature must be sorted from SMALL to LARGE value
 *
 * [Example]
 *    { 25  ,   ppm01  },       Start temperature in Crystal Profile
 *    { 26  ,   ppm02  },
 *    { 27  ,   ppm03  },
 *    { 28  ,   ppm04  },
 *    { 29  ,   ppm05  },
 *    { 30  ,   ppm06  },
 *    { 35  ,   ppm07  },
 *    { 40  ,   ppm08  },
 *    { 45  ,   ppm09  },
 *    { 50  ,   ppm10  },
 *    { 55  ,   ppm11  },
 *    { 56  ,   ppm12  },
 *    { 57  ,   ppm13  },
 *    { 60  ,   ppm14  },
 *    { 65  ,   ppm15  },
 *    { ..  ,   ....   },
 *    { 90  ,   ppmN   },       MAX temperature in Crystal Profile
 */
const rtc_freq_error_t  g_rtc_crystal_profile[] = 
{
    { 25    ,    0.000    },    /* Start temperature in RTC profile should be 25 */
    { 30    ,   -0.875    },
    { 35    ,   -3.500    },
    { 40    ,   -7.875    },
    { 45    ,   -14.000   },
    { 50    ,   -21.875   },
    { 55    ,   -31.500   },
    { 60    ,   -42.875   },
    { 65    ,   -56.000   },
    { 70    ,   -70.875   },
    { 75    ,   -87.500   },
    { 80    ,   -105.875  },
    { 85    ,   -126.000  },
    { 90    ,   -147.875  }     /* MAX support temperature in RTC profile */
};

const uint16_t g_crystal_profile_size = sizeof(g_rtc_crystal_profile) / sizeof(rtc_freq_error_t);
