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

/*****************************************************************************

   Application Name     -   MQTT Client
   Application Overview -   The device is running a MQTT client which is
                           connected to the online broker. Three LEDs on the
                           device can be controlled from a web client by
                           publishing msg on appropriate topics. Similarly,
                           message can be published on pre-configured topics
                           by pressing the switch buttons on the device.

   Application Details  - Refer to 'MQTT Client' README.html

*****************************************************************************/
//*****************************************************************************
//
//! \addtogroup mqtt_server
//! @{
//
//*****************************************************************************
/* Standard includes                                                         */
#include <stdlib.h>
#include <pthread.h>
#include <mqueue.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <FreeRTOS.h>
#include <task.h>

/* TI-Driver includes                                                        */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/SPI.h>
#include <ti/drivers/Timer.h>

/* Simplelink includes                                                       */
#include <ti/drivers/net/wifi/simplelink.h>

/* SlNetSock includes                                                        */
#include <ti/drivers/net/wifi/slnetifwifi.h>

/* MQTT Library includes                                                     */
#include <ti/net/mqtt/mqttclient.h>

/* Common interface includes                                                 */
#include "network_if.h"
#include "uart_term.h"

/* Application includes                                                      */
#include "Board.h"
#include "client_cbs.h"
#include "debug.h"
#include "queues.h"
#include "statistics.h"
#include "json_parser.h"
#include "test_publish.h"
#include "spi.h"

//*****************************************************************************
//                          LOCAL DEFINES
//*****************************************************************************

#define CLIENT_INIT_STATE        (0x01)
#define MQTT_INIT_STATE          (0x04)


#define SLNET_IF_WIFI_PRIO       (5)

/* Operate Lib in MQTT 3.1 mode.                                             */
#define MQTT_3_1                 true

/* Defining Broker IP address and port Number                                */
#define SERVER_ADDRESS           "192.168.137.181"
#define PORT_NUMBER              1883

/* Clean session flag                                                        */
#define CLEAN_SESSION            true

/* Retain Flag. Used in publish message.                                     */
#define RETAIN_ENABLE            1

/* Spawn task priority and Task and Thread Stack Size                        */
#define TASKSTACKSIZE            2048
#define RXTASKSIZE               4096
#define MQTTTHREADSIZE           4096
#define SPAWN_TASK_PRIORITY      9

//*****************************************************************************
//                      LOCAL FUNCTION PROTOTYPES
//*****************************************************************************
void * MqttClient(void *pvParameters);
void Mqtt_ClientStop(uint8_t disconnect);
void Mqtt_ServerStop();
void Mqtt_Stop();
void Mqtt_start();
int32_t Mqtt_IF_Connect();
int32_t MqttServer_start();
int32_t MqttClient_start(char *clientId);

//*****************************************************************************
//                 GLOBAL VARIABLES
//*****************************************************************************

/* Connection state: (0) - connected, (negative) - disconnected              */
int32_t gApConnectionState = -1;
static MQTTClient_Handle gMqttClient;

/* Receive task handle                                                       */


/* Client ID                                                                 */
/* If ClientId isn't set, the MAC address of the device will be copied into  */
/* the ClientID parameter.                                                   */

/* Subscription topics and qos values                                        */
#define SUBSCRIPTION_TOPIC_COUNT 4

static const char * const topic[SUBSCRIPTION_TOPIC_COUNT] = {
    "firefighter/status",
    "firefighter/arm/do_something",
    "firefighter/sensor",
    "firefighter/rover/speed"
};

static const unsigned char qos[SUBSCRIPTION_TOPIC_COUNT] = {
    MQTT_QOS_1,
    MQTT_QOS_1,
};

/* Message Queue                                                             */


//*****************************************************************************
//                 Banner VARIABLES
//*****************************************************************************
MQTTClient_ConnParams Mqtt_ClientCtx =
{
    MQTTCLIENT_NETCONN_IP4,
    SERVER_ADDRESS,
    PORT_NUMBER, 0, 0, 0,
    NULL
};

//*****************************************************************************
//
//! Application startup display on UART
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************

void * MqttClientThread(void * pvParameters)
{
    MQTTClient_run((MQTTClient_Handle)pvParameters);

//    UART_PRINT("MQTT client finished\n");

    vTaskSuspend(NULL);
    return(NULL);
}

