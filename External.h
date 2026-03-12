/*
 * External.h
 *
 * Created: 03.04.2017 21:04:41
 *  Author: Christof
 */



#ifndef EXTERNAL_H_
#define EXTERNAL_H_

#include <avr/io.h>
#include <stdbool.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "timer.h"
#include "Communication.h"
#include "myTimers.h"
#include "myconstants.h"
#include "secrets.h"
#include "eeprom_logger.h"

extern uint16_t actReportBetweenBlocks;
extern uint16_t actReportBetweenSensors;
extern uint16_t actWaitAfterLastSensor;

extern volatile uint8_t statusReport;
extern volatile bool sendStatusReport;

extern volatile bool doEEpromStoring;

extern volatile char strStatusNachtabsenkung[5];
extern volatile bool statusNachtabsenkung;

extern volatile uint32_t wasserstand;

class Communication;   // Forward declaration
class ComReceiver;
extern Serial debug;
extern Communication cnet;
extern ComReceiver cnetRec;
extern CRC_Calc crcGlobal;

#endif /* EXTERNAL_H_ */
