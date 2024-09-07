#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>

#include "uart_debug_print.h"
/***********************************************************************************************************************
* Function Name : uart_printf
* Interface     : int uart_printf(const char __far *format, ...)
* Description   : This function print out a string with format to UART2
*               : Not overriding compiler default printf due to different near far specification
* Arguments     : const char * format:
* Return Value  : int
 **********************************************************************************************************************/
int uart_debug_print(const char __far *format, ...)
{
    uint8_t buffer[256];
    uint16_t length, i;
    #ifdef __ICCRL78__
    va_list     arg = { NULL };
    #else
    va_list     arg = NULL; 
    #endif /* __ICCRL78__ */
    
    /* Parse the argument list, print to buffer string */
    va_start(arg, format);
    
    /* Format the string */
    length = vsprintf((char *)buffer, (const char *)format, arg);
    
    /* Print out to UART2 TX: optical port */
    send_message(COMM_PORT, buffer, length);
    for(i = 0; i < length; i++)
    {
      MCU_Delay(1200);
    }

    va_end(arg);
    
    return length;
}