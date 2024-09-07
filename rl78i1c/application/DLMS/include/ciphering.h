//
// --------------------------------------------------------------------------
//  Gurux Ltd
//
//
//
// Filename:        $HeadURL:  $
//
// Version:         $Revision:  $,
//                  $Date:  $
//                  $Author: $
//
// Copyright (c) Gurux Ltd
//
//---------------------------------------------------------------------------

#ifndef CHIPPERING_H
#define CHIPPERING_H

#include "bytebuffer.h"
#include "enums.h"
#include "AES_Wrapper.h"

typedef enum
{
  GCM_DECRYPT,
  GCM_ENCRYPT
}GCM_DECRYPT_ENCRYPT;

typedef struct
{
    DLMS_SECURITY security;
    unsigned char dedicatedciphering;
    unsigned char generalciphering;
    gxByteBuffer blockCipherKey;
    gxByteBuffer dedicatedCipherKey;
    gxByteBuffer systemTitle;
    gxByteBuffer clientSystemTitle;
    unsigned long *frameCounter;
    unsigned long frameCounter_MR;
    unsigned long frameCounter_US;
    unsigned long frameCounter_PH;
    unsigned long frameCounter_FW;
    unsigned long frameCounter_IHD;
    unsigned long frameCounter_DED;
    gxByteBuffer authenticationKey;
    gxByteBuffer CipherKey;
} ciphering;

void cip_init(ciphering* target);

void cip_clear(ciphering* target);

/**
* Encrypt data.
*/
int cip_encrypt(
  ciphering* settings,
  DLMS_SECURITY security,
  DLMS_COUNT_TYPE type,
  unsigned long frameCounter,
  unsigned char tag,
  uint8_t tag_len,
  uint8_t* tag_data,
  gxByteBuffer* systemTitle,
  gxByteBuffer* plainText,
  gxByteBuffer* encrypted,
  GCM_DECRYPT_ENCRYPT decrypt_ecrypt);

/**
* Decrypt data.
*/
int cip_decrypt(
    ciphering *settings,
    gxByteBuffer *title,
    gxByteBuffer *data,
    DLMS_SECURITY *security);

/**
     * Encrypt data using AES.
     *
     * @param data
     *            Encrypted data.
     * @param offset
     *            Data offset.
     * @param secret
     *            Secret.
     */
int cip_aes1Encrypt(
  gxByteBuffer* buff,
  unsigned short offset,
  gxByteBuffer* secret);

#endif //CHIPPERING_H