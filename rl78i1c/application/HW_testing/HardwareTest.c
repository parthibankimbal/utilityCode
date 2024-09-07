
#include "typedef.h"                /* GSCE Standard Typedef */
#include "r_cg_macrodriver.h"       /* Macro Driver Definitions */
#include "r_cg_wdt.h"               /* WDT Driver */
#include "RF_Comm.h"
#include "nic_comm.h"
/* Standard library */
#include <string.h>
#include <stdlib.h>
#include "r_cg_userdefine.h"
#include "eeprom.h"             /* EEPROM MW Layer */
//#include "bl_serialflash.h"             /* SerialFlash MW Layer */
#include "wrp_user_hardware.h"       /* Wrapper IIC header */
#include "relay.h"
#include "HardwareTest.h"
#include "communication.h"
#include "SingleWireOp.h"

uint32_t dTestAdd;
uint16_t wTestIndex, wTestIndex1;
unsigned char byTestFlag, byTestFlag2, byTestFlag1 = 0, byTestFlag3 = 0;
const unsigned char byTestBuf1[128] = "(ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890~!@#$%^&*()_+=ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890~!@#$%^&*()_+=ABCDEFGHIJKLMNOPQRSTUVWXYZ)";
//unsigned char byTestBuf[128];// = "(ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890~!@#$%^&*()_+=ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890~!@#$%^&*()_+=ABCDEFGHIJKLMNOPQRSTUVWXYZ)";

extern void RTC_COMP_calib(float value, uint8_t add);
extern float get_RTC_COMP_value(void);

st_Accumulate_Energies stAccEnergies = {0};

static uint8_t convt_byte_to_bcd(uint8_t byte_data)
{
    return ((byte_data/10)<<4)+(byte_data % 10);
}

static uint8_t convt_bcd_to_byte(uint8_t bcd_data)
{
    return((bcd_data&0xF0)>>4)*10 + (bcd_data&0x0F);
}

