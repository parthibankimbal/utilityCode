#include "AES_Wrapper.h"

int16_t aes_key_wrap(unsigned char *kek, unsigned char *key, unsigned char *cipheredKey, unsigned char len)
{
    return R_Aes_128_Wrap(kek, len/8, key, cipheredKey);
}
int16_t aes_key_unwrap(unsigned char *kek, unsigned char *cipheredKey, unsigned char *deCipheredKey, unsigned char len)
{
    return R_Aes_128_Unwrap(kek, len/8, cipheredKey, deCipheredKey);
}
void Aes_ecb_enc(uint8_t *plainText, uint8_t *cipheredText, uint8_t *aesKey, uint16_t len)
{
    R_Aes_Init();
    R_Aes_128_Ecbenc(plainText, cipheredText, aesKey, len/16);
    R_Aes_Close();
}
void Aes_ecb_dec(uint8_t *plainText, uint8_t *cipheredText, uint8_t *aesKey, uint16_t len)
{
    R_Aes_Init();
    R_Aes_128_Ecbdec(plainText, cipheredText, aesKey, 1);
    R_Aes_Close();
}

int16_t Aes_gcm_enc(uint8_t *plainText,
                    uint8_t *cipheredText,
                    uint16_t data_len,
                    uint8_t *EncKey,
                    uint8_t *AuthTag,
                    uint8_t AuthTagSize,
                    uint8_t *iv,
                    uint8_t ivSize,
                    uint8_t *AuthKey,
                    uint8_t AuthKeySize)
{
    int16_t pass = 0;
    pass = (int16_t)R_gcm_Init(GCM_AESKEY_LEN_128);
    pass |= (int16_t)R_gcm_enc(plainText, cipheredText, data_len, EncKey, AuthTag, AuthTagSize, iv, ivSize, AuthKey, AuthKeySize);
    R_gcm_Close();
    return pass;
}

int16_t Aes_gcm_dec(uint8_t *cipheredText,
                    uint8_t *plainText,
                    uint16_t data_len,
                    uint8_t *EncKey,
                    uint8_t *AuthTag,
                    uint8_t AuthTagSize,
                    uint8_t *iv,
                    uint8_t ivSize,
                    uint8_t *AuthKey,
                    uint8_t AuthKeySize)
{
    int16_t pass = 0;
    pass = (int16_t)R_gcm_Init(GCM_AESKEY_LEN_128);
    pass |= (int16_t)R_gcm_dec(cipheredText, plainText, data_len, EncKey, AuthTag, AuthTagSize, iv, ivSize, AuthKey, AuthKeySize);
    R_gcm_Close();
    return pass;
}