#include <rover_state.h>
#include "debug.h"
#include "queues.h"
#include <stdio.h>

Direction MQTT_GetDirection(void){

    //return rover_test_1();    //Test Case 1
    //return rover_test_2();  //Test Case 2
    //return rover_test_3();  //Test Case 9 (Emergency Stop)
    //return rover_test_4();  //Test Case 4
    return rover_test_5();  //Test Case 5
}

void change_speed(unsigned int s){
    switch(s){
    case 0: speed = 0x00;
    break;
    case 1: speed = 0x11;
    break;
    case 2: speed = 0x22;
    break;
    case 3: speed = 0x32;
    break;
    case 4: speed = 0x42;
    break;
    case 5: speed = 0x52;
    }
}

int getSpeed(){
    int s;

    switch(speed){
    case 0x00: s=0;
    break;
    case 0x11: s=1;
    break;
    case 0x22: s=2;
    break;
    case 0x32: s=3;
    break;
    case 0x42: s=4;
    break;
    case 0x52: s=5;
    break;
    default: s=0;
    break;
    }

    return s;
}

char *getDirection(){

    switch(dir){
    case forward: return "Forward";
    break;
    case reverse: return "Reverse";
    break;
    case right: return "Right";
    break;
    case left: return "Left";
    break;
    case stop: return "Stop";
    break;
    case none: return "None";
    break;
    default: return "Error";
    break;
    }

    return "Error";
}

void moveRover(Direction d){
    static char msg[8] = {0};
    static int len;
    static int baud_rate = 0;
    static int delay = 0;
    static int stop_count = 0;
    unsigned int checksum1, checksum2, checksum3;

   static int send_baud = 1;

   dir = d;

   if(send_baud == 1){
       baud_rate = 1;
       len = snprintf(msg, sizeof(baud), "%c", baud);
       roverUART(msg);

       send_baud = 0;
   }


    switch(d){
    case forward:
        checksum1 = (motor1 + 0 + 0) & checkSumMask;        //left forward, right back
        checksum2 = (motor2 + 1 + speed) & checkSumMask;
        checksum3 = (motor3 + 0 + speed) & checkSumMask;

        len = snprintf(msg, 8, "%c", motor1);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", 0x00);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", 0x00);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", checksum1);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", motor2);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", 1);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", speed);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", checksum2);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", motor3);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", 0);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", speed);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", checksum3);
        roverUART(msg);

    break;
    case left:
        checksum1 = (motor1 + 0 + speed) & checkSumMask;        //right forward, left back
        checksum2 = (motor2 + 0 + speed) & checkSumMask;
        checksum3 = (motor3 + 0 + speed) & checkSumMask;

        len = snprintf(msg, 8, "%c", motor1);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", 0x00);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", speed);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", checksum1);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", motor2);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", 0);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", speed);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", checksum2);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", motor3);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", 0);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", speed);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", checksum3);
        roverUART(msg);

    break;
    case right:
        checksum1 = (motor1 + 1 + speed) & checkSumMask;    //all forward
        checksum2 = (motor2 + 1 + speed) & checkSumMask;
        checksum3 = (motor3 + 1 + speed) & checkSumMask;

        len = snprintf(msg, 8, "%c", motor1);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", 1);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", speed);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", checksum1);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", motor2);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", 1);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", speed);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", checksum2);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", motor3);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", 1);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", speed);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", checksum3);
        roverUART(msg);

    break;
    case reverse:
        checksum1 = (motor1 + 0 + 0) & checkSumMask;      //all reverse
        checksum2 = (motor2 + 0 + speed) & checkSumMask;
        checksum3 = (motor3 + 1 + speed) & checkSumMask;

        len = snprintf(msg, 8, "%c", motor1);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", 0x00);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", 0x00);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", checksum1);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", motor2);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", 0x00);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", speed);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", checksum2);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", motor3);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", 1);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", speed);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", checksum3);
        roverUART(msg);

    break;
    case stop:
        checksum1 = (motor1 + 0 + 0) & checkSumMask;          //all speed 0 (stop)
        checksum2 = (motor2 + 0 + 0) & checkSumMask;
        checksum3 = (motor3 + 0 + 0) & checkSumMask;

        len = snprintf(msg, 8, "%c", motor1);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", 0x00);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", 0x00);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", checksum1);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", motor2);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", 0x00);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", 0x00);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", checksum2);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", motor3);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", 0x00);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", 0x00);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", checksum3);
        roverUART(msg);

    break;
    default:
        checksum1 = (motor1 + 0 + 0) & checkSumMask;          //default = stop
        checksum2 = (motor2 + 0 + 0) & checkSumMask;
        checksum3 = (motor3 + 0 + 0) & checkSumMask;

        len = snprintf(msg, 8, "%c", motor1);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", 0x00);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", 0x00);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", checksum1);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", motor2);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", 0x00);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", 0x00);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", checksum2);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", motor3);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", 0x00);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", 0x00);
        roverUART(msg);
        len = snprintf(msg, 8, "%c", checksum3);
        roverUART(msg);
    }
}

int updateRoverState(RoverState *roverState, int timeInc) {
    static Direction curr = stop;

    switch (roverState->state) {
    case STATE_INIT:
        roverState->curTime = 0;

        roverState->state = STATE_WAITING_FOR_TIME_1;
        break;

    case STATE_WAITING_FOR_TIME_1:
    case STATE_WAITING_FOR_TIME_2:
    case STATE_WAITING_FOR_TIME_3:
    case STATE_WAITING_FOR_TIME_4:
        if(timeInc > 0){

        roverState->dir = MQTT_GetDirection();

        char d[2] = {0};
        switch(roverState->dir){
        case right: snprintf(d, 256, "R");
        break;
        case left: snprintf(d, 256, "L");
        break;
        case forward: snprintf(d, 256, "F");
        break;
        case reverse: snprintf(d, 256, "B");
        break;
        case stop:
            snprintf(d, 256, "S");

        break;
        default:
        }
            char msg[50] = {0};
            int len;

            moveRover(roverState->dir);

            if (roverState->state == STATE_WAITING_FOR_TIME_4) {
            }

            if (roverState->state == STATE_WAITING_FOR_TIME_4) {
                roverState->state = STATE_WAITING_FOR_TIME_1;
            } else {
                roverState->state += 1;
            }
        }
        break;

    default:
        return 0;
    }

    return 1;
}
