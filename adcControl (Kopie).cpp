#include <avr/io.h>
#include <avr/interrupt.h>
#include "wasserzaehler.h"
#define SAMPLES 64

volatile uint16_t adc_buffer[SAMPLES];
volatile uint16_t bufferB[SAMPLES];

volatile uint8_t active_buffer = 0;

volatile uint32_t buffer_mean = 0;  // 32-bit, kein Overflow

volatile uint8_t buffer_ready = 0;
volatile uint16_t dma_value = 0;   // Buffer für 1 Sample


void timer_init(void)
{
    TCC0.PER = 32000 - 1;
    TCC0.CTRLA = TC_CLKSEL_DIV1_gc;
}

void event_init(void)
{
    EVSYS.CH0MUX = EVSYS_CHMUX_TCC0_OVF_gc;
}

void adc_init(void)
{
    // PA5 als Eingang / analog
    PORTA.DIRCLR = PIN5_bm;
    PORTA.PIN5CTRL = PORT_ISC_INPUT_DISABLE_gc;

    ADCA.REFCTRL = ADC_REFSEL_INTVCC2_gc; // interne VCC/2
    ADCA.CTRLB = ADC_RESOLUTION_12BIT_gc;
    ADCA.PRESCALER = ADC_PRESCALER_DIV64_gc;

    ADCA.CH0.CTRL = ADC_CH_INPUTMODE_SINGLEENDED_gc;
    ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN5_gc;

    // Event-Trigger: Channel0
    ADCA.EVCTRL = ADC_EVACT_CH0_gc | ADC_EVSEL_0123_gc;

    ADCA.CTRLA = ADC_ENABLE_bm;

    // Dummy conversion, stabilisiert ADC
    ADCA.CH0.CTRL |= ADC_CH_START_bm;
    while(!(ADCA.CH0.INTFLAGS & ADC_CH_CHIF_bm));
    ADCA.CH0.INTFLAGS = ADC_CH_CHIF_bm;
}

void dma_init(void)
{
    DMA.CTRL = DMA_ENABLE_bm;

    uint32_t src = (uint32_t)&ADCA.CH0RES;
    uint32_t dst = (uint32_t)adc_buffer;

    DMA.CH0.SRCADDR0 = src & 0xFF;
    DMA.CH0.SRCADDR1 = (src >> 8) & 0xFF;
    DMA.CH0.SRCADDR2 = (src >> 16) & 0xFF;

    DMA.CH0.DESTADDR0 = dst & 0xFF;
    DMA.CH0.DESTADDR1 = (dst >> 8) & 0xFF;
    DMA.CH0.DESTADDR2 = (dst >> 16) & 0xFF;

    DMA.CH0.TRFCNT = SAMPLES * 2;

    DMA.CH0.CTRLB = DMA_CH_TRNINTLVL_MED_gc; // Interrupt-Level vor ENABLE

    DMA.CH0.ADDRCTRL =
        DMA_CH_SRCRELOAD_NONE_gc |
        DMA_CH_SRCDIR_FIXED_gc |
        DMA_CH_DESTRELOAD_BLOCK_gc |
        DMA_CH_DESTDIR_INC_gc;

    DMA.CH0.TRIGSRC = DMA_CH_TRIGSRC_ADCA_CH0_gc;

    DMA.CH0.CTRLA =
        DMA_CH_BURSTLEN_2BYTE_gc |
        DMA_CH_REPEAT_bm |
        DMA_CH_ENABLE_bm;


}

ISR(DMA_CH0_vect)
{
    uint32_t sum = 0;

    for(uint8_t i=0;i<SAMPLES;i++)
        sum += adc_buffer[i];

    buffer_mean = sum / SAMPLES;
    buffer_ready = 1;

    DMA.CH0.TRFCNT = SAMPLES * 2;
    DMA.CH0.CTRLA |= DMA_CH_ENABLE_bm;

    DMA.CH0.CTRLB |= DMA_CH_TRNIF_bm;
}

/*

ISR(DMA_CH0_vect)
{
    volatile uint16_t *buf;
    uint32_t sum = 0;

    // Ping-Pong Umschaltung
    if(active_buffer == 0)
    {
        buf = bufferA;
        uint32_t dst = (uint32_t)bufferB;
        DMA.CH0.DESTADDR0 = dst & 0xFF;
        DMA.CH0.DESTADDR1 = (dst >> 8) & 0xFF;
        DMA.CH0.DESTADDR2 = (dst >> 16) & 0xFF;
        active_buffer = 1;
    }
    else
    {
        buf = bufferB;
        uint32_t dst = (uint32_t)bufferA;
        DMA.CH0.DESTADDR0 = dst & 0xFF;
        DMA.CH0.DESTADDR1 = (dst >> 8) & 0xFF;
        DMA.CH0.DESTADDR2 = (dst >> 16) & 0xFF;
        active_buffer = 0;
    }

    // Mittelwert berechnen
    for(uint8_t i = 0; i < SAMPLES; i++)
        sum += buf[i];

    buffer_mean = sum / SAMPLES;
    dma_done_flag = 1;

    // DMA für nächsten Block wieder aktivieren
    DMA.CH0.TRFCNT = SAMPLES * 2;
    DMA.CH0.CTRLA |= DMA_CH_ENABLE_bm;

    // Interrupt-Flag löschen
    DMA.CH0.CTRLB |= DMA_CH_TRNIF_bm;
}

*/


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

volatile uint16_t adc_value = 0;

void adc_test_init(void)
{
    // PA5 als Eingang und analog konfigurieren
    PORTA.DIRCLR = PIN5_bm;                  // Eingang
    PORTA.PIN5CTRL = PORT_ISC_INPUT_DISABLE_gc; // digital input deaktivieren

    // ADC Setup
    ADCA.REFCTRL = ADC_REFSEL_INTVCC_gc;    // interne VCC/1.6 Referenz
    ADCA.CTRLB = ADC_RESOLUTION_12BIT_gc;
    ADCA.PRESCALER = ADC_PRESCALER_DIV64_gc;

    ADCA.CH0.CTRL = ADC_CH_INPUTMODE_SINGLEENDED_gc;
    ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN5_gc;

    ADCA.CTRLA = ADC_ENABLE_bm;
}

uint16_t adc_test_read(void)
{
    // Software-Start ADC
    ADCA.CH0.CTRL |= ADC_CH_START_bm;

    // Warten bis fertig
    while(!(ADCA.CH0.INTFLAGS & ADC_CH_CHIF_bm));

    // Interrupt-Flag löschen
    ADCA.CH0.INTFLAGS = ADC_CH_CHIF_bm;

    // ADC Wert lesen
    return ADCA.CH0RES;
}

