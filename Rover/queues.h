#ifndef QUEUES_H_
#define QUEUES_H_

void initMsgQueues(void);


//
// PublishMsg
//

typedef enum PublishMsgType {
    MSG_PUBLISH_STATISTICS,
    MSG_PUBLISH_STATUS,
    MSG_PUBLISH_STOP_THREAD,
    MSG_PUBLISH_GENERAL
} PublishMsgType;

typedef struct PublishMsg {
    PublishMsgType type;

    union {
        char status[64];
        struct {
            int publishAttempts;
            int publishSuccesses;
        } statistics;
        struct { //vchar*arm_stat, char* rover_dir ,double camera_temp, double heater_temp
            char rover_dir[15];
            int speed;
        }gen_info;
    };
} PublishMsg;

void publishBoardStatus(char *status);

void publishStatisticsFromISR(int publishAttempts, int publishSuccesses);

void stopPublishThread(void);

int receiveFromPublishQueue(PublishMsg *msg);

void publishGeneralMessageFromISR(int speed, char *rover_dir);
//
// MainMsg
//

typedef enum MainMsgType {
    MSG_MAIN_STATUS,
} MainMsgType;

typedef enum StatusType {
    STATUS_START,
    STATUS_STOP,
} StatusType;

typedef struct MainMsg {
    MainMsgType type;

    union {
        StatusType status;
    };
} MainMsg;

void updateStatus(StatusType status);

int receiveFromMainQueue(MainMsg *msg);

#endif /* QUEUES_H_ */
