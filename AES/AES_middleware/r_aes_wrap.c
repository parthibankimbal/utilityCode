/******************************************************************************
* DISCLAIMER

* This software is supplied by Renesas Electronics Corporation and is only
* intended for use with Renesas products. No other uses are authorized.

* This software is owned by Renesas Electronics Corporation and is protected under
* all applicable laws, including copyright laws.

* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES
* REGARDING THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY,
* INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
* PARTICULAR PURPOSE AND NON-INFRINGEMENT.  ALL SUCH WARRANTIES ARE EXPRESSLY
* DISCLAIMED.

* TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS
* ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE
* FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES
* FOR ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR ITS
* AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

* Renesas reserves the right, without notice, to make changes to this
* software and to discontinue the availability of this software.
* By using this software, you agree to the additional terms and
* conditions found by accessing the following link:
* http://www.renesas.com/disclaimer
******************************************************************************/
/* Copyright (C) 2011 Renesas Electronics Corporation. All rights reserved.  */
/******************************************************************************
* File Name    : r_aes_wrap.c
* Version      : 1.00
* Device(s)    : None
* Tool-Chain   :
* H/W Platform : Unified Energy Meter Platform
* Description  :
******************************************************************************
* History : DD.MM.YYYY Version Description
*         : 07.07.2014
******************************************************************************/

/******************************************************************************
Includes   <System Includes> , "Project Includes"
******************************************************************************/
#include "r_aes_hwip.h"
#include "r_aes_keywrap.h"
#include <string.h>

/******************************************************************************
Macro definitions
******************************************************************************/

/******************************************************************************
Typedef definitions
******************************************************************************/

/******************************************************************************
Imported global variables and functions (from other files)
******************************************************************************/

/******************************************************************************
Exported global variables and functions (to be accessed by other files)
******************************************************************************/

/******************************************************************************
Private global variables and functions
******************************************************************************/

/******************************************************************************
* Function Name : R_Aes_128_Wrap
* Interface     : int R_Aes_128_Wrap(const u8 *kek, int n, const u8 *plain, u8 *cipher)
* Description   : Wrap keys with AES Key Wrap Algorithm (128-bit KEK) (RFC3394)
* Arguments     : const u8 * kek  : 16-octet Key encryption key (KEK)
*               : int n           : Length of the plaintext key in 64-bit units; e.g., 2 = 128-bit = 16 bytes
*               : const u8 * plain: Plaintext key to be wrapped, n * 64 bits
*               : u8 * cipher     : Wrapped key, (n + 1) * 64 bits
* Return Value  : 0 on success, -1 on failure
******************************************************************************/
int16_t R_Aes_128_Wrap(const uint8_t *kek, int16_t n, const uint8_t *plain, uint8_t *cipher)
{
    uint8_t *a, *r, b[16];
    int16_t i, j;
    // uint8_t ekey128[AES_EKEY128_LEN];

    a = cipher;
    r = cipher + 8;

    /* 1) Initialize variables. */
    memset(a, 0xa6, 8);
    memcpy(r, plain, 8 * n);

    //R_Aes_128_Keysch(kek, (uint32_t near *)ekey128);

    /* 2) Calculate intermediate values.
     * For j = 0 to 5
     *     For i=1 to n
     *         B = AES(K, A | R[i])
     *         A = MSB(64, B) ^ t where t = (n*j)+i
     *         R[i] = LSB(64, B)
     */
    for (j = 0; j <= 5; j++) {
        r = cipher + 8;
        for (i = 1; i <= n; i++) {
            memcpy(b, a, 8);
            memcpy(b + 8, r, 8);

            R_Aes_Init();
            R_Aes_128_Ecbenc(b, b, (uint8_t *)kek, 1);
            R_Aes_Close();

            memcpy(a, b, 8);
            a[7] ^= (uint8_t)(n * j + i);
            memcpy(r, b + 8, 8);
            r += 8;
        }
    }

    /* 3) Output the results.
     *
     * These are already in @cipher due to the location of temporary
     * variables.
     */

    return 0;
}
