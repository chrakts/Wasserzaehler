#include <avr/io.h>
#include <avr/interrupt.h>
#include "wasserzaehler.h"

volatile uint16_t adc_buf[64];
volatile uint16_t avg_value = 0;

volatile uint16_t adc_sample = 0;
volatile uint16_t dma_sample = 0;


void port_init(void)
{
    PORTA.DIR = 0xDF;   // 1101 1111  -> PA5 = 0 (Input), Rest = Output
    PORTA.PIN5CTRL = PORT_ISC_INPUT_DISABLE_gc | PORT_OPC_TOTEM_gc;  // Digital input buffer off
}


void adc_init(void)
{
    ADCA.CTRLB = ADC_RESOLUTION_12BIT_gc;
    ADCA.CTRLA = ADC_ENABLE_bm;

    ADCA.REFCTRL   = ADC_REFSEL_INTVCC_gc;
    ADCA.PRESCALER = ADC_PRESCALER_DIV64_gc;

    ADCA.CH0.CTRL    = ADC_CH_INPUTMODE_SINGLEENDED_gc;
    ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN5_gc;
}

void timer_init(void)
{
    // 32 MHz / 64 = 500 kHz
    TCC0.CTRLA = TC_CLKSEL_DIV64_gc;
    TCC0.PER   = 499;     // 1 kHz Overflow

    TCC0.CCA   = 20;      // Compare Match ~40 µs nach Overflow

    TCC0.INTCTRLA = TC_OVFINTLVL_LO_gc;   // Overflow ISR
}

void evsys_init(void)
{
    // Event Channel 0 = TCC0 Compare Match A
    EVSYS.CH0MUX = EVSYS_CHMUX_TCC0_CCA_gc;
}

void dma_init(void)
{
    DMA.CTRL = DMA_ENABLE_bm | DMA_PRIMODE_RR0123_gc;

    DMA.CH0.TRIGSRC = DMA_CH_TRIGSRC_EVSYS_CH0_gc;

    uint32_t src = (uint32_t)&ADCA.CH0RES;

    DMA.CH0.SRCADDR0 = (uint8_t)(src & 0xFF);
    DMA.CH0.SRCADDR1 = (uint8_t)((src >> 8) & 0xFF);
    DMA.CH0.SRCADDR2 = (uint8_t)((src >> 16) & 0xFF);

    uint32_t dst = (uint32_t)&adc_buf[0];

    DMA.CH0.DESTADDR0 = (uint8_t)(dst & 0xFF);
    DMA.CH0.DESTADDR1 = (uint8_t)((dst >> 8) & 0xFF);
    DMA.CH0.DESTADDR2 = (uint8_t)((dst >> 16) & 0xFF);

    DMA.CH0.TRFCNT = 128;   // 64 Samples × 2 Byte

    DMA.CH0.ADDRCTRL =
        DMA_CH_SRCRELOAD_BURST_gc |
        DMA_CH_SRCDIR_FIXED_gc    |
        DMA_CH_DESTRELOAD_BLOCK_gc|
        DMA_CH_DESTDIR_INC_gc;

    DMA.CH0.CTRLB = DMA_CH_TRNINTLVL_LO_gc;

    DMA.CH0.CTRLA =
        DMA_CH_ENABLE_bm |
        DMA_CH_REPEAT_bm |
        DMA_CH_BURSTLEN_2BYTE_gc;
}


