#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint16_t adc_buf[64];
volatile uint16_t adc_sample = 0;
volatile uint16_t dma_sample = 0;
volatile uint32_t dma_isr_count = 0;

//
// --- ADC INIT --------------------------------------------------------------
//
void adc_init(void)
{
    ADCA.CTRLB = ADC_RESOLUTION_12BIT_gc;
    ADCA.CTRLA = ADC_ENABLE_bm;

    ADCA.REFCTRL   = ADC_REFSEL_INTVCC_gc;
    ADCA.PRESCALER = ADC_PRESCALER_DIV64_gc;

    ADCA.CH0.CTRL    = ADC_CH_INPUTMODE_SINGLEENDED_gc;
    ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN5_gc;
}

//
// --- TIMER INIT (1 kHz) ----------------------------------------------------
//
void timer_init(void)
{
    // 32 MHz / 64 = 500 kHz
    TCC0.CTRLA = TC_CLKSEL_DIV64_gc;
    TCC0.PER   = 499;     // 1 kHz Overflow

    TCC0.INTCTRLA = TC_OVFINTLVL_LO_gc;
}

//
// --- DMA INIT --------------------------------------------------------------
//
void dma_init(void)
{
    DMA.CTRL = DMA_ENABLE_bm | DMA_PRIMODE_RR0123_gc;

    // Trigger: Timer Overflow (Event Channel 0)
    EVSYS.CH0MUX = EVSYS_CHMUX_TCC0_OVF_gc;
    DMA.CH0.TRIGSRC = DMA_CH_TRIGSRC_EVSYS_CH0_gc;

    // Source = ADC result register (16-bit)
    uint32_t src = (uint32_t)&ADCA.CH0RES;
    DMA.CH0.SRCADDR0 = (uint8_t)(src & 0xFF);
    DMA.CH0.SRCADDR1 = (uint8_t)((src >> 8) & 0xFF);
    DMA.CH0.SRCADDR2 = (uint8_t)((src >> 16) & 0xFF);

    // Destination = adc_buf
    uint32_t dst = (uint32_t)&adc_buf[0];
    DMA.CH0.DESTADDR0 = (uint8_t)(dst & 0xFF);
    DMA.CH0.DESTADDR1 = (uint8_t)((dst >> 8) & 0xFF);
    DMA.CH0.DESTADDR2 = (uint8_t)((dst >> 16) & 0xFF);

    // 64 Samples × 2 Byte
    DMA.CH0.TRFCNT = 128;

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

//
// --- TIMER ISR: Start ADC --------------------------------------------------
//
ISR(TCC0_OVF_vect)
{
    ADCA.CH0.CTRL |= ADC_CH_START_bm;
}

//
// --- DMA ISR ---------------------------------------------------------------
//
ISR(DMA_CH0_vect)
{
    dma_isr_count++;

    adc_sample = ADCA.CH0RES;
    dma_sample = adc_buf[0];

    DMA.INTFLAGS = DMA_CH0TRNIF_bm;
}

//
// --- MAIN ------------------------------------------------------------------
//
int main(void)
{
    adc_init();
    timer_init();
    dma_init();

    PMIC.CTRL |= PMIC_LOLVLEN_bm;
    sei();

    while (1)
    {
        // Hier kannst du adc_sample, dma_sample, dma_isr_count ausgeben
        // oder auf ein Display schicken
    }
}
