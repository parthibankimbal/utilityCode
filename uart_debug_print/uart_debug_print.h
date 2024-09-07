#include "communication.h"
#include "wrp_app_mcu.h"

#ifndef UART_DEBUG_PRINT
#define UART_DEBUG_PRINT

#define COMM_PORT    OPTICAL_COMM_PORT

int uart_debug_print(const char __far *format, ...);

#endif