/*
 * spi.c
 *
 *  Created on: Nov 6, 2019
 *      Author: peyto
 */

#include <ti/drivers/Timer.h>
#include <ti/drivers/SPI.h>
#include "Board.h"

#include "test_publish.h"
#include "queues.h"
#include "debug.h"
#include "rover_state.h"


void SPI_Transfer(SPI_Transaction *transaction){
    static SPI_Handle spi = NULL;

    if(spi == NULL){
        SPI_Params spiParams;
        SPI_Params_init(&spiParams);
        spi = SPI_open(Board_SPI0, &spiParams);
    }

    uint8_t         transmitBuffer[8];
    uint8_t         receiveBuffer[8];
    bool            transferOK;

    transferOK = SPI_transfer(spi, &transaction);
}


