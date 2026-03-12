/*
* wasserzaehler.cpp
*/

#include "wasserzaehler.h"

void setup()
{
  init_clock(SYSCLK,PLL,true,CLOCK_CALIBRATION);
	PORTA_DIRSET = PIN2_bm | PIN3_bm | PIN4_bm;
	PORTA_OUTSET = 0xff;
	PORTB_DIRSET = 0xff;
	PORTC_DIRSET = PIN1_bm;
	PORTD_DIRSET = 0xff;
	PORTE_DIRSET = PIN0_bm | PIN1_bm | PIN3_bm;

	initZaehler();

  eeprom_value_t temp;
  eeprom_logger_init(&temp);
  wasserstand = temp;

  PMIC_CTRL = PMIC_LOLVLEX_bm | PMIC_HILVLEN_bm | PMIC_MEDLVLEN_bm;
	sei();
	cnet.open(Serial::BAUD_57600,F_CPU);
}

int main(void)
{
	setup();
	cnet.broadcastUInt8((uint8_t) RST.STATUS,'S','0','R');
	init_mytimer();



	while (1)
	{
		cnetRec.comStateMachine();
		cnetRec.doJob();
    if(doEEpromStoring)
    {
      doEEpromStoring = false;
      uint32_t temp = eeprom_logger_store_if_changed_safe(&wasserstand);
      cnet.broadcastUInt32(temp,'W','i','i');
    }

		if( sendStatusReport )
    {
        sendStatusReport = false;
        MyTimers[TIMER_REPORT].value = actReportBetweenSensors;
        MyTimers[TIMER_REPORT].state = TM_START;
        switch(statusReport)
        {
          case FIRSTREPORT:
            cnet.broadcastUInt32((uint32_t) wasserstand,'W','0','a');
          break;

          case LASTREPORT:
              MyTimers[TIMER_REPORT].value = actReportBetweenBlocks;
              MyTimers[TIMER_REPORT].state = TM_START;
          break;
      }
      LEDGRUEN_OFF;
    }
	}
}

void initZaehler()
{
  PORTC_DIRCLR = PIN2_bm;               // PC2 Eingang
  PORTC_PIN2CTRL = PORT_OPC_PULLUP_gc;    // Pull-Up aktivieren

  PORTC_INT0MASK = PIN2_bm;                     // Pin2 Interrupt aktiv
  PORTC_INTCTRL = PORT_INT0LVL_LO_gc | PORT_ISC_FALLING_gc; // niedrige Priorität + falling edge
  PORTC_INTFLAGS = PIN2_bm;                     // Flag löschen
}

ISR(PORTC_INT0_vect)
{
    PORTC_INT0MASK &= ~PIN2_bm; // sofort Interrupt für PC2 ausschalten
    wasserstand += 1;
    MyTimers[TIMER_ENTPRELLEN].state = TM_START;

}

