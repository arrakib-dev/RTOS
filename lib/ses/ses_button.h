#ifndef SES_BUTTON_H_
#define SES_BUTTON_H_

/* INCLUDES ******************************************************************/

#include <stdbool.h>

/* DEFINES *******************************************************************/
#define BUTT_DEBOUNCING_NONE		0
#define BUTT_DEBOUNCING_TIMER		1
#define BUTT_DEBOUNCING_TASK        2

/* FUNCTION PROTOTYPES *******************************************************/

/**
 * Initializes rotary encoder button and pushbutton
 */
void button_init(uint8_t);

/** 
 * Get the state of the pushbutton.
 */
bool button_isPushButtonPressed(void);

/** 
 * Get the state of the rotary button.
 */
bool button_isRotaryButtonPressed(void);

/** 
 * 
 */
typedef void (*pButtonCallback)(void);

/** 
 * Rotarybutton interrupt Callback
 */
void button_setRotaryButtonCallback(pButtonCallback callback);

/** 
 * Pushbutton interrupt Callback
 */
void button_setPushButtonCallback(pButtonCallback callback);

/** 
 * Button debouncing function
 */
void button_checkState(void);

#endif /* SES_BUTTON_H_ */