//*****************************************************************************
//
//! Task implementing MQTT Server plus client bridge
//!
//! This function
//!    1. Initializes network driver and connects to the default AP
//!    2. Initializes the mqtt client ans server libraries and set up MQTT
//!       with the remote broker.
//!    3. set up the button events and their callbacks(for publishing)
//!    4. handles the callback signals
//!
//! \param  none
//!
//! \return None
//!
//*****************************************************************************
void *MqttClient(void *pvParameters) {
    char *clientId = (char *)pvParameters;

    // Initializing Client and Subscribing to the Broker.
    if(gApConnectionState >= 0) {
        if(MqttClient_start(clientId) == -1) {
            UART_PRINT("MQTT Client lib initialization failed\n");
            pthread_exit(0);
            return(NULL);
        }
    }


    while (1) {
        PublishMsg msg;
        if (!receiveFromPublishQueue(&msg)) continue;

        switch(msg.type) {
        case MSG_PUBLISH_STATISTICS: {
            char topic[] = "firefighter/rover/statistics";

            char payload[256];
            snprintf(payload, 256,
                     "{\"publishAttempts\":%d,\"publishSuccesses\":%d}",
                     msg.statistics.publishAttempts, msg.statistics.publishSuccesses);

            MQTTClient_publish(gMqttClient,
                               topic, strlen(topic),
                               payload, strlen(payload),
                               MQTT_QOS_1 | ((RETAIN_ENABLE) ? MQTT_PUBLISH_RETAIN : 0));
            incrementPublishAttempts();

            break;
        }

        case MSG_PUBLISH_STATUS: {
            char topic[] = "firefighter/rover/status";

            char payload[128];
            snprintf(payload, 128, "{\"status\":\"%s\"}", msg.status);

            MQTTClient_publish(gMqttClient,
                               topic, strlen(topic),
                               payload, strlen(payload),
                               MQTT_QOS_1 | ((RETAIN_ENABLE) ? MQTT_PUBLISH_RETAIN : 0));
            incrementPublishAttempts();

            break;
        }
        case MSG_PUBLISH_GENERAL: {
            char topic[] = "firefighter/rover/direction";

            char payload[256];
             create_gen_status_json(payload, msg.gen_info.rover_dir, msg.gen_info.speed);
//             UART_PRINT("INSIDE PUBLISH GEN  \n");
            MQTTClient_publish(gMqttClient,
                               topic, strlen(topic),
                               payload, strlen(payload),
                               MQTT_QOS_1 | ((RETAIN_ENABLE) ? MQTT_PUBLISH_RETAIN : 0));
            incrementPublishAttempts();
            break;

        }
        case MSG_PUBLISH_STOP_THREAD: {
            pthread_exit(0);
            break;
        }
        }
    }
}

//*****************************************************************************
//
//! This function connect the MQTT device to an AP with the SSID which was
//! configured in SSID_NAME definition which can be found in Network_if.h file,
//! if the device can't connect to to this AP a request from the user for other
//! SSID will appear.
//!
//! \param  none
//!
//! \return None
//!
//*****************************************************************************
int32_t Mqtt_IF_Connect()
{
    // Reset The state of the machine
    Network_IF_ResetMCUStateMachine();

    // Start the driver
    long ret = Network_IF_InitDriver(ROLE_STA);
    if (ret < 0) {
        UART_PRINT("Failed to start SimpleLink Device\n", ret);
        return(-1);
    }

    // Initialize AP security params
    SlWlanSecParams_t SecurityParams = { 0 };
    SecurityParams.Key = (signed char *) SECURITY_KEY;
    SecurityParams.KeyLen = strlen(SECURITY_KEY);
    SecurityParams.Type = SECURITY_TYPE;

    // Connect to the Access Point
    if (Network_IF_ConnectAP(SSID_NAME, SecurityParams) < 0) {
        UART_PRINT("Connection to an AP failed\n");
        return(-1);
    }

    return(0);
}

//*****************************************************************************
//!
//! MQTT Start - Initialize and create all the items required to run the MQTT
//! protocol
//!
//! \param  none
//!
//! \return None
//!
//*****************************************************************************
void Mqtt_start(char *clientId)
{
    pthread_attr_t pAttrs;
    struct sched_param priParam;
    int32_t retc = 0;
    /*Set priority and stack size attributes                                 */
    pthread_attr_init(&pAttrs);
    priParam.sched_priority = 2;
    retc = pthread_attr_setschedparam(&pAttrs, &priParam);
    retc |= pthread_attr_setstacksize(&pAttrs, MQTTTHREADSIZE);
    retc |= pthread_attr_setdetachstate(&pAttrs, PTHREAD_CREATE_DETACHED);

    if(retc != 0)
    {
        UART_PRINT("MQTT thread create fail\n");
        return;
    }

    pthread_t mqttThread;
    retc = pthread_create(&mqttThread, &pAttrs, MqttClient, (void *) clientId);
    if(retc != 0)
    {
        UART_PRINT("MQTT thread create fail\n");
        return;
    }
//    UART_PRINT("Created MQttClient\n");

}

//*****************************************************************************
//!
//! MQTT Stop - Close the client instance and free all the items required to
//! run the MQTT protocol
//!
//! \param  none
//!
//! \return None
//!
//*****************************************************************************