uint8_t TestMem(uint8_t byMemType)
{
    unsigned char byTestBuf[128];
    if ((IS_TEST_JUMPER_INACTIVE()))
    {
        return FALSE;
    }
    WRP_IIC_DRIVER_Init();

    if (0 == byMemType)
    {
        //byTestFlag1 = 0;
        //dTestAdd = 0;
        //memcpy(byTestBuf,byTestBuf1,128);
        //for (wTestIndex1 = 0; wTestIndex1 < 1024; wTestIndex1++)
        //{
        //        EPR_Write(dTestAdd,byTestBuf,128);
        //        dTestAdd += 128;
        //        R_WDT_Restart();
        //}                
        //dTestAdd = 0;
        //for (wTestIndex1 = 0; wTestIndex1 < 1024; wTestIndex1++)
        //{
        //        for (byTestFlag2 = 0; byTestFlag2 < 128; byTestFlag2++)
        //        {
        //                byTestBuf[byTestFlag2] = 0;
        //        }
        //        EPR_Read(dTestAdd,byTestBuf,128);
        //        //if(memcmp(byTestBuf,"(ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890~!@#$%^&*()_+=ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890~!@#$%^&*()_+=ABCDEFGHIJKLMNOPQRSTUVWXYZ)",128) != 0)
        //        if(memcmp(byTestBuf,byTestBuf1,128) != 0)
        //        {
        //                byTestFlag1 = 1;
        //                break;
        //        }
        //        dTestAdd += 128;
        //        R_WDT_Restart();
        //}
        ///////////////////////////////////////////////////////////////////////////////////////////////
        byTestFlag1 = 0;
        R_WDT_Restart();

        dTestAdd = 0;
        memcpy(byTestBuf, byTestBuf1, 128);
        EPR_Write(dTestAdd, byTestBuf, 128);
        memset(byTestBuf, 0, 128);
        EPR_Read(dTestAdd, byTestBuf, 128);
        if (memcmp(byTestBuf, byTestBuf1, 128) != 0)
        {
            byTestFlag1 = 1;
        }
        
        dTestAdd = (uint32_t)(EPR_DEVICE_SIZE - EPR_DEVICE_PAGESIZE);
        memcpy(byTestBuf, byTestBuf1, 128);
        EPR_Write(dTestAdd, byTestBuf, 128);
        memset(byTestBuf, 0, 128);
        EPR_Read(dTestAdd, byTestBuf, 128);
        if (memcmp(byTestBuf, byTestBuf1, 128) != 0)
        {
            byTestFlag1 = 1;
        }
        memset(byTestBuf, 0 , 128);
        dTestAdd = 0;
        EPR_Write(dTestAdd, byTestBuf, 128);
        dTestAdd = (uint32_t)(EPR_DEVICE_SIZE - EPR_DEVICE_PAGESIZE);
        EPR_Write(dTestAdd, byTestBuf, 128);
        ///////////////////////////////////////////////////////////////////////////////////////////////
        if (0 != byTestFlag1)
        {
            WRP_IIC_DRIVER_DeInit();
            return FALSE;
        }
        else
        {
            return TRUE;
        }
    }
    else if (1 == byMemType)
    {
        //byTestFlag1 = 0;
        //dTestAdd = 0;
        //for (byTestFlag2 = 0; byTestFlag2 < 128; byTestFlag2++)
        //{
        //        if(SFL_OK != BL_SFL_DeviceErase4KB(dTestAdd))
        //        {
        //                byTestFlag1 = 1;
        //                break;
        //        }
        //        dTestAdd += 4096;
        //        R_WDT_Restart();
        //}
        //        
        //dTestAdd = 0;
        ////memcpy(byTestBuf,"(ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890~!@#$%^&*()_+=ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890~!@#$%^&*()_+=ABCDEFGHIJKLMNOPQRSTUVWXYZ)",128);
        //memcpy(byTestBuf,byTestBuf1,128);
        //for (wTestIndex1 = 0; wTestIndex1 < 4096; wTestIndex1++)
        //{
        //        BL_SFL_DeviceWrite(dTestAdd,byTestBuf,128);
        //        dTestAdd += 128;
        //        R_WDT_Restart();
        //}
        //dTestAdd = 0;          
        //for (wTestIndex1 = 0; wTestIndex1 < 4096; wTestIndex1++)
        //{
        //        for (byTestFlag2 = 0; byTestFlag2 < 128; byTestFlag2++)
        //        {
        //                byTestBuf[byTestFlag2] = 0;
        //        }
        //        BL_SFL_DeviceRead(dTestAdd,byTestBuf,128);
        //        //if(memcmp(byTestBuf,"(ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890~!@#$%^&*()_+=ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890~!@#$%^&*()_+=ABCDEFGHIJKLMNOPQRSTUVWXYZ)",128) != 0)
        //        if(memcmp(byTestBuf,byTestBuf1,128) != 0)
        //        {
        //                byTestFlag1 = 1;
        //                break;
        //        }
        //        dTestAdd += 128;
        //        R_WDT_Restart();
        //}
        ///////////////////////////////////////////////////////////////////////////////////////
    /*    byTestFlag1 = 0;
        R_WDT_Restart();

        dTestAdd = 0;
        if (SFL_OK != BL_SFL_DeviceErase4KB(dTestAdd))
        {
            byTestFlag1 = 1;
        }
        dTestAdd = (uint32_t)4096 * 127;
        if (SFL_OK != BL_SFL_DeviceErase4KB(dTestAdd))
        {
            byTestFlag1 = 1;
        }

        dTestAdd = 0;
        memcpy(byTestBuf, byTestBuf1, 128);
        BL_SFL_DeviceWrite(dTestAdd, byTestBuf, 128);
        memset(byTestBuf, 0, 128);
        BL_SFL_DeviceRead(dTestAdd, byTestBuf, 128);
        if (memcmp(byTestBuf, byTestBuf1, 128) != 0)
        {
            byTestFlag1 = 1;
        }

        dTestAdd = (uint32_t)4096 * 127;
        BL_SFL_DeviceWrite(dTestAdd, byTestBuf, 128);
        memset(byTestBuf, 0, 128);
        BL_SFL_DeviceRead(dTestAdd, byTestBuf, 128);
        if (memcmp(byTestBuf, byTestBuf1, 128) != 0)
        {
            byTestFlag1 = 1;
        }
        ///////////////////////////////////////////////////////////////////////////////////////

        if (0 != byTestFlag1)
        {
            WRP_IIC_DRIVER_DeInit();
            return FALSE;
        }
        else
        {
            return TRUE;
        }
        */
        return TRUE;
    }
    
    return FALSE;
}

