#include "romflash.h"
#include <string.h>

fsl_user_callback fsl_kickWD_Callback = 0;

void kick_watchdog_fsl(void)
{
    if(fsl_kickWD_Callback != 0)
    {
        fsl_kickWD_Callback();
    }    
}
void MyErrorHandler(void)
{
        kick_watchdog_fsl();
	NOP();
}

void Init_RomFlash(void)
{   
   fsl_u08 my_fsl_status;
   fsl_descriptor_t  Flash_Init_Info;	
   
   DI();
   /* Disable bank programming */
   RPERDIS = 1U;
   FLMWRP_bit.no0= 1;
   while(FLMWRP_bit.no0==0)
   {
        kick_watchdog_fsl();
   }
   RPERDIS = 0U;
   FLMODE_bit.no1 = 0;
   while(FLMODE_bit.no1==1)
   {
        kick_watchdog_fsl();
   }
   FLMWRP_bit.no0= 0;
   
   Flash_Init_Info.fsl_flash_voltage_u08 	= 0x00; //Full speed-mode
   Flash_Init_Info.fsl_frequency_u08 		= 24;	//24MHz
   Flash_Init_Info.fsl_auto_status_check_u08	= 0x01;	//internal mode	
   my_fsl_status= FSL_Init((const __far fsl_descriptor_t*)&Flash_Init_Info);
   FSL_Open();
   EI();
   if(my_fsl_status != FSL_OK)
   {
	MyErrorHandler();
   }
}

uint8_t CheckBlank(uint16_t u16FromBlock, uint16_t u16ToBlock)
{
    fsl_u08 my_fsl_status = FSL_OK;
    uint16_t u16CBTemp;
    
    kick_watchdog_fsl();
    
    DI();
    FSL_Open();
    FSL_PrepareFunctions();
    for(u16CBTemp = u16FromBlock; u16CBTemp <= u16ToBlock; u16CBTemp++)
    {
        
        kick_watchdog_fsl();
    	my_fsl_status = FSL_BlankCheck(u16CBTemp);			//Blank Check if Device is not blank issue Erase Command
    	if(my_fsl_status != FSL_OK)
        {
    		//Blank_Check_Error(blk);
    		MyErrorHandler();
    	}
        else
        {
                //[CcnvCA78K0R] 			NOP();
                __nop();// ": BLANK" );
                break;
    	}
    }
     
    FSL_Close();
    EI();
    
    kick_watchdog_fsl();
    //MCU_Delay(50000);
    if (my_fsl_status != FSL_OK)
    {
            return 0;
    }
    return 1;
}


void erase_romFlash(uint16_t u16FromBlock, uint16_t u16ToBlock)
{
    // erase all user flash blocks
    // the first flash block to be erased is 'FIRST_USER_FLASH_BLK' and the last
    // flash block to be erased is 'NO_OF_FLASH_BLOCKS' - 1
    // these values are defined in 'command.h' and 'flash_header.h'	
    
    
    //	unsigned char Status;
    //unsigned char rxstatus;
    fsl_fsw_t  fsl_window;
    fsl_u08 my_fsl_status;
    uint16_t u16EFTemp;
    
    
    kick_watchdog_fsl();
    
    DI();
    FLMWEN = 1U;
    BANKPGEN = 1U;
    FLMWEN = 0U;
    
    FSL_Open();    	
    FSL_PrepareFunctions();
    FSL_PrepareExtFunctions();
    
    fsl_window.fsl_start_block_u16 = 0x100;
    fsl_window.fsl_end_block_u16 = 0x1FF;
    FSL_SetFlashShieldWindow(&fsl_window);
    for(u16EFTemp = u16FromBlock; u16EFTemp <= u16ToBlock; u16EFTemp++ )
    {
        
        kick_watchdog_fsl();
    	my_fsl_status = FSL_Erase(u16EFTemp);
    	while(my_fsl_status == FSL_BUSY)
    	{
                
                kick_watchdog_fsl();
                my_fsl_status = FSL_StatusCheck();
    	}
    	 
    	if(my_fsl_status != FSL_OK)
        {
    		MyErrorHandler();
    		break;
    	}
    }
    
    FLMWEN = 1U;
    BANKPGEN = 0U;
    FLMWEN = 0U;
    
    
    kick_watchdog_fsl();
    FSL_Close();
    EI();
    //MCU_Delay(50000);
}

