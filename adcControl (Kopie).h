#ifndef ADCCONTROL_H_INCLUDED
#define ADCCONTROL_H_INCLUDED

void timer_init(void);

void event_init(void);
void adc_init(void);

void dma_init(void);
ISR(DMA_CH0_vect);

void adc_test_init(void);
void dma_test_init(void);
uint16_t adc_test_read(void);
void test_dma();

void adc_event_init(void);

#endif // ADCCONTROL_H_INCLUDED
