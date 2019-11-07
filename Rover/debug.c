#include "debug.h"
#include <string.h>

#include <ti/drivers/UART.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/dpl/HwiP.h>

#include "Board.h"


void roverUART(char *outVal)
{
    static UART_Handle uart = NULL;
    if (uart == NULL)
    {
        UART_Params uartParams;
        UART_Params_init(&uartParams);
        uartParams.writeDataMode = UART_DATA_BINARY;
        uartParams.readDataMode = UART_DATA_BINARY;
        uartParams.readReturnMode = UART_RETURN_FULL;
        uartParams.readEcho = UART_ECHO_OFF;
        uartParams.baudRate = 9600;

        uart = UART_open(Board_UART0, &uartParams);
    }

    UART_write(uart, outVal, 1);
}

void dbgOutputLoc(unsigned int outLoc)
{
    if (outLoc > 127)
    {
    epicFail();
    }

    /*
    GPIO_write(Board_GPIO0, outLoc & (1 << 0));
    GPIO_write(Board_GPIO1, outLoc & (1 << 1));
    GPIO_write(Board_GPIO2, outLoc & (1 << 2));
    GPIO_write(Board_GPIO3, outLoc & (1 << 3));
    GPIO_write(Board_GPIO4, outLoc & (1 << 4));
    GPIO_write(Board_GPIO5, outLoc & (1 << 5));
    GPIO_write(Board_GPIO6, outLoc & (1 << 6));

    GPIO_toggle(Board_GPIO7);
    */
}

void epicFail(void)
{
    dbgOutputLoc(DLOC_EPIC_FAIL);

    // Enter a critical zone and spin
    const uintptr_t key = HwiP_disable();
    while (1);
}
