#include <ti/drivers/Timer.h>
#include "Board.h"

#include "statistics.h"
#include "queues.h"
#include "debug.h"
#include "uart_term.h"

#define TIMER_PERIOD_MS 5000

static int publishAttempts;
static int publishSuccesses;

void statisticsTimerCallback(Timer_Handle handle) {
    publishStatisticsFromISR(publishAttempts, publishSuccesses);
}

void initStatistics(void) {
    publishAttempts = 0;
    publishSuccesses = 0;

    Timer_Params params;
    Timer_Params_init(&params);
    params.period = TIMER_PERIOD_MS * 1000;
    params.periodUnits = Timer_PERIOD_US;
    params.timerMode = Timer_CONTINUOUS_CALLBACK;
    params.timerCallback = statisticsTimerCallback;

    Timer_Handle timer = Timer_open(Board_Statistics, &params);

    if (!timer) {
        /* Failed to initialized timer */
        epicFail();
    }

    if (Timer_start(timer) == Timer_STATUS_ERROR) {
        /* Failed to start timer */
        epicFail();
    }
}

void incrementPublishAttempts(void) {
    publishAttempts++;
}

void incrementPublishSuccesses(void) {
    publishSuccesses++;
}
