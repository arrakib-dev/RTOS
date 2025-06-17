/* INCLUDES ******************************************************************/

#include <avr/io.h>
#include "ses_led.h"

/* DEFINES & MACROS **********************************************************/

// red LED wiring on SES board
#define LED_RED_PORT       	PORTF
#define LED_RED_DDR         DDRF
#define LED_RED_BIT         5

// green LED wiring on SES board
#define LED_GREEN_PORT      PORTD
#define LED_GREEN_DDR       DDRD
#define LED_GREEN_BIT       2

// yellow LED wiring on SES board
#define LED_YELLOW_PORT     PORTD
#define LED_YELLOW_DDR      DDRD
#define LED_YELLOW_BIT      3


/* FUNCTION DEFINITION *******************************************************/

void led_redInit(void) {
    // Set the corresponding pin to output
    LED_RED_DDR |= (1 << LED_RED_BIT);
    // Turn off the led initially
    led_redOff();

}

void led_redToggle(void) {
    // invert the corresponding bit
    LED_RED_PORT ^= (1 << LED_RED_BIT);
}

void led_redOn(void) {
    // clear the corresponding pin to set the output low (0V)
    LED_RED_PORT &= ~(1 << LED_RED_BIT);
}

void led_redOff(void) {
    // set the corresponding pin to set the output high (Vcc)
    LED_RED_PORT |= (1 << LED_RED_BIT);
}

void led_yellowInit(void) {
    // Set the corresponding pin to output
    LED_YELLOW_DDR |= (1 << LED_YELLOW_BIT);
    // Turn off the led initially
    led_yellowOff();
}

void led_yellowToggle(void) {
    // invert the corresponding bit
    LED_YELLOW_PORT ^= (1 << LED_YELLOW_BIT);
}

void led_yellowOn(void) {
    // clear the corresponding pin to set the output low (0V)
    LED_YELLOW_PORT &= ~(1 << LED_YELLOW_BIT);
}

void led_yellowOff(void) {
    // set the corresponding pin to set the output high (Vcc)
    LED_YELLOW_PORT |= (1 << LED_YELLOW_BIT);
}

void led_greenInit(void) {
    // Set the corresponding pin to output
    LED_GREEN_DDR |= (1 << LED_GREEN_BIT);
    // Turn off the led initially
    led_greenOff();
}

void led_greenToggle(void) {
    // invert the corresponding bit
    LED_GREEN_PORT ^= (1 << LED_GREEN_BIT);
}

void led_greenOn(void) {
    // clear the corresponding pin to set the output low (0V)
    LED_GREEN_PORT &= ~(1 << LED_GREEN_BIT);
}

void led_greenOff(void) {
    // set the corresponding pin to set the output high (Vcc)
    LED_GREEN_PORT |= (1 << LED_GREEN_BIT);
}