void Mqtt_Stop()
{
    if(gApConnectionState >= 0)
    {
        Mqtt_ClientStop(1);
    }

    stopPublishThread();

    sl_Stop(SL_STOP_TIMEOUT);
//    UART_PRINT("\n Client Stop completed\r\n");
}

int32_t MqttClient_start(char *clientId)
{
    int32_t lRetVal = -1;
    int32_t iCount = 0;
//    UART_PRINT("IN MQTT client Start\n");

    int32_t threadArg = 100;
    pthread_attr_t pAttrs;
    struct sched_param priParam;

    MQTTClient_Params MqttClientExmple_params = {0};
    MqttClientExmple_params.clientId = clientId;
    MqttClientExmple_params.connParams = &Mqtt_ClientCtx;
    MqttClientExmple_params.mqttMode31 = MQTT_3_1;
    MqttClientExmple_params.blockingSend = false;

    /*Initialize MQTT client lib                                             */
    gMqttClient = MQTTClient_create(MqttClientCallback, &MqttClientExmple_params);
    if(gMqttClient == NULL) {
        /*lib initialization failed                                          */

        return(-1);
    }

    /*Open Client Receive Thread start the receive task. Set priority and    */
    /*stack size attributes                                                  */
    pthread_attr_init(&pAttrs);
    priParam.sched_priority = 2;

    lRetVal = pthread_attr_setschedparam(&pAttrs, &priParam);
    lRetVal |= pthread_attr_setstacksize(&pAttrs, RXTASKSIZE);
    lRetVal |= pthread_attr_setdetachstate(&pAttrs, PTHREAD_CREATE_DETACHED);

    pthread_t rx_task_hndl;
    lRetVal |= pthread_create(&rx_task_hndl, &pAttrs, MqttClientThread, (void *) &threadArg);
    if(lRetVal != 0)
    {
        UART_PRINT("Client Thread Create Failed failed\n");
        return(-1);
    }

    /*Initiate MQTT Connect                                                  */
    if(gApConnectionState >= 0)
    {
        /*The return code of MQTTClient_connect is the ConnACK value that
           returns from the server */

        lRetVal = MQTTClient_connect(gMqttClient);

        /*negative lRetVal means error,
           0 means connection successful without session stored by the server,
           greater than 0 means successful connection with session stored by
           the server */
        if(0 > lRetVal)
        {
            /*lib initialization failed                                      */
            UART_PRINT("Connection to broker failed: %d\n",
                       lRetVal);
        }
        else
        {
            uint8_t subIndex;
            MQTTClient_SubscribeParams subscriptionInfo[
                SUBSCRIPTION_TOPIC_COUNT];

            for(subIndex = 0; subIndex < SUBSCRIPTION_TOPIC_COUNT; subIndex++)
            {
                subscriptionInfo[subIndex].topic = (char *)topic[subIndex];
                subscriptionInfo[subIndex].qos = qos[subIndex];
            }

            if(MQTTClient_subscribe(gMqttClient, subscriptionInfo, SUBSCRIPTION_TOPIC_COUNT) < 0){
                UART_PRINT("Error subscribing\n");
                MQTTClient_disconnect(gMqttClient);
            } else {
                for(iCount = 0; iCount < SUBSCRIPTION_TOPIC_COUNT; iCount++)
                {
                    UART_PRINT("Client subscribed on %s\n", topic[iCount]);
                }
            }
        }
    }

    return(0);
}

//*****************************************************************************
//!
//! MQTT Client stop - Unsubscribe from the subscription topics and exit the
//! MQTT client lib.
//!
//! \param  none
//!
//! \return None
//!
//*****************************************************************************

void Mqtt_ClientStop(uint8_t disconnect)
{
    uint32_t iCount;
//    UART_PRINT("Client Stopped,\n");
    MQTTClient_UnsubscribeParams subscriptionInfo[SUBSCRIPTION_TOPIC_COUNT];

    for(iCount = 0; iCount < SUBSCRIPTION_TOPIC_COUNT; iCount++)
    {
        subscriptionInfo[iCount].topic = (char *)topic[iCount];
    }

    MQTTClient_unsubscribe(gMqttClient, subscriptionInfo,
                           SUBSCRIPTION_TOPIC_COUNT);
    for(iCount = 0; iCount < SUBSCRIPTION_TOPIC_COUNT; iCount++)
    {
        UART_PRINT("Unsubscribed from the topic %s\r\n", topic[iCount]);
    }

    /*exiting the Client library                                             */
    MQTTClient_delete(gMqttClient);
}

