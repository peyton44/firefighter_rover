#pragma once

#define baud 0xaa    //baud rate
//#define motor1 0b00001111  //Address 128
#define motor1 0x80
//#define motor2 0b00001011  //Address 129
#define motor2 0x81
//#define motor3 0b00001101  //Address 130
#define motor3 0x82
//#define speed  0b01000010  //Speed 66
static unsigned char speed = 0x42;
//#define checkSumMask 0b01111111
#define checkSumMask 0x7f

typedef enum direction {right, left, forward, reverse, stop, none} Direction;

static Direction dir = none;

Direction MQTT_GetDirection();
void moveRover(Direction d);

typedef enum State {
    STATE_INIT = 1,
    STATE_WAITING_FOR_TIME_1,
    STATE_WAITING_FOR_TIME_2,
    STATE_WAITING_FOR_TIME_3,
    STATE_WAITING_FOR_TIME_4,
} State;

typedef struct RoverState {
    State state;

    int curTime;

    Direction dir;
} RoverState;

int updateRoverState(RoverState *roverState, int timeInc);
void change_speed(unsigned int s);
int getSpeed();
char *getDirection();
