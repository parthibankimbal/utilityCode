#include "stdio.h"
#include <stdint.h>
#include <string.h>

#ifndef CRC16_H
#define CRC16_H
#define CRC16 0x8005

typedef enum  
{
 Final,
 Continue
}MyEnum;

////crc generator////
uint16_t gen_crc16(const uint8_t *data, uint16_t out,uint16_t size,char continues);
#endif //CRC16_H