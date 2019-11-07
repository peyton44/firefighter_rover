#pragma once

void dbgUARTVal(char *outVal);

void roverUART(char *outVal);

void dbgOutputLoc(unsigned int outLoc);

void epicFail(void);

enum OutputLocs {
    DLOC_ENTER_MAIN_TASK                = 0x01,
    DLOC_ENTER_MAIN_TASK_LOOP           = 0x02,
    DLOC_BEFORE_SEND_RECV_QUEUE         = 0x03,
    DLOC_AFTER_SEND_RECV_QUEUE          = 0x04,
    DLOC_ENTER_ISR                      = 0x05,
    DLOC_LEAVE_ISR                      = 0x06,
    DLOC_BEFORE_SEND_RECV_QUEUE_ISR     = 0x07,
    DLOC_AFTER_SEND_RECV_QUEUE_ISR      = 0x08,

    DLOC_BEFORE_MQTT_CB = 0x10,
    DLOC_AFTER_MQTT_CB = 0x11,

    DLOC_MALLOC_FAILED                  = 0x60,
    DLOC_STACK_OVERFLOW                 = 0x61,

    DLOC_EPIC_FAIL                      = 0x7f
};
