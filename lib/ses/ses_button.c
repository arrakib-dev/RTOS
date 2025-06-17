/* INCLUDES ******************************************************************/
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "ses_button.h"
#include "ses_timer.h"


/* DEFINES & MACROS **********************************************************/

// Push button wiring on SES board
#define BUTTON_PUSH_PORT    PORTB
#define BUTTON_PUSH_DDR     DDRB
#define BUTTON_PUSH_PIN     PINB
#define BUTTON_PUSH_BIT     4

// Rotary button wiring on SES board
#define BUTTON_ROTARY_PORT  PORTB
#define BUTTON_ROTARY_DDR   DDRB
#define BUTTON_ROTARY_PIN   PINB
#define BUTTON_ROTARY_BIT   5

// defines for button debouncing
#define BUTTON_NUM_DEBOUNCE_CHECKS          10
#define BUTTON_DEBOUNCE_POS_PUSHBUTTON      0x01
#define BUTTON_DEBOUNCE_POS_ROTARYBUTTON    0x02

//function pointers for button callbacks
static volatile pButtonCallback RotaryButtonCB;
static volatile pButtonCallback PushButtonCB;

// predeclaration
void button_checkState(void);

/* FUNCTION DEFINITION *******************************************************/

void button_init(uint8_t debouncing){
    // Push button initialization
    // Set the corresponding pin to input 
    BUTTON_PUSH_DDR  &= ~(1 << BUTTON_PUSH_BIT);
    // Activate the internal pull-up resistor
    BUTTON_PUSH_PORT |=  (1 << BUTTON_PUSH_BIT);

    // Rotary button initialization
    // Set the corresponding pin to output 
    BUTTON_ROTARY_DDR  &= ~(1 << BUTTON_ROTARY_BIT);
    // Activate the internal pull-up resistor
    BUTTON_ROTARY_PORT |=  (1 << BUTTON_ROTARY_BIT);

    // case 1: there is no button debouncing (interrupt)
    if (debouncing == BUTT_DEBOUNCING_NONE) {
        // Disable Pin Change Interrupt during initialization
        PCICR &= ~(1 << PCIE0);

        // Enable interrupt for push- and rotary buttons
        PCMSK0 |= (1 << BUTTON_PUSH_BIT) | (1 << BUTTON_ROTARY_BIT);

        // Clear pending pin-change interrupt
        PCIFR |= (1 << PCIFR);

        // Enable Pin Change Interrupt
        PCICR |= (1 << PCIE0);
    }

    // case 2: button debouncing by hardware timer
    if (debouncing == BUTT_DEBOUNCING_TIMER) {
        timer1_start();
        timer1_setCallback(button_checkState);

    } 

    // case 3: button debouncing by a dedicated task
    if (debouncing == BUTT_DEBOUNCING_TASK) {/* nothing happens here*/}

}

bool button_isPushButtonPressed(void){
    // Read and mask the corresponding bit, then negate it because the button pulls to GND if pushed
    return !(bool)(BUTTON_PUSH_PIN & (1 << BUTTON_PUSH_BIT));
}

bool button_isRotaryButtonPressed(void){
    // Read and mask the corresponding bit, then negate it because the button pulls to GND if pushed
    return !(bool)(BUTTON_ROTARY_PIN & (1 << BUTTON_ROTARY_BIT));
}

ISR(PCINT0_vect){
    
    if ( (RotaryButtonCB != NULL) & button_isRotaryButtonPressed() ) {
        RotaryButtonCB();
    }
    
    if ( (PushButtonCB != NULL) & button_isPushButtonPressed() ) {
        PushButtonCB();
    }
}


// Rotarybutton interrupt Callback setter
void button_setRotaryButtonCallback(pButtonCallback callback){
    // callback validation check
    if(callback != NULL){
        RotaryButtonCB = callback;
    }

    return;
}


// Pushbutton interrupt Callback setter
void button_setPushButtonCallback(pButtonCallback callback){
    // callback validation check
    if(callback != NULL){
        PushButtonCB = callback;
    }

    return;
}


void button_checkState(){
    static uint8_t state[BUTTON_NUM_DEBOUNCE_CHECKS] = { 0 };
    static uint8_t index = 0;
    static uint8_t debouncedState = 0;
    uint8_t lastDebouncedState = debouncedState;

    // each bit in every state byte represents one button
    state[index] = 0;
    if (button_isPushButtonPressed()) {
        state[index] |= BUTTON_DEBOUNCE_POS_PUSHBUTTON;
    }
    if (button_isRotaryButtonPressed()) {
        state[index] |= BUTTON_DEBOUNCE_POS_ROTARYBUTTON;
    }

    index++;
    if (index == BUTTON_NUM_DEBOUNCE_CHECKS) {
        index = 0;
    }

    // init compare value and compare with ALL reads, only if
    // we read BUTTON_NUM_DEBOUNCE_CHECKS consistent ”1's” in the state
    // array, the button at this position is considered pressed
    uint8_t j = 0xFF;
    for (uint8_t i = 0; i < BUTTON_NUM_DEBOUNCE_CHECKS; i++) {
        j = j & state[i];
    }
    debouncedState = j;

    /* trigger callback only if: the corresponding debounced state bit is 1 AND the last debounced bit is 0
    -> these conditions ensure that callback will be invoked only once */
    if ( (debouncedState & BUTTON_DEBOUNCE_POS_ROTARYBUTTON) && 
        !(lastDebouncedState & BUTTON_DEBOUNCE_POS_ROTARYBUTTON) && 
        RotaryButtonCB != NULL ) {

        RotaryButtonCB();

    }
    /* trigger callback only if: the corresponding debounced state bit is 1 AND the last debounced bit is 0
    -> these conditions ensure that callback will be invoked only once */
    if ( (debouncedState & BUTTON_DEBOUNCE_POS_PUSHBUTTON) && 
        !(lastDebouncedState & BUTTON_DEBOUNCE_POS_PUSHBUTTON) && 
        PushButtonCB != NULL ) {

        PushButtonCB();

    }
    
}