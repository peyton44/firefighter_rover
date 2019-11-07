/*
 * Copyright (c) 2016, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//*****************************************************************************
//
//! \addtogroup mqtt_server
//! @{
//
//*****************************************************************************
/* Standard includes                                                         */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "json_parser.h"
/* Kernel (Non OS/Free-RTOS/TI-RTOS) includes                                */
#include "pthread.h"
#include "mqueue.h"

/* Common interface includes                                                 */
#include "uart_term.h"
#define JSMN_STATIC
/* Application includes                                                      */
#include "client_cbs.h"
#include "debug.h"
#include "queues.h"
#include "jsmn.h"
#include "statistics.h"
#include "rover_state.h"


#include <ti/drivers/dpl/HwiP.h>

//*****************************************************************************
//                          LOCAL DEFINES
//*****************************************************************************
#define APP_PRINT               Report

#define OS_WAIT_FOREVER         (0xFFFFFFFF)
#define OS_NO_WAIT              (0)
#define OS_OK                   (0)

#define MQTTClientCbs_ConnackRC(data) (data & 0xff) 
/**< CONNACK: Return Code (LSB) */

//*****************************************************************************
//                 GLOBAL VARIABLES
//*****************************************************************************

/* Message Queue                                                              */
static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
  if (tok->type == JSMN_STRING && (int)strlen(s) == tok->end - tok->start &&
      strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
    return 0;
  }
  return -1;
}

//****************************************************************************
//                      CLIENT CALLBACKS
//****************************************************************************

