
#ifndef ROM_FLASH_H
#define ROM_FLASH_H

#include "fsl.h"
#include "fsl_types.h"
#include "typedef.h"

#include <iodefine.h>
#include "r_cg_macrodriver.h"   /* MD Macro Driver */


typedef void(*fsl_user_callback)(void);

extern fsl_user_callback fsl_kickWD_Callback;
#define FIRMWARE_ACTIVATION_FLAG_ADDR           0x3FD00
#define FAR_FIRMWARE_ACTIVATION_FLAG_ADDR       0x7FD00
#define FIRST_USER_FLASH_ADDR                   0x40000
#define LAST_USER_FLASH_ADDR                    0x7FC00
#define START_BLOCK_NUMBER                      0x100
#define END_BLOCK_NUMBER                        0x1FF
#define IMAGE_CONFIG_DATA                       0x3FB00
#define FIRMWARE_CONFIG_DATA                    0x7FB00

typedef enum
{
  image_init = 0xAA,
  image_pass = 0x88,
  image_fail = 0x00
}eImage_status;

void Init_RomFlash(void);
uint8_t CheckBlank(uint16_t u16FromBlock, uint16_t u16ToBlock);
void erase_romFlash(uint16_t u16FromBlock, uint16_t u16ToBlock);
uint8_t write_romFlash (uint32_t u32FlashAddress, uint8_t* u8DataBuf, uint16_t u16Length);
uint8_t write_dataPktToFlash(uint32_t u32FlashAddress, uint8_t* u8DataBuf, uint16_t u16Length);
void read_dataPktToFlash(uint32_t u32FlashAddress, uint8_t* u8DataBuf, uint16_t u16Length);
void write_image_flag_activation_rom_initiated(void);
void write_image_flag_activation_rom_pass(void);
void write_image_flag_activation_rom_fail(void);
uint8_t read_image_flag_rom_flag(void);
void swapBootAreaBank(void);

#endif
