#include "stdio.h"
#include <stdint.h>
#include <string.h>
#include "crc16.h"
////crc generator////
uint16_t gen_crc16(const uint8_t *data, uint16_t out,uint16_t size,char continues)
{
	//uint16_t out = 0;
	uint16_t crc = 0;
	uint16_t bits_read = 0, bit_flag;
	uint16_t i;
	uint16_t j = 0x0001;
	//out=iv;
	// Sanity check: 
	if(data == NULL)
		return 0;
	while(size > 0)
	{
		bit_flag = out >> 15;
		// Get next bit: 
		out <<= 1;
		out |= (*data >> bits_read) & 1; // item a) work from the least significant bits
		// Increment bit counter: 
		bits_read++;
		if(bits_read > 7)
		{
			bits_read = 0;
			data++;
			size--;
		}

		// Cycle check: 
		if(bit_flag)
			out ^= CRC16;
	}
	if(!continues)
	{
		// item b) "push out" the last 16 bits
		for (i = 0; i < 16; ++i) {
			bit_flag = out >> 15;
			out <<= 1;
			if(bit_flag)
				out ^= CRC16;
		}
		// item c) reverse the bits
		i = 0x8000;
		for (; i != 0; i =((i >> 1) & 0x7fff), j = j << 1) {
			if (i & out) crc |= j;
		}
	}
	else
	{
		crc=out;
	}
	return crc;
}