#ifndef EEPROM_LOGGER_H
#define EEPROM_LOGGER_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Typ für gespeicherte Werte
typedef uint32_t eeprom_value_t;

// Initialisiert den Logger, liest letzten Wert
// Rückgabe: 1 = gültiger Wert gefunden, 0 = EEPROM leer
uint8_t eeprom_logger_init(eeprom_value_t *value);

// Wert speichern (rotierend, ausfallsicher)
// value kann volatile sein, z.B. aus ISR
uint32_t eeprom_logger_store(volatile eeprom_value_t *value);

// Zugriff auf letzten gespeicherten Wert
eeprom_value_t eeprom_logger_get_last(void);

// Speichern nur wenn aktueller Wert nicht dem zuletzt gespeicherten Wert entspricht.
uint32_t eeprom_logger_store_if_changed_safe(volatile eeprom_value_t *value);

#ifdef __cplusplus
}
#endif

#endif // EEPROM_LOGGER_H

