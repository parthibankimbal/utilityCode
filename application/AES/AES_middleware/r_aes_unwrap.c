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
* File Name    : r_aes_unwrap.c
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
* Function Name : R_Aes_128_Unwrap
* Interface     : int16_t R_Aes_128_Unwrap(const uint8_t *kek, int16_t n, const uint8_t *cipher, uint8_t *plain)
* Description   : Unwrap key with AES Key Wrap Algorithm (128-bit KEK) (RFC3394)
* Arguments     : const uint8_t * kek   : Key encryption key (KEK)
*               : int16_t n             : Length of the plaintext key in 64-bit units; e.g., 2 = 128-bit = 16 bytes
*               : const uint8_t * cipher: Wrapped key to be unwrapped, (n + 1) * 64 bits
*               : uint8_t * plain       : Plaintext key, n * 64 bits
* Return Value  : int16_t, 0 on success, -1 on failure (e.g., integrity verification failed)
******************************************************************************/
int16_t R_Aes_128_Unwrap(const uint8_t *kek, int16_t n, const uint8_t *cipher, uint8_t *plain)
{
    uint8_t a[8], *r, b[16];
    int16_t i, j;
    //uint8_t ekey128[AES_EKEY128_LEN];

    /* 1) Initialize variables. */
    memcpy(a, cipher, 8);
    r = plain;
    memcpy(r, cipher + 8, 8 * n);

    //R_Aes_128_Keysch(kek, (uint32_t near *)ekey128);

    /* 2) Compute intermediate values.
     * For j = 5 to 0
     *     For i = n to 1
     *         B = AES-1(K, (A ^ t) | R[i]) where t = n*j+i
     *         A = MSB(64, B)
     *         R[i] = LSB(64, B)
     */
    for (j = 5; j >= 0; j--) {
        r = plain + (n - 1) * 8;
        for (i = n; i >= 1; i--) {
            memcpy(b, a, 8);
            b[7] ^= (uint8_t)(n * j + i);

            memcpy(b + 8, r, 8);

            R_Aes_Init();
            R_Aes_128_Ecbdec(b, b, (uint8_t *)kek, 1);
            R_Aes_Close();

            memcpy(a, b, 8);
            memcpy(r, b + 8, 8);
            r -= 8;
        }
    }

    /* 3) Output results.
     *
     * These are already in @plain due to the location of temporary
     * variables. Just verify that the IV matches with the expected value.
     */
    for (i = 0; i < 8; i++) {
        if (a[i] != 0xa6)
            return -1;
    }

    return 0;
}
