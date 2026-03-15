#include "myTimers.h"
#include "ledHardware.h"

// 1:  9.9  ms
// 2:  19.8 ms
// 5:  49.4 ms
//10:  99.0 ms
//101: 1000 ms
//1000: 10s

volatile TIMER MyTimers[MYTIMER_NUM]= {	{TM_START,RESTART_YES,actReportBetweenSensors,0,nextReportStatus},
                                        {TM_START,RESTART_YES,30000,0,storeEEprom}, // 30000  alle 5 Minuten
                                        {TM_START,RESTART_YES,100,0,led1Blinken},
                                        {TM_STOP,RESTART_NO,5,0,entprellen}
};

void led1Blinken(uint8_t test)
{
	//LEDROT_TOGGLE;
}

void entprellen(uint8_t test)
{
  PORTC_INT0MASK |= PIN2_bm;
}

void storeEEprom(uint8_t test)
{
  doEEpromStoring = true;
}

void nextReportStatus(uint8_t test)
{
  //LEDGRUEN_ON;
	sendStatusReport = true;
	statusReport+=1;
	if( statusReport > LASTREPORT )
        statusReport = FIRSTREPORT;
}



