#ifndef PUSH_SCHEDULAR_USER
#define PUSH_SCHEDULAR_USER

#include <stdint.h>
#include <string.h>
#include "push_schedular.h"
#include "eeprom.h"
#include "crc_ccitt_ffff.h"
#include "factory_settings.h"

#ifdef UTILITY_JNK
#define DEFAULT_INSTANT_PROFILE_PUSH_ACTIONS    {1, \
                                                            {0xFF,  0xFF,  0xFF, 0xFF, 0x00, 0x00, \
                                                             0xFF,  0xFF,  0xFF, 0xFF, 0xFF, 0xFF, \
                                                             0xFF,  0xFF,  0xFF, 0xFF, 0xFF, 0xFF, \
                                                             0xFF,  0xFF,  0xFF, 0xFF, 0xFF, 0xFF},\
                                                             0,\
                                                             0,\
                                                             0,\
                                                }
#elif ((UTILITY_INTELLI) || (UTILITY_APRAAVA) || (UTILITY_PURBANCHAL))
#define DEFAULT_INSTANT_PROFILE_PUSH_ACTIONS    {2, \
                                                            {0xFF,  0xFF,  0xFF, 0xFF,  0, 0, \
                                                             0xFF,  0xFF,  0xFF, 0xFF, 30, 0, \
                                                             0xFF,  0xFF,  0xFF, 0xFF, 0xFF, 0xFF, \
                                                             0xFF,  0xFF,  0xFF, 0xFF, 0xFF, 0xFF},\
                                                             0,\
                                                             0,\
                                                             0,\
                                                }
#endif

extern st_push_schedular st_Instant_push_time;
void set_default_instant_push_profile_time(void);
void set_instant_push_profile_time(st_push_schedular* Instant_push_time);
void get_instant_push_profile_time(st_push_schedular* Instant_push_time);
#endif //PUSH_SCHEDULAR_USER