uint8_t write_romFlash(uint32_t u32FlashAddress, uint8_t* u8DataBuf, uint16_t u16Length)
{
    fsl_u08 my_fsl_status = FSL_OK;
    fsl_write_t my_fsl_write_str;
    uint32_t tick_count;
    
    
    kick_watchdog_fsl();
    DI();
    FSL_Open();
    FSL_PrepareFunctions();
    
    tick_count = u16Length % 4;
    if(0 != tick_count)
    {
	    while(0 != tick_count)
	    {
		    tick_count--;
		    u8DataBuf[u16Length++] = 0xFF;
	    }
    }

    my_fsl_write_str.fsl_data_buffer_p_u08 = (fsl_u08 *)u8DataBuf;
    my_fsl_write_str.fsl_word_count_u08 = (uint8_t)((u16Length/4) % 0xFF);
    my_fsl_write_str.fsl_destination_address_u32 = u32FlashAddress;
    if(((u32FlashAddress < FIRST_USER_FLASH_ADDR) && (u32FlashAddress != FIRMWARE_ACTIVATION_FLAG_ADDR)) || (u32FlashAddress > FAR_FIRMWARE_ACTIVATION_FLAG_ADDR))
    {
           MyErrorHandler();
           my_fsl_status = FSL_ERR_WRITE;
    }
    else
    {
            my_fsl_status = FSL_Write((__near fsl_write_t*)&my_fsl_write_str);
            for(tick_count=0;tick_count<100000;tick_count++);
            for(tick_count=0;tick_count<100000;tick_count++);
            for(tick_count=0;tick_count<100000;tick_count++);
    }
    FSL_Close();
    EI();
    //MCU_Delay(50000);
    if (my_fsl_status != FSL_OK)
    {
            return 0;
    }
    return 1;
}

uint8_t write_dataPktToFlash(uint32_t u32FlashAddress, uint8_t* u8DataBuf, uint16_t u16Length)
{
        uint16_t u16BlockNo;
        uint8_t u8TempReadBuf[4], ret = 0;
        if ((u32FlashAddress <= 0x400C0) && ((u32FlashAddress + u16Length) >= 0x400C2))
        {
            read_dataPktToFlash(0x000C0, u8TempReadBuf, 3);
            u16BlockNo = 0x400C0 - u32FlashAddress;
            if (memcmp(&u8DataBuf[u16BlockNo], u8TempReadBuf, 3))
            {
                return 0;
            }
        }
        if ((u32FlashAddress % 0x400) == 0)
        {
                u16BlockNo = u32FlashAddress / 0x400;
                if ((u16BlockNo < START_BLOCK_NUMBER) || (u16BlockNo > END_BLOCK_NUMBER))
                {
                        return 0;
                }
                //Erase
                erase_romFlash(u16BlockNo, u16BlockNo);
                //Blank Check
                if (0 == CheckBlank(u16BlockNo, u16BlockNo))
                {
                        return 0;
                }
                if(u16BlockNo == 256)
                {
                    read_dataPktToFlash(0x000C0, u8TempReadBuf, 4);
                    u8TempReadBuf[3] = 0x84;
                    write_romFlash(0x400C0, u8TempReadBuf, 4);
                }
        }
        if (u32FlashAddress == 0x40080)
        {
            ret = write_romFlash(u32FlashAddress, u8DataBuf, 64);
            ret &= write_romFlash(u32FlashAddress + 68, &u8DataBuf[68], 60);
            return ret;
        }
        else
        {
            return (write_romFlash(u32FlashAddress, u8DataBuf, u16Length));
        }
}

void write_image_flag_activation_rom_initiated(void)
{
  uint32_t activation_init = image_init;
  write_dataPktToFlash(FAR_FIRMWARE_ACTIVATION_FLAG_ADDR, (uint8_t*)&activation_init, 4);
}

void write_image_flag_activation_rom_pass(void)
{
  uint32_t activation_init = image_pass;
  write_dataPktToFlash(FIRMWARE_ACTIVATION_FLAG_ADDR, (uint8_t*)&activation_init, 4);
}

void write_image_flag_activation_rom_fail(void)
{
  uint32_t activation_init = image_fail;
  write_dataPktToFlash(FIRMWARE_ACTIVATION_FLAG_ADDR, (uint8_t*)&activation_init, 4);
}

uint8_t read_image_flag_rom_flag(void)
{
  uint8_t activation_init = 0;
  {
    read_dataPktToFlash(FIRMWARE_ACTIVATION_FLAG_ADDR, &activation_init, 1);
  }
  return activation_init;
}

void read_dataPktToFlash(uint32_t u32FlashAddress, uint8_t* u8DataBuf, uint16_t u16Length)
{
        uint16_t temp;
        uint8_t __far *pu8FarPtr = (uint8_t __far *)u32FlashAddress;
        
        for(temp = 0; temp < u16Length; temp++)
        {
                u8DataBuf[temp] = *pu8FarPtr++;
        }
}

void swapBootAreaBank(void)
{
        DI();
        FSL_Open();    	
        FSL_PrepareFunctions();	
        FSL_PrepareExtFunctions();
        FSL_SwapActiveBootCluster();
        EI();
}