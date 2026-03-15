#include <avr/io.h>
#include <avr/interrupt.h>
#include "wasserzaehler.h"

volatile uint16_t adc_buf_A[64];
volatile uint16_t adc_buf_B[64];
volatile uint16_t avg_value = 0;

volatile uint16_t adc_sample = 0;
volatile uint16_t dma_sample = 0;

volatile uint8_t active_buffer = 0;

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
    ADCA.EVCTRL = ADC_EVSEL_0123_gc | ADC_EVACT_CH0_gc;
}

void timer_init(void)
{
        // --- TCC0 -> 1 kHz ---
    TCC0.CTRLA = TC_CLKSEL_DIV64_gc;     // Prescaler 64
    TCC0.PER   = 499;                    // 1 kHz
}

void evsys_init(void)
{
    EVSYS.CH0MUX = EVSYS_CHMUX_TCC0_OVF_gc;
}

void dma_init(void)
{
    DMA.CH0.TRIGSRC = DMA_CH_TRIGSRC_ADCA_CH0_gc;

    uint32_t src = (uint32_t)&ADCA.CH0RES;

    DMA.CH0.SRCADDR0 = (uint8_t)(src & 0xFF);
    DMA.CH0.SRCADDR1 = (uint8_t)((src >> 8) & 0xFF);
    DMA.CH0.SRCADDR2 = (uint8_t)((src >> 16) & 0xFF);

    uint32_t dst = (uint32_t)&adc_buf_A[0];

    DMA.CH0.DESTADDR0 = (uint8_t)(dst & 0xFF);
    DMA.CH0.DESTADDR1 = (uint8_t)((dst >> 8) & 0xFF);
    DMA.CH0.DESTADDR2 = (uint8_t)((dst >> 16) & 0xFF);

    DMA.CH0.TRFCNT = 128;   // 64 Samples × 2 Byte

    DMA.CH0.ADDRCTRL =
        DMA_CH_SRCRELOAD_BURST_gc |
        DMA_CH_SRCDIR_INC_gc    |
        DMA_CH_DESTRELOAD_BLOCK_gc|
        DMA_CH_DESTDIR_INC_gc;

    DMA.CH0.CTRLB = DMA_CH_TRNINTLVL_LO_gc;

    DMA.CH0.CTRLA =
        DMA_CH_ENABLE_bm |
        DMA_CH_REPEAT_bm |
        DMA_CH_SINGLE_bm |
        DMA_CH_BURSTLEN_2BYTE_gc;
    DMA.CTRL = DMA_ENABLE_bm | DMA_PRIMODE_RR0123_gc;
}

ISR(DMA_CH0_vect)
{
    uint32_t sum = 0;

    if(active_buffer==0)
    {
      LEDBLAU_TOGGLE;
      active_buffer = 1;
      uint32_t dst = (uint32_t)&adc_buf_B[0];

      DMA.CH0.DESTADDR0 = (uint8_t)(dst & 0xFF);
      DMA.CH0.DESTADDR1 = (uint8_t)((dst >> 8) & 0xFF);
      DMA.CH0.DESTADDR2 = (uint8_t)((dst >> 16) & 0xFF);
      DMA.CH0.CTRLB |= DMA_CH_TRNIF_bm;
      for (uint8_t i = 0; i < 64; i++)
          sum += (uint32_t) adc_buf_A[i];
    }
    else
    {
      LEDBLAU_TOGGLE;
      active_buffer = 0;
      uint32_t dst = (uint32_t)&adc_buf_A[0];

      DMA.CH0.DESTADDR0 = (uint8_t)(dst & 0xFF);
      DMA.CH0.DESTADDR1 = (uint8_t)((dst >> 8) & 0xFF);
      DMA.CH0.DESTADDR2 = (uint8_t)((dst >> 16) & 0xFF);
      DMA.CH0.CTRLB |= DMA_CH_TRNIF_bm;
      for (uint8_t i = 0; i < 64; i++)
          sum += (uint32_t) adc_buf_B[i];
    }

    avg_value = (uint16_t) (sum >> 6);
}

