/***********************************************************************************************************************
* Copyright (C) 2015 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/

/***********************************************************************************************************************
* File Name	   : r_aes_hwip.h
* Description  : Header file of AES Hardware IP Library.
***********************************************************************************************************************/
#ifndef __r_aes_hwip_h__
#define __r_aes_hwip_h__

/***********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
***********************************************************************************************************************/
//#include "r_stdint.h"
#include "r_cg_macrodriver.h"


/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
#define AES_KEY128_LEN		(16)		/* 128 bits (16 bytes) */
#define AES_KEY192_LEN		(24)		/* 192 bits (24 bytes) */
#define AES_KEY256_LEN		(32)		/* 256 bits (32 bytes) */
#define AES_BLOCK_SIZE		(16)		/* 128 bits (16 bytes) */
#define AES_IVEC_SIZE		(16)		/* 128 bits (16 bytes) */

#define AES_EKEY128_LEN		(44)		/* 176 bytes (44*4(uint32_t)) */
#define AES_EKEY192_LEN		(52)		/* 208 bytes (52*4(uint32_t)) */
#define AES_EKEY256_LEN		(60)		/* 240 bytes (60*4(uint32_t)) */

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Exported global variables
***********************************************************************************************************************/
//extern const mw_version_t R_aes_version;

/***********************************************************************************************************************
Exported global variables (to be accessed by other files)
***********************************************************************************************************************/
void R_Aes_Init(void);
void R_Aes_Close(void);

void R_Aes_128_Cbcenc(uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint16_t);
void R_Aes_128_Cbcdec(uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint16_t);
void R_Aes_128_Ecbenc(uint8_t* input, uint8_t* output, uint8_t* key, uint16_t length);
void R_Aes_128_Ecbdec(uint8_t* input, uint8_t* output, uint8_t* key, uint16_t length);

void R_Aes_192_Cbcenc(uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint16_t);
void R_Aes_192_Cbcdec(uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint16_t);
void R_Aes_192_Ecbenc(uint8_t*, uint8_t*, uint8_t*, uint16_t);
void R_Aes_192_Ecbdec(uint8_t*, uint8_t*, uint8_t*, uint16_t);

void R_Aes_256_Cbcenc(uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint16_t);
void R_Aes_256_Cbcdec(uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint16_t);
void R_Aes_256_Ecbenc(uint8_t*, uint8_t*, uint8_t*, uint16_t);
void R_Aes_256_Ecbdec(uint8_t*, uint8_t*, uint8_t*, uint16_t);

#endif
