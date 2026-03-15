#ifndef INTERPOLATOR_H_INCLUDED
#define INTERPOLATOR_H_INCLUDED

#include "wasserzaehler.h"

#define OFFSET_SHIFT    10   // größer = langsamer
#define AMP_DECAY       6    // größer = langsamer

#define AMP_MIN         64   // Unterdrückung bei Stillstand

#define LUT_SIZE        256

#define LUT_SIZE 256

#define PHASE_BINS      32
#define HYSTERESIS      20
#define SIN_SCALE       2048

typedef struct
{
    int32_t sum;
    uint16_t count;

} phase_bin_t;


typedef struct
{
    phase_bin_t bin[PHASE_BINS];

    int16_t amplitude;
    int16_t offset;

    uint8_t current_bin;
    uint8_t halfwave;

    uint8_t phase16;

    uint32_t period_counter;

} sin_tracker_t;

int testMain(void);
void sintracker_prefill(sin_tracker_t *s);
void sintracker_init(
        sin_tracker_t *s,
        int16_t offset,
        int16_t amplitude);
void sintracker_recalculate(sin_tracker_t *s);
uint8_t sintracker_estimate_bin(
        sin_tracker_t *s,
        int16_t sample);
void sintracker_process(
        sin_tracker_t *s,
        int16_t adc);


#endif // INTERPOLATOR_H_INCLUDED
