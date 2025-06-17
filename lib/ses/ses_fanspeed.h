#ifndef SES_FANSPEED_H_
#define SES_FANSPEED_H_

/* INCLUDES ******************************************************************/
#include <stdint.h>
#include <avr/io.h>

/* DEFINES *******************************************************************/


/* FUNCTION PROTOTYPES *******************************************************/

/**
 * fanspeed_init: initialises the pins, timer and interrupts needed for the fan
 */
void fanspeed_init(void);

/**
 * fanspeed_getRecent: returns the last fan speed measurement
 * 
 * @return fan RPM value
 */
uint16_t fanspeed_getRecent(void);

/**
 * fanspeed_getFiltered: returns the median filtered fan speed measurement
 * 
 * @return fan filtered RPM value
 */
uint16_t fanspeed_getFiltered(void);


#endif    /* SES_FANSPEED_H_ */