//*****************************************************************************
//!
//! Set the ClientId with its own mac address
//! This routine converts the mac address which is given
//! by an integer type variable in hexadecimal base to ASCII
//! representation, and copies it into the ClientId parameter.
//!
//! \param  macAddress  -   Points to string Hex.
//!
//! \return void.
//!
//*****************************************************************************
int32_t SetClientIdNamefromMacAddress(char *ClientId)
{
    int32_t ret = 0;
    uint8_t Client_Mac_Name[2];
    uint8_t Index;
    uint16_t macAddressLen = SL_MAC_ADDR_LEN;
    uint8_t macAddress[SL_MAC_ADDR_LEN];

    /*Get the device Mac address */
    ret = sl_NetCfgGet(SL_NETCFG_MAC_ADDRESS_GET, 0, &macAddressLen,
                       &macAddress[0]);

    /*When ClientID isn't set, use the mac address as ClientID               */
    if(ClientId[0] == '\0')
    {
        /*6 bytes is the length of the mac address                           */
        for(Index = 0; Index < SL_MAC_ADDR_LEN; Index++)
        {
            /*Each mac address byte contains two hexadecimal characters      */
            /*Copy the 4 MSB - the most significant character                */
            Client_Mac_Name[0] = (macAddress[Index] >> 4) & 0xf;
            /*Copy the 4 LSB - the least significant character               */
            Client_Mac_Name[1] = macAddress[Index] & 0xf;

            if(Client_Mac_Name[0] > 9)
            {
                /*Converts and copies from number that is greater than 9 in  */
                /*hexadecimal representation (a to f) into ascii character   */
                ClientId[2 * Index] = Client_Mac_Name[0] + 'a' - 10;
            }
            else
            {
                /*Converts and copies from number 0 - 9 in hexadecimal       */
                /*representation into ascii character                        */
                ClientId[2 * Index] = Client_Mac_Name[0] + '0';
            }
            if(Client_Mac_Name[1] > 9)
            {
                /*Converts and copies from number that is greater than 9 in  */
                /*hexadecimal representation (a to f) into ascii character   */
                ClientId[2 * Index + 1] = Client_Mac_Name[1] + 'a' - 10;
            }
            else
            {
                /*Converts and copies from number 0 - 9 in hexadecimal       */
                /*representation into ascii character                        */
                ClientId[2 * Index + 1] = Client_Mac_Name[1] + '0';
            }
        }
    }
    return(ret);
}

void mainThread(void * args)
{
    pthread_t spawn_thread = (pthread_t) NULL;
    pthread_attr_t pAttrs_spawn;
    struct sched_param priParam;
    int32_t retc = 0;
    UART_Handle tUartHndl;

    /*Initialize SlNetSock layer with CC31xx/CC32xx interface */
    SlNetIf_init(0);
    SlNetIf_add(SLNETIF_ID_1, "CC32xx",
                (const SlNetIf_Config_t *)&SlNetIfConfigWifi,
                SLNET_IF_WIFI_PRIO);

    SlNetSock_init(0);
    SlNetUtil_init(0);

    GPIO_init();
    SPI_init();
    Timer_init();

    GPIO_setConfig(Board_GPIO0, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_write(Board_GPIO0, 1);

    initMsgQueues();
    initStatistics();
    init_publish();
    /*Configure the UART                                                     */
    tUartHndl = InitTerm();
    /*remove uart receive from LPDS dependency                               */
    UART_control(tUartHndl, UART_CMD_RXDISABLE, NULL);

    // Create the sl_Task
    pthread_attr_init(&pAttrs_spawn);
    priParam.sched_priority = SPAWN_TASK_PRIORITY;
    retc = pthread_attr_setschedparam(&pAttrs_spawn, &priParam);
    retc |= pthread_attr_setstacksize(&pAttrs_spawn, TASKSTACKSIZE);
    retc |= pthread_attr_setdetachstate(&pAttrs_spawn, PTHREAD_CREATE_DETACHED);
    if (retc != 0) {
        epicFail();
    }

    if(pthread_create(&spawn_thread, &pAttrs_spawn, sl_Task, NULL) != 0) {
        epicFail();
    }

    if(sl_Start(0, 0, 0) < 0) {
        epicFail();
    }

    // Set the ClientId with its own MAC address
    char clientId[13] = {'\0'};
    SetClientIdNamefromMacAddress(clientId);

    if(sl_Stop(SL_STOP_TIMEOUT) < 0) {
        epicFail();
    }
    // Connect to AP
    gApConnectionState = Mqtt_IF_Connect();

    // Start MQTT client thread
    Mqtt_start(clientId);

    GPIO_write(Board_GPIO0, 1);

    while (1) {
        MainMsg msg;
        if (!receiveFromMainQueue(&msg)) continue;

        switch (msg.type) {
        case MSG_MAIN_STATUS: {
            UART_PRINT("Got status update: %d\n", msg.status);

            if (msg.status == STATUS_START) {
                publishBoardStatus("Starting up");
            } else if (msg.status == STATUS_STOP) {
                publishBoardStatus("Shutting down");
            }

            break;
        }
        }
    }
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
