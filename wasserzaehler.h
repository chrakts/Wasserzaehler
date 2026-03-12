
#ifndef LUEFTERSTEUERUNG_H_
#define LUEFTERSTEUERUNG_H_

#include <avr/io.h>
#include <stdbool.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "myconstants.h"
#include "Serial.h"
#include "External.h"
#include "timer.h"
#include "cmultiStandardCommands.h"
#include "ComReceiver.h"
#include "CommandFunctions.h"
#include "Communication.h"
#include "xmegaClocks.h"

#include "xxHardware.h"
#include "ledHardware.h"

void initZaehler();

#endif /* LUEFTERSTEUERUNG_H_ */
