#ifndef INTERPOLATOR_H_INCLUDED
#define INTERPOLATOR_H_INCLUDED

#include "wasserzaehler.h"

#include <stdint.h>

#define NUM_BINS 12
#define TABLE_SIZE 256  // Größe der Sinus-Lookup-Tabelle

// Lookup-Tabelle für Sinuswerte (0..255 -> -127..127)
const int8_t sin_table[TABLE_SIZE] = {
    0, 3, 6, 9, 12, 16, 19, 22, 25, 28, 31, 34, 37, 40, 43, 46,
    49, 52, 55, 58, 61, 64, 67, 70, 73, 76, 79, 81, 84, 87, 90, 93,
    95, 98, 101, 104, 106, 109, 111, 114, 117, 119, 122, 124, 127,
    127, 124, 122, 119, 117, 114, 111, 109, 106, 104, 101, 98, 95,
    93, 90, 87, 84, 81, 79, 76, 73, 70, 67, 64, 61, 58, 55, 52, 49,
    46, 43, 40, 37, 34, 31, 28, 25, 22, 19, 16, 12, 9, 6, 3, 0,
    -3, -6, -9, -12, -16, -19, -22, -25, -28, -31, -34, -37, -40,
    -43, -46, -49, -52, -55, -58, -61, -64, -67, -70, -73, -76,
    -79, -81, -84, -87, -90, -93, -95, -98, -101, -104, -106, -109,
    -111, -114, -117, -119, -122, -124, -127, -127, -124, -122, -119,
    -117, -114, -111, -109, -106, -104, -101, -98, -95, -93, -90, -87,
    -84, -81, -79, -76, -73, -70, -67, -64, -61, -58, -55, -52, -49,
    -46, -43, -40, -37, -34, -31, -28, -25, -22, -19, -16, -12, -9, -6,
    -3
};
/*
// --- Tracker-Strukturen
typedef struct {
    int32_t sum;
    uint16_t count;
} BinState;

typedef struct {
    BinState bins[NUM_BINS];
    int16_t offset;
    int16_t amplitude;
    uint8_t last_bin;
    bool cycle_started;
} SinTracker;

*/
int testMain(void);
//void sintracker_init(SinTracker *st);
//bool sintracker_process(SinTracker *st, int16_t sample);


#endif // INTERPOLATOR_H_INCLUDED
