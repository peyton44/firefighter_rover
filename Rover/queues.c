#include <FreeRTOS.h>
#include <queue.h>

#include "queues.h"
#include "debug.h"
#include "string.h"
QueueHandle_t publishQueue = NULL, mainQueue = NULL;

void initMsgQueues(void) {
    publishQueue = xQueueCreate(10, sizeof(PublishMsg));
    mainQueue = xQueueCreate(10, sizeof(MainMsg));
}


//
// PublishMsg helpers
//

void publishBoardStatus(char *status) {
    PublishMsg msg = { .type = MSG_PUBLISH_STATUS };
    strncpy(msg.status, status, 64);

    dbgOutputLoc(DLOC_BEFORE_SEND_RECV_QUEUE);
    xQueueSend(publishQueue, &msg, portMAX_DELAY);
    dbgOutputLoc(DLOC_AFTER_SEND_RECV_QUEUE);
}

void publishStatisticsFromISR(int publishAttempts, int publishSuccesses) {
    PublishMsg msg = {
        .type = MSG_PUBLISH_STATISTICS,
        .statistics.publishAttempts = publishAttempts,
        .statistics.publishSuccesses = publishSuccesses
    };

    dbgOutputLoc(DLOC_BEFORE_SEND_RECV_QUEUE);
    xQueueSendFromISR(publishQueue, &msg, NULL);
    dbgOutputLoc(DLOC_AFTER_SEND_RECV_QUEUE);
}


void publishGeneralMessageFromISR(int speed, char *rover_dir) {
   PublishMsg msg = {
        .type = MSG_PUBLISH_GENERAL,
        .gen_info.speed = speed
    };
   strcpy(msg.gen_info.rover_dir, rover_dir);

    dbgOutputLoc(DLOC_BEFORE_SEND_RECV_QUEUE);
    xQueueSendFromISR(publishQueue, &msg, NULL);
    dbgOutputLoc(DLOC_AFTER_SEND_RECV_QUEUE);
}

void stopPublishThread(void) {
    PublishMsg msg = { .type = MSG_PUBLISH_STOP_THREAD };

    dbgOutputLoc(DLOC_BEFORE_SEND_RECV_QUEUE);
    xQueueSend(publishQueue, &msg, portMAX_DELAY);
    dbgOutputLoc(DLOC_AFTER_SEND_RECV_QUEUE);
}

int receiveFromPublishQueue(PublishMsg *msg) {
    dbgOutputLoc(DLOC_BEFORE_SEND_RECV_QUEUE);
    int success = xQueueReceive(publishQueue, msg, portMAX_DELAY) == pdTRUE;
    dbgOutputLoc(DLOC_BEFORE_SEND_RECV_QUEUE);

    return success;
}


//
// MainMsg helpers
//

void updateStatus(StatusType status) {
    MainMsg msg = { .type = MSG_MAIN_STATUS, .status = status };

    dbgOutputLoc(DLOC_BEFORE_SEND_RECV_QUEUE);
    xQueueSend(mainQueue, &msg, portMAX_DELAY);
    dbgOutputLoc(DLOC_AFTER_SEND_RECV_QUEUE);
}

int receiveFromMainQueue(MainMsg *msg) {
    dbgOutputLoc(DLOC_BEFORE_SEND_RECV_QUEUE);
    int success = xQueueReceive(mainQueue, msg, portMAX_DELAY) == pdTRUE;
    dbgOutputLoc(DLOC_BEFORE_SEND_RECV_QUEUE);

    return success;
}
