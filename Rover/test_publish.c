/*
 * test_publish.c
 *
 *  Created on: Oct 17, 2019
 *      Author: Abs
 */


#include <ti/drivers/Timer.h>
#include "Board.h"

#include "test_publish.h"
#include "queues.h"
#include "debug.h"
#include "rover_state.h"

#define TIMER_PERIOD_MS 500

void GenInfoTimerCallback(Timer_Handle handle) {

    publishGeneralMessageFromISR(getSpeed(), getDirection());
}

void init_publish(void) {
    Timer_Params params;
    Timer_Params_init(&params);
    params.period = TIMER_PERIOD_MS * 1000;
    params.periodUnits = Timer_PERIOD_US;
    params.timerMode = Timer_CONTINUOUS_CALLBACK;
    params.timerCallback = GenInfoTimerCallback;

    Timer_Handle timer = Timer_open(Board_GEN_INFO, &params);

    if (!timer) {
        /* Failed to initialized timer */
        epicFail();
    }

    if (Timer_start(timer) == Timer_STATUS_ERROR) {
        /* Failed to start timer */
        epicFail();
    }
}

