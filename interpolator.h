#ifndef INTERPOLATOR_H_INCLUDED
#define INTERPOLATOR_H_INCLUDED

#include "wasserzaehler.h"

#include <stdint.h>

#define NUM_BINS 12

/*
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
};*/

// -----------------------------
// SinTracker (µC optimiert)
// -----------------------------
// -----------------------------
// SinTracker (nahe am Python-Code)
// -----------------------------
class SinTracker {
public:
    float offset = 3486.0f;
    float amplitude = 100.0f;
    uint16_t min_sample = 65535;
    uint16_t max_sample = 0;

    uint8_t last_bin = 0;

    float bin_means[12] = {0.0f};
    uint16_t bin_counts[12] = {0};

    uint16_t visited_mask = 0;
    volatile uint32_t cycle_counter = 0;  // Ist der Umdrehungszähler (2 Schritte pro Umdrehung) -> 5 l/Schritt
    volatile uint64_t actual_phase = 0;   // 1/12 Phasenschritt -> 0,42 l / Phasenschritt
    bool up_direction = true;             // true: zählt aufwärts, false: zählt abwärts

    bool cycle_detected = false;

    // -------------------------
    uint8_t process_sample(uint16_t sample)
    {
        if(sample<min_sample)
          min_sample = sample;
        if(sample>max_sample)
          max_sample = sample;
        // 1. Offset entfernen
        float delta = (float)sample - offset;

        // 2. Normieren
        float norm = delta / amplitude;

        if (norm > 1.0f) norm = 1.0f;
        if (norm < -1.0f) norm = -1.0f;

        // 3. Phase bestimmen (0..1023)
        float phase_f = (norm + 1.0f) * 511.5f;
        uint16_t phase = (uint16_t)phase_f;

        if (phase > 1023) phase = 1023;

        // 4. Bin bestimmen (0..11)
        uint8_t bin = (phase * 12) / 1024;

        // 5. Inkrementeller Mittelwert (wie Python)
        /*
        bin_counts[bin]++;
        float count = (float)bin_counts[bin];
        bin_means[bin] += ((float)sample - bin_means[bin]) / count;
        */

        // 6. Besuch markieren
        visited_mask |= (1 << bin);

        // 7. Zyklus erkannt?
        if (visited_mask == 0x0FFF)
        {
            if (last_bin > 6)       // stellt fest, ob die Phase gerade in positiver oder negativer Richtung durchlaufen werden
              up_direction = false;
            else
              up_direction = true;

            cycle_detected = true;
            cycle_counter++;
            // neuer Offset
            /*
            float sum = 0.0f;
            for (uint8_t i = 0; i < 12; i++)
            {
                sum += bin_means[i];
            }
            offset = sum / 12.0f;*/
            offset = (min_sample>>1)+(max_sample>>1);

            // neue Amplitude
            /*
            float max_dev = 0.0f;
            for (uint8_t i = 0; i < 12; i++)
            {
                float dev = fabsf(bin_means[i] - offset);
                if (dev > max_dev)
                    max_dev = dev;
            }
            amplitude = max_dev;
            */
            amplitude = max_sample-min_sample;
            // Reset
            /*
            for (uint8_t i = 0; i < 12; i++)
            {
                bin_means[i] = 0.0f;
                bin_counts[i] = 0;
            }
            */
            min_sample = 65535;
            max_sample = 0;
            visited_mask = 0;
        }

        last_bin = bin;
        if(up_direction==true)
          actual_phase = bin;
        else
          actual_phase = 11-bin;
        return bin;
    }
};

int testMain(void);

#endif // INTERPOLATOR_H_INCLUDED
