#include "eeprom_logger.h"
#include <avr/eeprom.h>
#include <stddef.h>

// --- Konfiguration ---
#define EEPROM_START_ADDR  0x000
#define EEPROM_BYTES       1000
#define EEPROM_VALID_MARK  0xA5

// --- Datensatz ---
typedef struct {
    uint8_t  valid;      // wird zuletzt geschrieben
    uint8_t  crc;        // CRC8 über seq + value
    uint32_t seq;        // 32-Bit Sequenznummer
    eeprom_value_t value; // gespeicherter Wert
} eeprom_entry_t;

#define ENTRY_SIZE   sizeof(eeprom_entry_t)  // 10 Bytes
#define ENTRY_COUNT  (EEPROM_BYTES / ENTRY_SIZE) // 100 Einträge

// --- statische Variablen ---
static eeprom_entry_t e_last;
static uint32_t seq_last;
static uint16_t index_last;

// --- CRC8 ---
static uint8_t crc8(const uint8_t *data, uint8_t len)
{
    uint8_t crc = 0xFF;
    while (len--) {
        crc ^= *data++;
        for (uint8_t i = 0; i < 8; i++) {
            if (crc & 0x80)
                crc = (crc << 1) ^ 0x31;
            else
                crc <<= 1;
        }
    }
    return crc;
}

// --- Adresse berechnen ---
static inline uint16_t entry_addr(uint16_t index)
{
    return EEPROM_START_ADDR + (index * ENTRY_SIZE);
}

// --- Letzten gültigen Eintrag finden ---
static uint8_t load_latest(eeprom_entry_t *e)
{
    eeprom_entry_t entry;
    uint8_t found = 0;
    uint32_t best_seq = 0;
    uint16_t best_index = 0;

    for (uint16_t i = 0; i < ENTRY_COUNT; i++) {
        uint16_t addr = entry_addr(i);
        eeprom_read_block(&entry, (void *)addr, ENTRY_SIZE);

        if (entry.valid != EEPROM_VALID_MARK)
            continue;

        uint8_t calc_crc = crc8((uint8_t *)&entry.seq,
                                sizeof(entry.seq) + sizeof(entry.value));

        if (calc_crc != entry.crc)
            continue;

        if (!found || entry.seq > best_seq) {
            best_seq   = entry.seq;
            best_index = i;
            *e = entry;
            found = 1;
        }
    }

    if (found) {
        seq_last   = e->seq;
        index_last = best_index;
    }
    return found;
}

// --- 32-Bit atomar aus volatile lesen ---
static uint32_t read_volatile32(volatile uint32_t *ptr)
{
    uint32_t val1, val2;
    do {
        val1 = *ptr;
        val2 = *ptr;
    } while (val1 != val2);
    return val1;
}

// --- Initialisierung ---
uint8_t eeprom_logger_init(eeprom_value_t *value)
{
    uint8_t valid = load_latest(&e_last);
    if (valid) {
        *value = e_last.value;
    } else {
        seq_last = 0;
        index_last = ENTRY_COUNT - 1;
        *value = 0;
    }
    return valid;
}

// --- Wert speichern ---
uint32_t eeprom_logger_store(volatile eeprom_value_t *value)
{
    eeprom_entry_t e;

    // atomar Wert aus ISR kopieren
    eeprom_value_t safe_value = read_volatile32(value);

    e.seq = seq_last + 1;
    e.value = safe_value;
    e.crc = crc8((uint8_t *)&e.seq, sizeof(e.seq) + sizeof(e.value));
    e.valid = 0xFF; // ungültig während des Schreibens

    uint16_t next_index = (index_last + 1) % ENTRY_COUNT;
    uint16_t addr = entry_addr(next_index);

    // 1. Block schreiben (valid noch ungültig)
    eeprom_update_block(&e, (void *)addr, ENTRY_SIZE);

    // 2. valid-Byte zum Schluss → atomar
    eeprom_update_byte((uint8_t *)(addr + offsetof(eeprom_entry_t, valid)),
                        EEPROM_VALID_MARK);

    // lokale Variablen aktualisieren
    e_last = e;
    seq_last = e.seq;
    index_last = next_index;
    return(seq_last);
}

// --- Letzten gespeicherten Wert abrufen ---
eeprom_value_t eeprom_logger_get_last(void)
{
    return e_last.value;
}

uint32_t eeprom_logger_store_if_changed_safe(volatile eeprom_value_t *value)
{
    // Atomar Wert aus ISR kopieren
    eeprom_value_t safe_value = read_volatile32(value);

    // Temporäre Variable für letzten gültigen EEPROM-Wert
    eeprom_entry_t last_entry;
    uint8_t found = load_latest(&last_entry);

    if (found && safe_value == last_entry.value) {
        // Wert ist gleich dem letzten gültigen EEPROM-Wert → nichts tun
        return(0);
    }

    // Wert ist neu → sicheren Store durchführen
    return(eeprom_logger_store(&safe_value));
}

