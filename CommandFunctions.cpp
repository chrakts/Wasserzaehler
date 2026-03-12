/*
 * CommandFunctions.cpp
 */

#include "CommandFunctions.h"
#include "External.h"
#include "secrets.h"

INFORMATION information[NUM_INFORMATION]=
{
  {"DT",'t','1','N',STRING,3,(void*)strStatusNachtabsenkung,gotStatusNachtabsenkung},
};

COMMAND cnetCommands[NUM_COMMANDS] =
	{
    cmultiStandardCommands,
		{'W','s',CUSTOMER,UINT_32,1,jobSetWasserstand},
	};

void gotStatusNachtabsenkung()
{
	if(strStatusNachtabsenkung[1]=='n')
	  statusNachtabsenkung = true;
  else
	  statusNachtabsenkung = false;
}


void jobSetWasserstand(ComReceiver *comRec, char function,char address,char job, void * pMem)
{
  //wasserstand = 9999;
  wasserstand = ((uint32_t*)pMem)[0];
  //eeprom_logger_store_if_changed_safe(&wasserstand);
}

