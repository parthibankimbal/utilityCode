#ifndef EM_ENERGIES_WRP_H
#define EM_ENERGIES_WRP_H

#include "typedef.h"
#include "em_type.h"
#include "em_measurement.h"
#include "em_operation.h"
#include "factory_settings.h"

void WPR_em_get_energies(EM_ENERGY_DATA *energies);
void WPR_em_set_energies(EM_ENERGY_DATA *energies);
void get_operational_EM_data(EM_OPERATION_DATA *test_op_data);
void set_operational_EM_data(EM_OPERATION_DATA *test_op_data);
#endif //EM_ENERGIES_WRP_H