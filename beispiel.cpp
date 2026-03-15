/*
In diesem Beispiel wird mit den Peripherien DMA, Event-System und ADC des
Mikrocontrollers ATxmega32A4 gearbeitet.
Als Beispiel werden 3 Temperatursensoren verwendet, die an den Pins 1, 2 und 3
des Ports A angeschlossen sind.
Mit Hilfe des DMA wird die Information vom ADC entsprechend der im Timer
konfigurierten Zeit empfangen und direkt im Speicher abgelegt,
ohne die Hauptverarbeitung zu beeinflussen.
*/

#define F_CPU 32000000UL  // CPU-Frequenz auf 32 MHz definieren

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdio.h>

// Funktionsdeklarationen
void ADC_Inicializar();    // Initialisiert den Analog-Digital-Wandler
void Lectura_ADC();        // Verarbeitet die ADC-Messwerte

void DMA_Inicializar();    // Initialisiert den DMA-Controller
void ADC_DMA0_Temp1();     // Konfiguriert DMA für Temperatursensor 1
void ADC_DMA1_Temp2();     // Konfiguriert DMA für Temperatursensor 2
void ADC_DMA2_Temp3();     // Konfiguriert DMA für Temperatursensor 3

void Clock_Inicializar();  // Konfiguriert den Systemtakt

void Event_System();       // Konfiguriert das Event-System
void Timer_Init();         // Initialisiert den Timer

// Struktur zum Speichern der Messwerte der 3 Sensoren
struct lectira_struct{
    uint16_t Temp1[10];  // Puffer für 10 Messwerte von Sensor 1
    uint16_t Temp2[10];  // Puffer für 10 Messwerte von Sensor 2
    uint16_t Temp3[10];  // Puffer für 10 Messwerte von Sensor 3
}Lecturas;

// Variablen zur Speicherung der gemittelten Temperaturen
uint16_t T1,T2,T3;

// Flags, die anzeigen, wann jeder DMA seine Übertragung beendet hat
bool Bandera_DMA0 = false;
bool Bandera_DMA1 = false;
bool Bandera_DMA2 = false;

// Interrupt-Service-Routine für DMA Kanal 0
ISR(DMA_CH0_vect){
    // Prüft, ob das Transfer-Flag gesetzt ist
    if(DMA.CH0.CTRLB & DMA_CH_TRNIF_bm){
         DMA.CH0.CTRLB |= DMA_CH_TRNIF_bm; // Interrupt-Flag löschen
         Bandera_DMA0 = true; // Signalisiert, dass die DMA0-Übertragung abgeschlossen ist
    }
}

// Interrupt-Service-Routine für DMA Kanal 1
ISR(DMA_CH1_vect){
    if(DMA.CH1.CTRLB & DMA_CH_TRNIF_bm){
         DMA.CH1.CTRLB |= DMA_CH_TRNIF_bm; // Interrupt-Flag löschen
         Bandera_DMA1 = true; // Signalisiert, dass die DMA1-Übertragung abgeschlossen ist
    }
}

// Interrupt-Service-Routine für DMA Kanal 2
ISR(DMA_CH2_vect){
    if(DMA.CH2.CTRLB & DMA_CH_TRNIF_bm){
         DMA.CH2.CTRLB |= DMA_CH_TRNIF_bm; // Interrupt-Flag löschen
         Bandera_DMA2 = true; // Signalisiert, dass die DMA2-Übertragung abgeschlossen ist
    }
}