void relayTest(uint8_t byConnectState)
{
    if (0 == byConnectState)
    {
        RELAY_SwitchOn_R();
        //for (wTestIndex1 = 0; wTestIndex1 < 10; wTestIndex1++)
        //{
        //    Switch_Delay();
        //}
        //RELAY_SwitchOn_Y();
        //for (wTestIndex1 = 0; wTestIndex1 < 10; wTestIndex1++)
        //{
        //    Switch_Delay();
        //}
        //RELAY_SwitchOn_B();
    }
    else if (1 == byConnectState)
    {
        RELAY_SwitchOff_R();
        //for (wTestIndex1 = 0; wTestIndex1 < 10; wTestIndex1++)
        //{
        //    Switch_Delay();
        //}
        //RELAY_SwitchOff_Y();
        //for (wTestIndex1 = 0; wTestIndex1 < 10; wTestIndex1++)
        //{
        //    Switch_Delay();
        //}
        //RELAY_SwitchOff_B();
    }
}

uint8_t rfPinsCheck(uint8_t byInput)
{
    if (0 == byInput)
    {
        if ((0 != RF_NET_STAT()) || (0 != RF_CTS_STAT()) /*|| (0 != IS_RF_LINK_MISSING())*/)
        {
            return FALSE;
        }
        //RF_MTR_BZY_DEASSERT();
        RF_RST_ASSERT();
        RF_RTS_ASSERT();
        RF_EVENT_NOT_DEASSERT();
        //RF_POWER_NOT_DEASSERT();
        //RF_Z_CROSS_DEASSERT();
        stRF_params.RF_Comm_skip = RF_COMM_MULTIPLIER_100MS * 10;
        RF_COMM_DATA_SENT_TIMEOUT_FLAG = 0;
        return TRUE;
    }
    else if (1 == byInput)
    {
        if ((0 == RF_NET_STAT()) || (0 == RF_CTS_STAT()) /*|| (0 == IS_RF_LINK_MISSING())*/)
        {
            return FALSE;
        }
        //RF_MTR_BZY_ASSERT();
        RF_RST_DEASSERT();
        RF_RTS_DEASSERT();
        RF_EVENT_NOT_ASSERT();
        //RF_POWER_NOT_ASSERT();
        //RF_Z_CROSS_ASSERT();
        stRF_params.RF_Comm_skip = RF_COMM_MULTIPLIER_100MS * 10;
        RF_COMM_DATA_SENT_TIMEOUT_FLAG = 0;
        return TRUE;
    }
    return FALSE;
}

uint8_t send_receive_RF_data(en_RF_stat cmd)
{
    uint8_t data_buffer[50] ={0};
    uint8_t data_len = 0;
    uint32_t u32TimeOut;
    en_RF_stat cmd_type_backup = stRF_params.RF_stat;
	
    stRF_params.RF_stat = cmd;
    RF_Params_get_set_func(data_buffer, &data_len, stRF_params.RF_stat, Write_data);
    RF_comm_frame_gen(data_buffer, &data_len);
    Send_RF_data(data_buffer, data_len, TRUE);
    u32TimeOut = 0;
    do
    {
      if ((rx_msg[RF_COMM_PORT].inter_Activity_timeout >= DEFAULT_TICK_PROCESS_FRAME) && (rx_msg[RF_COMM_PORT].len != 0) && (rx_msg[RF_COMM_PORT].Process_Frame == 0))
      {
          rx_msg[RF_COMM_PORT].Process_Frame = 1;
      }
      if (rx_msg[RF_COMM_PORT].Process_Frame == 1)
      {
        RF_comm_frame_parser(rx_msg[RF_COMM_PORT].buf.uint8, rx_msg[RF_COMM_PORT].len);
        rx_msg[RF_COMM_PORT].len = 0;
        rx_msg[RF_COMM_PORT].Process_Frame = 0;
        break;
      }
      R_WDT_Restart();
      MCU_Delay(500);
    } while (u32TimeOut++ < 1500);
    
    stRF_params.RF_stat = cmd_type_backup;
    if (u32TimeOut++ >= 1500)
    {
      return 1;
    }
    return 0;
}

uint8_t set_get_Rf_key(uint8_t key_index)
{
    stRF_params.RF_Key_String[0] = key_index;
    return send_receive_RF_data(Cmd_Test_RF_GET_KEY);
}

