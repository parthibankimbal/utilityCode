#ifndef EM_ENERGIES_H
#define EM_ENERGIES_H

#include "crc_ccitt_ffff.h"
#include "eeprom.h"
#include "em_energies_wrp.h"
#include "factory_settings.h"

#define LOG_ENERGIES_ACCUMULATION_TIME  5                                                     //accumulate energies for these seconds before writing it to the memomry.
#ifdef SINGLE_PHASE_METER
  #define ACCUMULATED_ENERGY_COUNT      (FACTORY_METER_V_REF*FACTORY_METER_I_MAX/3600)*2      //maximum watts per seconds
#else
  #define ACCUMULATED_ENERGY_COUNT      (FACTORY_METER_V_REF*FACTORY_METER_I_MAX*3/3600)*2    //maximum watts per seconds
#endif //SINGLE_PHASE_METER
#define MINIMUM_ENERGY_COUNT            1       //(1 = 1 watt)
#define MAX_FORWARD_TRY                 3       //write fail check
#define MAX_BACKWARD_TRY                3       //read fail check

typedef enum
{
  e_Import_Only,
  e_Import_Export,
}e_metering_mode_type;

typedef enum
{
    Active_Imp,
    Active_Exp,
    Reactive_Ind_Imp,
    Reactive_Ind_Exp,
    Reactive_Cap_Imp,
    Reactive_Cap_Exp,
    Apparent_Imp,
    Apparent_Exp,
}ENERGY_TYPE;

extern EM_USER_ENERGY_COUNTER myEnergies;
extern e_metering_mode_type e_metering_mode;

void read_energies_from_backend(EM_USER_ENERGY_COUNTER *energies);
void read_energies_from_storage(EM_USER_ENERGY_COUNTER *energies);
void write_energies_to_storage(EM_USER_ENERGY_COUNTER *energies, uint8_t forceWrite);
uint8_t get_metering_mode(void);
void set_metering_mode(uint8_t metering_mode);
void restore_energies_in_metrology(EM_USER_ENERGY_COUNTER *energies);
#endif