// Hauptfunktion
int main(void){
   cli(); // Globale Interrupts während der Initialisierung deaktivieren
   
   // Initialisierung aller Peripheriemodule
   Clock_Inicializar(); // Systemtakt konfigurieren
   ADC_Inicializar();   // ADC konfigurieren
   Event_System();      // Event-System konfigurieren
   Timer_Init();        // Timer initialisieren
   DMA_Inicializar();   // DMA konfigurieren
   
   // Interrupts auf allen Prioritätsstufen aktivieren
   PMIC.CTRL |= PMIC_HILVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_LOLVLEN_bm;
   PORTD.DIRSET |= PIN0_bm; // PIN0 von Port D als Ausgang konfigurieren
   
   sei(); // Globale Interrupts aktivieren

   // Hauptprogrammschleife
   while (1){
       // Wenn alle DMA-Übertragungen abgeschlossen sind
       if(Bandera_DMA0 && Bandera_DMA1 && Bandera_DMA2){
           Lectura_ADC(); // ADC-Werte verarbeiten
           
           // HINWEIS: Hier ist ein Fehler - die Flags sollten wieder auf false gesetzt werden
           Bandera_DMA0 = false;
           Bandera_DMA1 = false; 
           Bandera_DMA2 = false;
       }
   }
}

// Funktion zur Konfiguration des Systemtakts auf 32 MHz
void Clock_Inicializar(){
    OSC_CTRL |= OSC_RC32MEN_bm; // Internen 32-MHz-Oszillator aktivieren
    while(!(OSC_STATUS & 0x02)); // Warten bis der Oszillator stabil ist
    CCP = CCP_IOREG_gc; // Schreiben in geschützte Register erlauben
    CLK_CTRL = CLK_SCLKSEL_RC32M_gc; // 32-MHz-Oszillator als Haupttakt wählen
}

// Funktion zum Verarbeiten der ADC-Messwerte und Berechnen der Mittelwerte
void Lectura_ADC(){
    uint16_t lectura0 = 0; // Akkumulator für Sensor 1
    uint16_t lectura1 = 0; // Akkumulator für Sensor 2
    uint16_t lectura2 = 0; // Akkumulator für Sensor 3

    // Summiert die 10 Messwerte jedes Sensors
    for(uint8_t i = 0; i<10; i++){
       lectura0 += Lecturas.Temp1[i];
       lectura1 += Lecturas.Temp2[i];
       lectura2 += Lecturas.Temp3[i];
    }
    
    // Berechnet den Mittelwert jedes Sensors
    T1 = lectura0 / 10;
    T2 = lectura1 / 10;
    T3 = lectura2 / 10;
}

// Funktion zur Initialisierung des ADC
void ADC_Inicializar(){
    ADCA.CTRLA = ADC_ENABLE_bm; // ADC aktivieren
    ADCA.CTRLB = ADC_RESOLUTION_12BIT_gc | ADC_CONMODE_bm | ADC_SWEEP_01_gc; // 12-Bit-Auflösung, kontinuierlicher Modus und Kanal-Sweep
    ADCA.PRESCALER = ADC_PRESCALER_DIV16_gc; // ADC-Taktteiler
    ADCA.REFCTRL = ADC_REFSEL_INT1V_gc; // Interne Referenzspannung von 1 V
    
    // Konfiguration Kanal 0 (Sensor 1)
    ADCA.CH0.CTRL = ADC_CH_INPUTMODE_SINGLEENDED_gc; // Single-Ended-Modus
    ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN0_gc; // Pin PA0 auswählen
    
    // Konfiguration Kanal 1 (Sensor 2)
    ADCA.CH1.CTRL = ADC_CH_INPUTMODE_SINGLEENDED_gc; // Single-Ended-Modus
    ADCA.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN1_gc; // Pin PA1 auswählen
    
    // Konfiguration Kanal 2 (Sensor 3)
    ADCA.CH2.CTRL = ADC_CH_INPUTMODE_SINGLEENDED_gc; // Single-Ended-Modus
    ADCA.CH2.MUXCTRL = ADC_CH_MUXPOS_PIN2_gc; // Pin PA2 auswählen
    
    // ADC-Ereignisse konfigurieren (Kanäle 5,6,7 des Event-Systems)
    ADCA.EVCTRL = ADC_EVSEL_567_gc | ADC_EVACT_CH0123_gc;
}