//*****************************************************************************
//
//! Callback in case of various event (for clients connection with remote
//! broker)
//!
//! \param[in]  event       - is a event occurred
//! \param[in]  metaData    - is the pointer for the message buffer
//!                           (for this event)
//! \param[in]  metaDateLen - is the length of the message buffer
//! \param[in]  data        - is the pointer to the buffer for data
//!                           (for this event)
//! \param[in]  dataLen     - is the length of the buffer data
//!
//! return none
//
//*****************************************************************************
void MqttClientCallback(int32_t event,
                        void * metaData,
                        uint32_t metaDateLen,
                        void *data,
                        uint32_t dataLen)
{
//    UART_PRINT("in MqttClientCallbak\n");

    dbgOutputLoc(DLOC_BEFORE_MQTT_CB);

    switch((MQTTClient_EventCB)event) {
    case MQTTClient_OPERATION_CB_EVENT: {
        switch(((MQTTClient_OperationMetaDataCB *)metaData)->messageType) {
        case MQTTCLIENT_OPERATION_CONNACK: {
            uint16_t ConnACK = *(uint16_t *)data;

            /* Check if Conn Ack return value is Success (0) or       */
            /* Error - Negative value                                 */
            if(0 == MQTTClientCbs_ConnackRC(ConnACK))
            {
                APP_PRINT("Connected to broker\n");
            }
            else
            {
                APP_PRINT("CONNACK: Connection Error: %d\n", ConnACK);
            }
            break;
        }

        case MQTTCLIENT_OPERATION_EVT_PUBACK: {
            incrementPublishSuccesses();
            break;
        }

        case MQTTCLIENT_OPERATION_SUBACK: {
            APP_PRINT("Sub Ack\n");
            break;
        }

        case MQTTCLIENT_OPERATION_UNSUBACK: {
            char *UnSub = data;
            APP_PRINT("UnSub Ack: %s\n", UnSub);
            break;
        }
        }
        break;
    }
    case MQTTClient_RECV_CB_EVENT:
    {
        MQTTClient_RecvMetaDataCB *recvMetaData =
            (MQTTClient_RecvMetaDataCB *)metaData;

        const char *topic = recvMetaData->topic, *payload = data;
        size_t topicLen = recvMetaData->topLen, payloadLen = dataLen;

        //        APP_PRINT("Got topic \"%.*s\" with payload \"%.*s\"\n", topicLen, topic, payloadLen, payload);
        static unsigned int s = 0;
        static Direction  dir = none;

        if (!strncmp(topic, "firefighter/arm/do_something", topicLen)) {
            parse_string(payload, payloadLen);
        }else if (!strncmp(topic, "firefighter/rover/speed", topicLen)){
            jsmn_parser p;
            jsmntok_t t[4];

            jsmn_init(&p);
            int r = jsmn_parse(&p, payload, payloadLen, t, 4);
            if (r != 3 || t[0].type != JSMN_OBJECT) {
                APP_PRINT("Invalid payload\n");
                break;
            }

            if (jsoneq(payload, &t[1], "speed")) {
                APP_PRINT("speed expected\n");
                break;
            }

            const char *value = payload + t[2].start;
            size_t valueLen = t[2].end - t[2].start;
            APP_PRINT("Speed is \"%.*s\"\n",  valueLen, value);

            if (!strncmp(value, "0", 1)) {
                s = 0;
            } else if (!strncmp(value, "1", 1)) {
                s = 1;
            } else if (!strncmp(value, "2", 1)) {
                s = 2;
            } else if (!strncmp(value, "3", 1)) {
                s = 3;
            } else if (!strncmp(value, "4", 1)) {
                s = 4;
            } else if (!strncmp(value, "5", 1)) {
                s = 5;
            } else {
                s = 0;
            }

            change_speed(s);
            moveRover(dir);

        } else if (!strncmp(topic, "firefighter/sensor", topicLen)){
            jsmn_parser p;
            jsmntok_t t[4];

            jsmn_init(&p);
            int r = jsmn_parse(&p, payload, payloadLen, t, 4);
            if (r != 3 || t[0].type != JSMN_OBJECT) {
                APP_PRINT("Invalid payload\n");
                GPIO_write(Board_GPIO0, 0);
                break;
            }

            if (jsoneq(payload, &t[1], "direction")) {
                APP_PRINT("direction expected\n");
                GPIO_write(Board_GPIO0, 0);
                break;
            }

            const char *value = payload + t[2].start;
            size_t valueLen = t[2].end - t[2].start;
            APP_PRINT("Direction is \"%.*s\"\n",  valueLen, value);

            GPIO_write(Board_GPIO0, 1);

            if (!strncmp(value, "forward", 7)) {
                dir = forward;
            } else if (!strncmp(value, "re", 2)) {
                dir = reverse;
            } else if (!strncmp(value, "right", 5)) {
                dir = right;
            } else if (!strncmp(value, "left", 4)) {
                dir = left;
            } else if (!strncmp(value, "stop", 4)) {
                dir = stop;
            } else if (!strncmp(value, "estop", 5)) {
                dir = stop;
            } else {
                dir = stop;
            }

            moveRover(dir);

        } else if (!strncmp(topic, "firefighter/status", topicLen)) {
            jsmn_parser p;
            jsmntok_t t[4];

            jsmn_init(&p);
            int r = jsmn_parse(&p, payload, payloadLen, t, sizeof(t) / sizeof(t[0]));
            if (r != 3 || t[0].type != JSMN_OBJECT) {
                APP_PRINT("Invalid payload\n");
                break;
            }

            if (jsoneq(payload, &t[1], "status")) {
                APP_PRINT("Status expected\n");
                break;
            }

            const char *value = payload + t[2].start;
            size_t valueLen = t[2].end - t[2].start;
            APP_PRINT("Status is \"%.*s\"\n",  valueLen, value);

            if (!jsoneq(payload, &t[2], "start")) {
                updateStatus(STATUS_START);
            } else if (!jsoneq(payload, &t[2], "stop")) {
                updateStatus(STATUS_STOP);
            }
        }

        break;
    }

    case MQTTClient_DISCONNECT_CB_EVENT: {
        APP_PRINT("Disconnected from broker\n");
        moveRover(stop);
        break;
    }
    }

    dbgOutputLoc(DLOC_AFTER_MQTT_CB);
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
