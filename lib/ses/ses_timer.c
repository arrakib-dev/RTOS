/* INCLUDES ******************************************************************/
#include <stdlib.h>
#include <avr/interrupt.h>

#include "ses_timer.h"

/* DEFINES & MACROS **********************************************************/
// Timer compare value for 1ms and 5ms
#define TIMER0_CYC_FOR_1MILLISEC	249
#define TIMER1_CYC_FOR_5MILLISEC    312

// Timer_n mode macros
#define TIMER_MODE_MASK			0x03
#define TIMER_MODE_BIT_POS		0
#define TIMER_CTC_MODE			0x02
#define TIMER_PSC_MASK			0x07
#define TIMER_PSC_BIT_POS		0
#define TIMER_PSC64				0x03
#define TIMER_STOP				0x00


static pTimerCallback fp_Timer0_Callback;
static pTimerCallback fp_Timer1_Callback;

/*FUNCTION DEFINITION ********************************************************/

void timer0_setCallback(pTimerCallback cb) {
	if(cb != NULL){
		fp_Timer0_Callback = cb;
	}
	return;
}

void timer0_start() {
	// Set CTC mode
	TCCR0A &= ~(TIMER_MODE_MASK << TIMER_MODE_BIT_POS);
	TCCR0A |=  (TIMER_CTC_MODE << TIMER_MODE_BIT_POS);

	// Set prescaler to 64
	TCCR0B &= ~(TIMER_PSC_MASK << TIMER_PSC_BIT_POS);
	TCCR0B |=  (TIMER_PSC64 << TIMER_PSC_BIT_POS);

	// Set interrupt for Compare A
	TIMSK0 |= (1 << OCIE0A);

	// Clear possible pending interrupt
	TIFR0 &= ~(1 << OCF0A);

	// Set the compare value for 1ms interruption
	OCR0A = TIMER0_CYC_FOR_1MILLISEC;
	
}


void timer0_stop() {
	// Clear Prescaler bits (CS02-CS00)
	TCCR0B &= ~(TIMER_PSC_MASK << TIMER_PSC_BIT_POS);
	// Set prescaler to 0 (no clock source)
	TCCR0B |=  (TIMER_STOP << TIMER_PSC_BIT_POS);
}

void timer1_setCallback(pTimerCallback cb) {
	if(cb != NULL){
		fp_Timer1_Callback = cb;
	}
	return;
}


void timer1_start() {
	// Set CTC mode
	TCCR1A &= ~(1 << WGM10);
	TCCR1A &= ~(1 << WGM11);
	TCCR1B |=  (1 << WGM12);
	TCCR1B &= ~(1 << WGM13);

	// Set prescaler to 256
	TCCR1B &= ~(1 << CS10);
	TCCR1B &= ~(1 << CS11);
    TCCR1B |=  (1 << CS12);

	// Set interrupt for Compare A
	TIMSK1 |= (1 << OCIE1A);

	// Clear possible pending interrupt
	TIFR1 &= ~(1 << OCF1A);

	// Set the compare value for 1ms interruption
	OCR1A = TIMER1_CYC_FOR_5MILLISEC;
}


void timer1_stop() {
	// Clear Prescaler bits (CS12-CS10)
	TCCR1B &= ~(TIMER_PSC_MASK << TIMER_PSC_BIT_POS);
	// Set prescaler to 0 (no clock source)
	TCCR1B |=  (TIMER_STOP << TIMER_PSC_BIT_POS);
}

ISR(TIMER0_COMPA_vect) {
	fp_Timer0_Callback();
}


ISR(TIMER1_COMPA_vect) {
	fp_Timer1_Callback();
}