uint8_t rfPinsCheck_OnBoard(uint8_t byInput)
{
    uint8_t u8TestByte = FALSE;
    
    if (0 == byInput)
    {
        //RF_MTR_BZY_DEASSERT();
        //RF_RST_ASSERT();
        RF_RTS_ASSERT();
        RF_EVENT_NOT_DEASSERT();
        //RF_POWER_NOT_DEASSERT();
        //RF_Z_CROSS_DEASSERT();
        
        u8TestByte = send_receive_RF_data(Cmd_Test_RF_IO_LOW);
        if ((0 != RF_NET_STAT()) || (0 != RF_CTS_STAT()) || (0 != IS_RF_LINK_MISSING()) || (u8TestByte != 0) || (stRF_params.RF_IO_Cmd_Res != 0)) /*|| (0 != RF_MOD_BZY_STAT())*/
        {
           u8TestByte = FALSE;
        }
        else
        {
           u8TestByte = TRUE;
        }
    }
    else if (1 == byInput)
    {
        //RF_MTR_BZY_ASSERT();
        //RF_RST_DEASSERT();
        RF_RTS_DEASSERT();
        RF_EVENT_NOT_ASSERT();
        //RF_POWER_NOT_ASSERT();
        //RF_Z_CROSS_ASSERT();

        u8TestByte = send_receive_RF_data(Cmd_Test_RF_IO_HIGH);
	
        if ((0 == RF_NET_STAT()) || (0 == RF_CTS_STAT()) || (u8TestByte != 0) || (stRF_params.RF_IO_Cmd_Res != 0)) /*|| (0 == RF_MOD_BZY_STAT())*/
        {
           u8TestByte = FALSE;
        }
        else
        {
            u8TestByte = TRUE;
        }
    }
    return u8TestByte;
}

void display_check_routine(void)
{
    uint8_t u8DCRIndex = 0;
    LCD_RamSetAll();
    for (u8DCRIndex = 0; u8DCRIndex < 20; u8DCRIndex++)
    {
        Switch_Delay();
    }
    LCD_RamClearAll();
    for (u8DCRIndex = 5; u8DCRIndex < 21; u8DCRIndex += 2)
    {
        LCD_WriteRAMDigitInfo_8_Comm(LCD_RAM_START_ADDRESS + u8DCRIndex, 0xFF);
    }
    for (u8DCRIndex = 0; u8DCRIndex < 20; u8DCRIndex++)
    {
        Switch_Delay();
    }
    LCD_RamClearAll();
    for (u8DCRIndex = 6; u8DCRIndex < 21; u8DCRIndex += 2)
    {
        LCD_WriteRAMDigitInfo_8_Comm(LCD_RAM_START_ADDRESS + u8DCRIndex, 0xFF);
    }
    for (u8DCRIndex = 0; u8DCRIndex < 20; u8DCRIndex++)
    {
        Switch_Delay();
    }
}

void accumulateEnergy_Callback(void)
{
    stAccEnergies.fPhaseEnergy += fabs(EM_GetActivePower(EM_LINE_PHASE)) / 28800;    // accumulated in 125ms Timer, so energy will be power/(8*3600)
    stAccEnergies.fNeutralEnergy += fabs(EM_GetActivePower(EM_LINE_NEUTRAL)) / 28800;
    stAccEnergies.fPhaseEnergy_kVAh += fabs(EM_GetApparentPower(EM_LINE_PHASE)) / 28800;
    stAccEnergies.fNeutralEnergy_kVAh += fabs(EM_GetApparentPower(EM_LINE_NEUTRAL)) / 28800;
    stAccEnergies.fPhaseEnergy_kVArh += fabs(EM_GetReactivePower(EM_LINE_PHASE)) / 28800;
    stAccEnergies.fNeutralEnergy_kVArh += fabs(EM_GetReactivePower(EM_LINE_NEUTRAL)) / 28800;
    stAccEnergies.u32AccTime_ms += 125;
    if (stAccEnergies.u32AccTime_ms >= stAccEnergies.u32AccTime_set)
    {
        stAccEnergies.u8AccEnergyStart = 0;
        R_IT8Bit0_Channel0_Stop();
    }
}

