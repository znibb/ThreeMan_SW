#include <avr/eeprom.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <stdint.h>
#include <stdlib.h>
#include <util/delay.h>

#define DATA_PIN PB3
#define CLOCK_PIN PB4

// Pips per die face, one nibble per die:
// bit0 = center, bit1 = W/E, bit2 = SW/NE, bit3 = NW/SE
static const uint8_t PIPS[6] = {
    0b0001,  // 1
    0b0100,  // 2
    0b0101,  // 3
    0b1100,  // 4
    0b1101,  // 5
    0b1110,  // 6
};

static uint16_t EEMEM boot_counter;

static void clock_pulse() {
    PORTB |= (1 << CLOCK_PIN);
    _delay_us(1);
    PORTB &= ~(1 << CLOCK_PIN);
    _delay_us(1);
}

// SHCLK and STCLK share one net, so the storage register latches the shift
// register as it was before each edge. A 9th pulse latches the full byte.
static void shift_out(uint8_t value) {
    for (uint8_t i = 0; i < 8; i++) {
        if (value & 0x80) {
            PORTB |= (1 << DATA_PIN);
        } else {
            PORTB &= ~(1 << DATA_PIN);
        }
        clock_pulse();
        value <<= 1;
    }
    clock_pulse();
}

// Collect noise from the LSBs of a floating ADC input (ADC1 / PB2)
static uint16_t adc_noise() {
    ADMUX = (1 << MUX0);                                    // ADC1, Vcc reference
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1);     // clk/64
    uint16_t seed = 0;
    for (uint8_t i = 0; i < 16; i++) {
        ADCSRA |= (1 << ADSC);
        while (ADCSRA & (1 << ADSC)) {
        }
        seed = (seed << 1) ^ (seed >> 15) ^ (ADC & 1) ^ ((uint16_t)ADCL << 3);
    }
    ADCSRA = 0;  // disable ADC
    return seed;
}

// Unbiased roll in 1..6
static uint8_t roll() {
    uint8_t r;
    do {
        r = rand() & 7;
    } while (r >= 6);
    return r + 1;
}

int main() {
    DDRB |= (1 << DATA_PIN) | (1 << CLOCK_PIN);

    // The counter guarantees a different seed on every boot even if the ADC
    // noise is poor
    uint16_t count = eeprom_read_word(&boot_counter);
    eeprom_update_word(&boot_counter, count + 1);
    srand(adc_noise() ^ (count * 40503u));

    uint8_t die1 = roll();
    uint8_t die2 = roll();
    shift_out(PIPS[die1 - 1] | (PIPS[die2 - 1] << 4));

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    while (1) {
        sleep_mode();
    }
}
