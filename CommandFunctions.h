/*
 * CommandFunctions.h
 *
 * Created: 26.04.2017 14:55:18
 *  Author: a16007
 */


#ifndef COMMANDFUNCTIONS_H_
#define COMMANDFUNCTIONS_H_

#include "wasserzaehler.h"

extern COMMAND cnetCommands[];
extern INFORMATION information[];

#define NUM_COMMANDS 1+CMULTI_STANDARD_NUM
#define NUM_INFORMATION 1

void gotStatusNachtabsenkung();

void jobSetWasserstand(ComReceiver *comRec, char function,char address,char job, void * pMem);

#endif /* COMMANDFUNCTIONS_H_ */
