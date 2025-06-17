#ifndef SES_FAN_H_
#define SES_FAN_H_

/* INCLUDES ******************************************************************/
#include <stdint.h>

/* DEFINES *******************************************************************/


/* FUNCTION PROTOTYPES *******************************************************/

/**
 * fan_init: initialises the timer providing PWM
 */
void fan_init(void);


/**
 * fan_enable: enables the fan power and sets the duty cycle start value to 0
 */
void fan_enable(void);


/**
 * fan_disable: disables the fan power and sets the duty cycle to 0
 */
void fan_disable(void);


/**
 * fan_setDutyCycle: sets the duty cycle for the fan control PWM
 * 
 * @param dc    duty cycle between 0 and 255   
 */
void fan_setDutyCycle(uint8_t dc);

#endif  /* SES_FAN_H_ */