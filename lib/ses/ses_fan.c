/* INCLUDES ******************************************************************/
#include "ses_fan.h"
#include "avr/io.h"

/* DEFINES *******************************************************************/
// fan power pin defines
#define FAN_POWER_DDR   DDRD
#define FAN_POWER_PORT  PORTD
#define FAN_POWER_PIN   PD4

// fan pwm pin defines
#define FAN_PWM_DDR    DDRC
#define FAN_PWM_PIN     PC6

/* FUNCTION DEFINITIONS ******************************************************/ 

void fan_init(void){

    // Configure PD4 and PC6 to output
    FAN_POWER_DDR |= (1 << FAN_POWER_PIN);
    FAN_PWM_DDR |= (1 << FAN_PWM_PIN);

    // Enable Timer3
    PRR1 &= ~(1 << PRTIM3);

    // Set Fast PWM 8bit mode
    TCCR3A |=  (1 << WGM30);
    TCCR3A &= ~(1 << WGM31);
    TCCR3B |= ~(1 << WGM32);
    TCCR3B &= ~(1 << WGM33);

    // Set Output Compare
    TCCR3A &= ~(1 << COM3A0);
    TCCR3A |=  (1 << COM3A1); 

    // Set prescaler division by 64
    TCCR3B |=  (1 << CS30);
    TCCR3B |=  (1 << CS31);
    TCCR3B &= ~(1 << CS32);

    // Reset
    OCR3A = 0;
}

void fan_enable(void){

    // Set the PD4 pin to high to turn on the FAN MOSFET
    FAN_POWER_PORT |= (1 << FAN_POWER_PIN);

    // reset the duty cycle to 0
    fan_setDutyCycle(0);
}

void fan_disable(void){

    // Set the PD4 pin to low to turn off the FAN MOSFET
    FAN_POWER_PORT &= ~(1 << FAN_POWER_PIN);

    // set the duty cycle to 0
    fan_setDutyCycle(0);
}

void fan_setDutyCycle(uint8_t dc){

    // write dc value to the output compare register
    OCR3A = dc;
}