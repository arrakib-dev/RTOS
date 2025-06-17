/* INCLUDES ******************************************************************/

#include "ses_fanspeed.h"
#include "ses_led.h"
#include "ses_display.h"
#include "ses_timer.h"
#include <avr/interrupt.h>
#include <util/atomic.h>

/* DEFINES *******************************************************************/

#define TIM1_BASE_CLK_FREQ  (uint32_t)16000000                              // Timer 1 base clock frequency (16MHz)
#define TIM1_PRESCALER      (uint16_t)256                                   // Timer 1 prescaler (256)
#define TIM1_CLK_FREQ       (uint32_t)(TIM1_BASE_CLK_FREQ/TIM1_PRESCALER)   // Timer 1 divided frequency
#define TIM1_MAX_VALUE      (uint16_t)((1 << 16) - 1)                       // uint16_t max decimal value  

#define TIM1_COMP_VAL_FOR_1SEC  TIM1_BASE_CLK_FREQ/TIM1_PRESCALER

#define BUFF_SIZE   7
#define MEDIAN_POS  BUFF_SIZE/2

/* VARIABLES *******************************************************/

static uint16_t lastSpeedRecord;                // global variable containing the last measured RPM value
static uint16_t samples[BUFF_SIZE] = {0};       // global array containing the last n (= SPEED_BUFF_SIZE) measured RPM value
static uint8_t buffIdx = 0;                     // current write index for "samples" array

/* FUNCTION DEFINITIONS *******************************************************/

/**
* fan_timer1_callback: called by hardware timer1 if it reaches the compare value (it takes 1s)
                        This function called when at least 1s elapses between two tacho signals
*/
static void fan_timer1_callback(void);



void fanspeed_init(){

    /* External interrupt (INT6) configuration */
    // Set the PE6 pin to input
    DDRE &= ~(1 << PE6);

    // Disable INT6
    EIMSK &= ~(1 << INT6);

    // Activate INT6 rising edge interrupt ensuring the speed signal detection 
    EICRB |= (1 << ISC61);
    EICRB |= (1 << ISC60);
 
	// Clear possible pending interrupt
	EIFR &= ~(1 << INT6);

    // Enable INT6
    EIMSK |= (1 << INT6);


    /* Timer 1 configuration */
    timer1_start();
    timer1_setCallback(fan_timer1_callback);

    OCR1A = TIM1_COMP_VAL_FOR_1SEC;
}



uint16_t fanspeed_getRecent(){

    uint16_t temp;

    // atomic access is neccessary because of multi byte operation (16bit integer vs 8bit core)
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
        temp = lastSpeedRecord;
    }

    return temp;

}


/**
 * swap: swaps two variables (helper function for quick-select algorithm)
 *
 * @param a   first variable to be swapped
 *
 * @param b   second variable to be swapped
 *
 */
static void swap(uint16_t *a, uint16_t *b) {
    uint16_t temp = *a;
    *a = *b;
    *b = temp;
}

uint16_t fanspeed_getFiltered(){

    /*  implementation of quick-select algorithm 
        that selects the n-th smallest element of an array
        (searches the modulo if n = the half of the array size)
    */

    uint16_t left = 0;              // left index points to the beginning of the array intitially 
    uint16_t right = BUFF_SIZE - 1; // right index points to the end of the array intitially 

    while (left <= right) {

        uint16_t pivot = samples[right];    // pivot is always the rightmost element
        int16_t i = left - 1;

        for (uint16_t j = left; j <= right - 1; j++) {

            if (samples[j] <= pivot) {
                ++i;
                swap(&samples[i], &samples[j]);
            }
        }
        swap(&samples[i + 1], &samples[right]);
        uint16_t pivotIndex = i + 1;


        if (pivotIndex == MEDIAN_POS) {
            // if the pivot is at the median position, the median value is the pivot element itself
            return samples[pivotIndex];

        } else if (pivotIndex < MEDIAN_POS) {
            /* if the pivot element is on the left side of the array, than the median is on the right side 
                (right index does not change, left index becomes larger)
            */
            left = pivotIndex + 1;

        } else {
            /* if the pivot element is on the right side of the array, than the median is on the left side 
                (left index does not change, right index becomes smaller)
            */
            right = pivotIndex - 1;

        }
    }

    return 0;   // this part is just for case (never executed), because quick-select will always terminate 
}

/*
* Measured RPM values for:
* 0% -> 500RPM
* 100% -> 2200RPM
*/
ISR(INT6_vect){
    // toggle yellow LED for visualization of the tacho signal
    led_yellowToggle();

    /* calculate the RPM using reciprocal method: 
        reciprocal method is more suitable for lower frequencies 
        500RPM  -> 16.6Hz tacho signal frequency  (500RPM  / 60 * 2(tacho signal/rotate) = 16.6Hz)
        2200RPM -> 73.3Hz tacho signal frequency  (2200RPM / 60 * 2(tacho signal/rotate) = 73.3Hz)
        these frequency values are much lower than the timer1's 62.5kHz frequency
    */
    lastSpeedRecord = (60 / 2 * TIM1_CLK_FREQ) / TCNT1;
    TCNT1 = 0;

    // store the last RPM value to the "samples" array
    samples[buffIdx] = lastSpeedRecord;
    ++buffIdx;
    // modulo by the size of the "samples" array to implement a circular buffer
    buffIdx %= BUFF_SIZE;

}

void fan_timer1_callback(){
    // toggle green LED for visualization 
    led_greenToggle();

    // time elapsed -> fan is almost stopped
    lastSpeedRecord = 0;

    // store the last RPM value (which is 0 here) to the "samples" array
    samples[buffIdx] = lastSpeedRecord;
    ++buffIdx;
    // modulo by the size of the "samples" array to implement a circular buffer
    buffIdx %= BUFF_SIZE;


}