uint8_t getInstantParam(uint32_t dAddress, uint8_t *buff, uint16_t byLength)
{
    #define _BV(x)  (1<<x)
    E_Set_Get_Instant_Param param_id = (E_Set_Get_Instant_Param)dAddress;
    stTime_struct *dateTime;
    uint8_t byTemp;
    uint8_t data[sizeof(stTime_struct)];
    uint8_t ret = 0;
    uint32_t u32Temp = 0;
    float fGIPTemp;
    double dGIPTemp;
    kick_watchdog();
    switch (param_id)
    {
    case Meter_Number: 
            /* Set meter number */
            get_meter_serial_number(buff);
            break;
    case RMS_Volt_Current:
            memcpy(buff,(uint8_t*)&g_inst_read_params.vrms, sizeof(float));
            buff += sizeof(float);
            memcpy(buff,(uint8_t*)&g_inst_read_params.irms_phase, sizeof(float));
            buff += sizeof(float);
            memcpy(buff,(uint8_t*)&g_inst_read_params.irms_neutral, sizeof(float));
            buff += sizeof(float);
            break; 
    case Active_kwh:
            dGIPTemp = ((double)myEnergies.total.energy[Active_Imp] + (double)myEnergies.remainder.energy[Active_Imp]);
            memcpy(buff, (uint8_t*)&dGIPTemp, sizeof(double));
            buff += sizeof(double);
            break;
    case RTC_Clock: 
            /* Get RTC data */
            dateTime = (stTime_struct*)&data;
            get_RTC_dateTime(dateTime);
            *buff++ = convt_byte_to_bcd(dateTime->second);
            *buff++ = convt_byte_to_bcd(dateTime->minute);
            *buff++ = convt_byte_to_bcd(dateTime->hour);
            *buff++ = convt_byte_to_bcd(dateTime->day);
            *buff++ = convt_byte_to_bcd(dateTime->weekday);
            *buff++ = convt_byte_to_bcd(dateTime->month);
            *buff++ = convt_byte_to_bcd(dateTime->year/100);
            *buff++ = convt_byte_to_bcd(dateTime->year%100);
            if (is_cover_open())
            {
              *buff++ = 1;
            }         
            else
            {
              *buff++ = 0;
            }
          break;

    case RTC_Comp_10_Plus:
    case RTC_Comp_10_Less:
        if (RTC_Comp_10_Plus == param_id)
        {
            RTC_COMP_calib(10.0, 1);
        }
        else
        {
            RTC_COMP_calib(-10.0, 1);
        }
        fGIPTemp = get_RTC_COMP_value();
        memcpy(buff, (uint8_t*)&fGIPTemp, sizeof(float));
        buff += sizeof(float);
        break;

    case Meter_Version_No: 
        /* Get Firmware Version */
        memcpy(buff,(uint8_t*)&METER_VERSION_NUMBER, sizeof(METER_VERSION_NUMBER));
        buff += sizeof(METER_VERSION_NUMBER);
        break;

    case Int_Firmware_Version_No:
        /* Get Firmware Version */
        memcpy(buff, (uint8_t*)&INTERNAL_VERSION, sizeof(INTERNAL_VERSION));
        buff += sizeof(INTERNAL_VERSION);
        break;
            
    case RF_Key_1:
    case RF_Key_2:
    case RF_Key_3:
    case RF_Key_4:
    case RF_Key_5:
    case RF_Key_6:
            if (0 != set_get_Rf_key(dAddress - (uint8_t)RF_Key_1 + 1))
            {
              ret = 1;
            }
            memcpy(buff,(uint8_t*)stRF_params.RF_Key_String, 16);
          break;

//    case 11: byPtrTemp = (uint8_t*)&g_inst_read_params.active_power[0];
//            for (byTemp = 0; byTemp < 20; byTemp++)
//            {
//                    *buff++ = *byPtrTemp++;
//            }
//          break;

//    case 12: byPtrTemp = (uint8_t*)&g_inst_read_params.apparent_power[0];
//            for (byTemp = 0; byTemp < 20; byTemp++)
//            {
//                    *buff++ = *byPtrTemp++;
//            }
//          break;

    case Board_Number: 
            /* Set meter number */
            EPR_Read(BOARD_SERIAL_NUMBER_LOC, buff, BOARD_SERIAL_NUMBER_SIZE);
            break;

    case Network_Config_RF:
        /* Set meter number */
        get_meter_serial_number(buff);
        buff += 12;
        memcpy(buff, (uint8_t*)&g_st_NIC_info.nvm_info.network_address, sizeof(uint32_t));
        buff += sizeof(uint32_t);
        memcpy(buff, (uint8_t*)&g_st_NIC_info.nvm_info.network_channel, sizeof(uint8_t));
        buff += sizeof(uint8_t);
        memcpy(buff, (uint8_t*)&g_st_NIC_info.nvm_info.encryption_key[1], 16);
        buff += 16;
        break;

    case Single_Wire_Offset:
        /* Set Single Wire Offset Value */
        *buff = 0;
        if (0xA5 != read_eeprom(STORAGE_EEPROM_NO_LOAD_OFFSET_FLAG_ADDR))
        {
            *buff = 1;
        }
        buff++;
        *buff++ = verify_SingleWire_Offset();
        break;

    case HW_Switch_Release_Chk: 
            *buff = 0;
            if (COVER_OPEN_PIN)
            {
                *buff |= _BV(0);
            }
            if (!PUSH_BUTTON_KEY)
            {
                *buff |= _BV(1);
            }
            if (!MAGNET_IN_STAT_1)
            {
                *buff |= _BV(2);
            }
            if (!MAGNET_IN_STAT_2)
            {
                *buff |= _BV(3);
            }
          break;

    case HW_Switch_Connect_Chk:
            *buff = 0;
            if (!COVER_OPEN_PIN)
            {
                *buff |= _BV(0);
            }
            if (PUSH_BUTTON_KEY)
            {
                *buff |= _BV(1);
            }
            if (MAGNET_IN_STAT_1)
            {
                *buff |= _BV(2);
            }
            if (MAGNET_IN_STAT_2)
            {
                *buff |= _BV(3);
            }
          break;

    case HW_Func_Test_Release:
            *buff = 0;
            if (!COVER_OPEN_PIN)
            {
                *buff |= _BV(0);
            }
            if (!PUSH_BUTTON_KEY)
            {
                *buff |= _BV(1);
                //byTemp = 1;
            }
            if (!MAGNET_IN_STAT_1)
            {
                *buff |= _BV(2);
            }
            if (!MAGNET_IN_STAT_2)
            {
                *buff |= _BV(3);
            }
          break;

    case HW_Func_Test: 
            *buff = 0;
            if (!COVER_OPEN_PIN)
            {
                *buff |= _BV(0);
            }
            if (PUSH_BUTTON_KEY)
            {
                *buff |= _BV(1);
                //byTemp = 1;
            }
            if (MAGNET_IN_STAT_1)
            {
                *buff |= _BV(2);
            }
            if (MAGNET_IN_STAT_2)
            {
                *buff |= _BV(3);
            }
            RELAY_SwitchOff_R();
            //RELAY_SwitchOff_B();
            for (wTestIndex1 = 0; wTestIndex1 < 20; wTestIndex1++)
            {
                Switch_Delay();
            }
            if (EM_GetCurrentRMS(EM_LINE_PHASE) > 0.05)
            {
                *buff |= _BV(4);
            }
            if (EM_GetCurrentRMS(EM_LINE_NEUTRAL) > 0.05)
            {
                *buff |= _BV(5);
            }
            RELAY_SwitchOn_R();
            //RELAY_SwitchOn_B();
            for (wTestIndex1 = 0; wTestIndex1 < 20; wTestIndex1++)
            {
                Switch_Delay();
            }
            if (EM_GetCurrentRMS(EM_LINE_PHASE) < 1.0)
            {
                *buff |= _BV(6);
            }
            if (EM_GetCurrentRMS(EM_LINE_NEUTRAL) < 1.0)
            {
                *buff |= _BV(7);
            }
          break;
/*
    case HW_Energy_Acc_Start:
            stAccEnergies.u8AccEnergyStart = 1;
            stAccEnergies.fPhaseEnergy = 0;
            stAccEnergies.fNeutralEnergy = 0;
            stAccEnergies.u32AccTime_ms = 0;

            dGIPTemp = ((double)myEnergies.total.energy[Active_Imp] + (double)myEnergies.remainder.energy[Active_Imp]);
            memcpy(buff, (uint8_t*)&dGIPTemp, sizeof(double));
            buff += sizeof(double);
            R_IT8Bit0_Channel0_Stop();
            R_IT8Bit0_Channel0_Set_Interval(enTime_125ms);
            R_IT8Bit0_Channel0_Start();
          break;
*/
    case HW_Energy_Acc_Read: 
            stAccEnergies.u8AccEnergyStart = 0;
            R_IT8Bit0_Channel0_Stop();
            stAccEnergies.dEnergyDelta = ((double)myEnergies.total.energy[Active_Imp] + (double)myEnergies.remainder.energy[Active_Imp]) - stAccEnergies.dEnergyDelta;
            memcpy(buff,(uint8_t*)&stAccEnergies.fPhaseEnergy, sizeof(stAccEnergies) - 5);     //kWh_Ph, kWh_Neu, kVAh_Ph, kVAh_Neu, kVArh_Ph, kVArh_Neu, Time in ms
            buff += sizeof(stAccEnergies) - 1;
            //dGIPTemp = ((double)myEnergies.total.energy[Active_Imp] + (double)myEnergies.remainder.energy[Active_Imp]);
            //memcpy(buff, (uint8_t*)&dGIPTemp, sizeof(double));
            //buff += sizeof(double);
          break;

      default:
            ret = 1;
          break;
    }
  return ret;
}

