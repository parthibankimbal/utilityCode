#include "crc_ccitt_ffff.h"
#ifndef HARWARE_CRC_CCITT_FFFF
static uint16_t CRCD = 0;
#endif //HARWARE_CRC_CCITT_FFFF
//https://gist.github.com/aurelj/270bb8af82f65fa645c1

/***********************************************************************************************************************
* Function Name: R_CRC_Calculate
* Description  : This function calculate the CRC value for a specified buffer in near memory
* Arguments    : buffer -
*              :     memory buffer to calculate CRC-16-CCITT
*              : length -
*              :     memory buffer length
* Return Value : None
***********************************************************************************************************************/
#ifdef HARWARE_CRC_CCITT_FFFF
/* refer to the implementation in the driver level of the crc. */
#else
uint16_t R_CRC_Calculate(uint8_t* buffer, uint16_t length)
{
	uint8_t t;
	uint8_t L;
	if (!buffer || length <= 0)
		return CRCD;

	while (length--) {
		CRCD ^= *buffer++;
		L = CRCD ^ (CRCD << 4);
		t = (L << 3) | (L >> 5);
		L ^= (t & 0x07);
		t = (t & 0xF8) ^ (((t << 1) | (t >> 7)) & 0x0F) ^ (uint8_t)(CRCD >> 8);
		CRCD = (L << 8) | t;
	}
	return CRCD;
}

/***********************************************************************************************************************
* Function Name: R_CRC_Clear
* Description  : This function clear the CRC result register.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_CRC_Clear(void)
{
	CRCD = 0;
}

/***********************************************************************************************************************
* Function Name: R_CRC_GetResult
* Description  : This function get the CRC result register.
* Arguments    : None
* Return Value : CRC-16-CCITT result (2 bytes)
***********************************************************************************************************************/
uint16_t R_CRC_GetResult(void)
{
	return (CRCD);
}
#endif //HARWARE_CRC_CCITT_FFFF

/***********************************************************************************************************************
* Function Name: R_CRC_Set
* Description  : This function set the CRC result register.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_CRC_Set(uint16_t initial_value)
{
	CRCD = initial_value;
}
/***********************************************************************************************************************
* Function Name: R_CRC_Calculate_Fresh
* Description  : This function get the CRC result of a buffer from reset value of CRCD to default.
* Arguments    : buffer and its length
* Return Value : CRCD
***********************************************************************************************************************/
uint16_t R_CRC_Calculate_Fresh(uint8_t* buffer, uint16_t length)
{
    R_CRC_Set(CRCD_DEFAULT);
    return R_CRC_Calculate(buffer, length);
}