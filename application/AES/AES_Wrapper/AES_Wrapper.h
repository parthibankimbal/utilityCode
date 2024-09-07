#ifndef AES_WRAPPER_H
#define AES_WRAPPER_H

#include "r_aes_hwip.h"
#include "r_gcm_hwip.h"
#include "r_aes_keywrap.h"

int16_t aes_key_wrap(unsigned char *kek, unsigned char *key, unsigned char *cipheredKey, unsigned char len);
int16_t aes_key_unwrap(unsigned char *kek, unsigned char *cipheredKey, unsigned char *deCipheredKey, unsigned char len);
void Aes_ecb_enc(uint8_t *plainText, uint8_t *cipheredText, uint8_t *aesKey, uint16_t len);
void Aes_ecb_dec(uint8_t *plainText, uint8_t *cipheredText, uint8_t *aesKey, uint16_t len);

int16_t Aes_gcm_enc(uint8_t *plainText,
                    uint8_t *cipheredText,
                    uint16_t data_len,
                    uint8_t *EncKey,
                    uint8_t *AuthTag,
                    uint8_t AuthTagSize,
                    uint8_t *iv,
                    uint8_t ivSize,
                    uint8_t *AuthKey,
                    uint8_t AuthKeySize);
                    
int16_t Aes_gcm_dec(uint8_t *cipheredText,
                    uint8_t *plainText,
                    uint16_t data_len,
                    uint8_t *EncKey,
                    uint8_t *AuthTag,
                    uint8_t AuthTagSize,
                    uint8_t *iv,
                    uint8_t ivSize,
                    uint8_t *AuthKey,
                    uint8_t AuthKeySize);
#endif /* AES_WRAPPER_H */