/*
void dma_init(void)
{
    DMA.CTRL = DMA_ENABLE_bm | DMA_PRIMODE_RR0123_gc;

    // Trigger: ADC Channel 0 Conversion Complete
    DMA.CH0.TRIGSRC = DMA_CH_TRIGSRC_ADCA_CH0_gc;

    // Quelle = 16-Bit-Resultatregister
    uint16_t src = (uint16_t)&ADCA.CH0RES;
    DMA.CH0.SRCADDR0 = src & 0xFF;
    DMA.CH0.SRCADDR1 = src >> 8;
    DMA.CH0.SRCADDR2 = 0;

    // Ziel = adc_buf[0]
    uint16_t dst = (uint16_t)&adc_buf_A[0];
    DMA.CH0.DESTADDR0 = dst & 0xFF;
    DMA.CH0.DESTADDR1 = dst >> 8;
    DMA.CH0.DESTADDR2 = 0;

    // 64 Samples × 2 Byte = 128 Byte
    DMA.CH0.TRFCNT = 128;

    DMA.CH0.ADDRCTRL =
        DMA_CH_SRCRELOAD_BURST_gc |     // Quelle nach Burst zurück
        DMA_CH_SRCDIR_FIXED_gc    |     // Quelle fest
        DMA_CH_DESTRELOAD_BLOCK_gc|     // Ziel nach Block zurück
        DMA_CH_DESTDIR_INC_gc;          // Ziel inkrementieren

    DMA.CH0.CTRLB = DMA_CH_TRNINTLVL_LO_gc;

    DMA.CH0.CTRLA =
        DMA_CH_ENABLE_bm |
        DMA_CH_REPEAT_bm |
        DMA_CH_BURSTLEN_2BYTE_gc;       // 2-Byte-Burst, kein SINGLE
}
*/


ISR(TCC0_OVF_vect)
{
    ADCA.CH0.CTRL |= ADC_CH_START_bm;
}


ISR(DMA_CH0_vect)
{
    uint32_t sum = 0;

    for (uint8_t i = 0; i < 64; i++)
        sum += adc_buf[i];

    avg_value = sum / 64;

    adc_sample = ADCA.CH0RES;
    dma_sample = adc_buf[0];
    avg_value = dma_sample;
    DMA.INTFLAGS = DMA_CH0TRNIF_bm;
    LEDBLAU_TOGGLE;
}


/*
ISR(DMA_CH0_vect)
{
    uint32_t sum = 0;

    for (uint8_t i = 0; i < 64; i++)
        sum += adc_buf_A[i];

    // zur Sicherheit auf 12 Bit maskieren
    avg_value = (sum / 64) & 0x0FFF;

    //avg_value = (adc_buf_A[0]>> 8 ) | (uint16_t)(adc_buf_A[0]& 0xff)<<8;
    avg_value = adc_buf_A[0];
    //avg_value = ADCA.CH0RES;


    LEDBLAU_TOGGLE;
    DMA.INTFLAGS = DMA_CH0TRNIF_bm;
}
*/
/*
ISR(DMA_CH0_vect)
{
    uint32_t sum = 0;
    uint16_t sample;


    for (uint8_t i = 0; i < 64; i++)
    {
        sample = ((uint16_t)adc_buf_A[i*2+1]<< 8 ) | (adc_buf_A[i*2]);
        sum += sample;
    }
    LEDBLAU_TOGGLE;
    avg_value = sum / 64;


    DMA.INTFLAGS = DMA_CH0TRNIF_bm;
}*/

/*
ISR(DMA_CH0_vect)
{
    uint32_t sum = 0;
    uint16_t sample;

    if (active_buffer == 0)
    {
        for (uint8_t i = 0; i < 64; i++)
        {
            sample = ((uint16_t)adc_buf_A[i*2+1]<< 8 ) | (adc_buf_A[i*2]);
            sum += sample;
        }

        avg_value = sum / 64;

        // Nächster Buffer = B
        uint16_t dst = (uint16_t)adc_buf_B;
        DMA.CH0.DESTADDR0 = dst & 0xFF;
        DMA.CH0.DESTADDR1 = dst >> 8;

        active_buffer = 1;


        LEDBLAU_OFF;
        //avg_value = adc_buf_A[1]; // *************************************************************
    }
    else
    {
        for (uint8_t i = 0; i < 64; i++)
        {
            sample = ((uint16_t)adc_buf_B[i*2+1]<< 8 ) | (adc_buf_B[i*2]);
            sum += sample;
        }

        avg_value = sum / 64;

        // Nächster Buffer = A
        uint16_t dst = (uint16_t)adc_buf_A;
        DMA.CH0.DESTADDR0 = dst & 0xFF;
        DMA.CH0.DESTADDR1 = dst >> 8;

        active_buffer = 0;
        LEDBLAU_ON;
        //avg_value = adc_buf_B[1]; // *************************************************************

    }
    //DMA.CH0.CTRLB |= DMA_CH_TRNIF_bm;  // Interrupt-Flag löschen
    DMA.INTFLAGS = DMA_CH0TRNIF_bm;
}*/
