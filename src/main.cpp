#include <avr/io.h>
#include <util/delay.h>

int main() {
    DDRB |= (1 << PB0);  // Set PB0 as output

    while (1) {
        PORTB ^= (1 << PB0); // Toggle LED
        _delay_ms(500);
    }
}