// Funktion zur Initialisierung des Timers
void Timer_Init(){
    TCC1.CTRLB = TC_WGMODE_NORMAL_gc; // Normaler Betriebsmodus
    TCC1.CTRLA = TC_CLKSEL_DIV64_gc; // Taktteiler 64
    TCC1.PER = 3125; // Timerperiode (~6.25 ms bei 32 MHz)
}

// Funktion zur Konfiguration des Event-Systems
void Event_System(){
    // Kanäle 5,6,7 werden durch Overflow des Timers TCC1 ausgelöst
    EVSYS.CH7MUX = EVSYS_CHMUX_TCC1_OVF_gc;
    EVSYS.CH6MUX = EVSYS_CHMUX_TCC1_OVF_gc;
    EVSYS.CH5MUX = EVSYS_CHMUX_TCC1_OVF_gc;
    
    //EVSYS.CH1MUX = EVSYS_CHMUX_ADCA_CH0_gc; // Kanal 1 durch ADC Kanal 0 ausgelöst
    //EVSYS.CH2MUX = EVSYS_CHMUX_ADCA_CH1_gc; // Kanal 2 durch ADC Kanal 1 ausgelöst
    //EVSYS.CH3MUX = EVSYS_CHMUX_ADCA_CH2_gc; // Kanal 3 durch ADC Kanal 2 ausgelöst
}

// Funktion zur Initialisierung des DMA-Controllers
void DMA_Inicializar(){
    DMA.CTRL = DMA_ENABLE_bm; // DMA-Controller aktivieren
    ADC_DMA0_Temp1(); // DMA für Sensor 1 konfigurieren
    ADC_DMA1_Temp2(); // DMA für Sensor 2 konfigurieren
    ADC_DMA2_Temp3(); // DMA für Sensor 3 konfigurieren
}

// DMA-Konfiguration für Temperatursensor 1 (Kanal 0)
void ADC_DMA0_Temp1(){

    // Interrupt bei abgeschlossener Übertragung (mittlere Priorität)
    DMA.CH0.CTRLB = DMA_CH_TRNIF_bm | DMA_CH_TRNINTLVL_MED_gc;
    
    // DMA aktivieren, 2-Byte Transfer (ADC 12 Bit), Wiederholmodus
    DMA.CH0.CTRLA = DMA_CH_ENABLE_bm | DMA_CH_BURSTLEN_2BYTE_gc | DMA_CH_REPEAT_bm;
        
    // Anzahl der Transfers (20 Bytes = 10 Samples × 2 Bytes)
    DMA.CH0.TRFCNT = 20;
    
    // Adresssteuerung
    DMA.CH0.ADDRCTRL = DMA_CH_SRCRELOAD_BURST_gc | DMA_CH_SRCDIR_INC_gc |
                        DMA_CH_DESTRELOAD_TRANSACTION_gc | DMA_CH_DESTDIR_INC_gc;
                        
    // Triggerquelle: ADC Kanal 0 Ergebnis
    DMA.CH0.TRIGSRC = DMA_CH_TRIGSRC_ADCA_CH0_gc;

    // Quelladresse (ADC Ergebnisregister Kanal 0)
    DMA.CH0.SRCADDR0 = ((uintptr_t)&ADCA.CH0.RES) & 0xFF;
    DMA.CH0.SRCADDR1 = (((uintptr_t)&ADCA.CH0.RES) >> 0x08) & 0xFF;
    DMA.CH0.SRCADDR2 = (((uintptr_t)&ADCA.CH0.RES) >> 0x0F) & 0xFF;

    // Zieladresse (Temp1-Puffer)
    DMA.CH0.DESTADDR0 = ((uintptr_t)&Lecturas.Temp1) & 0xFF;
    DMA.CH0.DESTADDR1 = (((uintptr_t)&Lecturas.Temp1) >> 0x08) & 0xFF;
    DMA.CH0.DESTADDR2 = (((uintptr_t)&Lecturas.Temp1) >> 0x0F) & 0xFF;
}