uint8_t setInstantParam(uint32_t dAddress, uint8_t *buff, uint16_t byLength)
{
    E_Set_Get_Instant_Param param_id = (E_Set_Get_Instant_Param)dAddress;
    stTime_struct *dateTime;
    uint8_t byTemp;
    uint8_t data[sizeof(stTime_struct)];
    uint8_t ret = 0;
    float f_value;
    
    switch (param_id)
    {
      case RTC_Clock: 
        dateTime = (stTime_struct*)&data;
        dateTime->second = convt_bcd_to_byte(buff[0]);
        dateTime->minute = convt_bcd_to_byte(buff[1]);
        dateTime->hour = convt_bcd_to_byte(buff[2]);
        dateTime->day = convt_bcd_to_byte(buff[3]);
        dateTime->weekday = convt_bcd_to_byte(buff[4]);
        dateTime->month = convt_bcd_to_byte(buff[5]);
        dateTime->year = convt_bcd_to_byte(buff[6])*100 + convt_bcd_to_byte(buff[7]);
        /* Set RTC data */
        set_RTC_dateTime(dateTime);
        reset_timeSyncRTC();
      break;
      case Meter_Number: 
        /* Set meter number */
        for(byTemp = 0; byTemp < byLength; byTemp++)
        {
          if(*buff == '0')
          {
            buff++;
          }
          else
          {
            break;
          }
        }
        ret = set_meter_serial_number(buff, byLength - byTemp);
      break;
    case Board_Number: 
            /* Set board number */
            EPR_Write(BOARD_SERIAL_NUMBER_LOC, buff, BOARD_SERIAL_NUMBER_SIZE);
            break;
    case HW_Energy_Acc_Start: 
            memset((uint8_t*)&stAccEnergies, 0, sizeof(st_Accumulate_Energies));        //Initializing the registers to 0
            stAccEnergies.u8AccEnergyStart = 1;                                         //Set the flag for energy accumulation
            stAccEnergies.u32AccTime_set = (uint32_t)(buff[0] * 1000);
            stAccEnergies.dEnergyDelta = ((double)myEnergies.total.energy[Active_Imp] + (double)myEnergies.remainder.energy[Active_Imp]);       //keeping snap of current energy register as starting point
            R_IT8Bit0_Channel0_Stop();
            R_IT8Bit0_Channel0_Set_Interval(enTime_125ms);
            R_IT8Bit0_Channel0_Start();
          break;
    case Clear_CoverOpen:
        clear_cover_open();
        break;
    case Single_Wire_Offset:
        /* Set Single Wire Offset Value */
        set_SingleWire_Offset();
        break;
    case RTC_calibrate:
		memcpy(&f_value, buff, sizeof(float));
		RTC_COMP_calib(f_value, (uint8_t)buff[4]);
		break;
    case RTC_Comp_Pulse_start:
        byTemp = buff[0] > 1 ? (uint8_t)RTCOS_FREQ_64HZ : (uint8_t)RTCOS_FREQ_1HZ;
        R_RTC_Set_ClockOutputOn((rtc_clock_out_t)byTemp);
	break;
    case RTC_Comp_Pulse_stop:
        R_RTC_Set_ClockOutputOff();
	break;
    case Display_Check:
        if (*buff == 1)
        {
            display_check_routine();
        }
        break;
    default:
            ret = 1;
          break;
    }

  return ret;
}
