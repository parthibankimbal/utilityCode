#include <typedef.h>
#ifndef CRC_CCITT_FFFF
#define CRC_CCITT_FFFF

#define CRCD_DEFAULT 0xFFFF
#define HARWARE_CRC_CCITT_FFFF

#ifdef HARWARE_CRC_CCITT_FFFF
#include "r_cg_macrodriver.h"
#include "r_cg_crc.h"
#endif

void R_CRC_Clear(void);
void R_CRC_Set(uint16_t initial_value);
uint16_t R_CRC_Calculate(uint8_t* buffer, uint16_t length);
uint16_t R_CRC_GetResult(void);
uint16_t R_CRC_Calculate_Fresh(uint8_t* buffer, uint16_t length);
#endif
