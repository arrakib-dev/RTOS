#include "ses_led.h"
#include "ses_scheduler.h"
#include "ses_button.h"
#include "ses_adc.h"
#include "ses_display.h"
#include "ses_usbserial.h"
#include "ses_fan.h"
#include "ses_fanspeed.h"
#include "ses_fsm.h"

/* MACRO *********************************************************/
// button debouncer task period time in ms:
#define BUTTON_TASK_EXEC_MS				5

/* GLOBAL VARIABLES *********************************************************/

/*FUNCTION DEFINITION *************************************************/

/**
* Button_Task: executes the button debouncing function (button_checkState)
*/
void ButtonDebouncer_Task(){

	button_checkState();

}


/**
* PushButtonCallback: performs the debounced button push events (fan enable or disable)
*/
void PushButtonCallback(){

	// prevButtonState stores the current fan power state (a.k.a button state)
	static bool prevButtonState = false;

	// state negation
	prevButtonState = !prevButtonState;

	// red LED indicates the current state
	led_redToggle();

	if(prevButtonState){
		fan_enable();

	}else{
		fan_disable();

	}
}

fsm_return_status_t state_setSystemHour(fsm_t * fsm, const event_t * event) {
	switch(event->signal) {
		//... handling of other events
		case PUSHBUTTON_PRESSED:
			fsm->state = state_setSystemMin;
		return RET_TRANSITION;
			default:
		return RET_IGNORED;
	}
}

fsm_return_status_t state_setSystemMin(fsm_t * fsm, const event_t * event) {
	switch(event->signal) {
		//... handling of other events
		case PUSHBUTTON_PRESSED:
			fsm->state = state_NormalOperation;
		return RET_TRANSITION;
			default:
		return RET_IGNORED;
	}
}

fsm_return_status_t state_NormalOperation(fsm_t * fsm, const event_t * event) {
	switch(event->signal) {
		//... handling of other events
		case ROTARYBUTTON_PRESSED:
			fsm->state = state_setSystemHour;
		return RET_TRANSITION;
			default:
		return RET_IGNORED;
	}
}

int main(void) {

	// LED initializations
	led_redInit();
	led_greenInit();
	led_yellowInit();

	// display initialization
	display_init();

	// button initialization
	button_init(BUTT_DEBOUNCING_TASK);
	button_setPushButtonCallback(PushButtonCallback);

	// Task descriptors for the LED task and the ButtonDebouncer task
	task_descriptor_t ButtonDebouncer_task;

	// ButtonDebouncer task initialization
	ButtonDebouncer_task.task 	= ButtonDebouncer_Task;
	//ButtonDebouncer_task.param;	// parameter is empty here 
	ButtonDebouncer_task.expire = BUTTON_TASK_EXEC_MS;
	ButtonDebouncer_task.period = BUTTON_TASK_EXEC_MS;
	scheduler_add(&ButtonDebouncer_task);

	scheduler_init();

	

	// fsm
	static fsm_t * alarmClock;

	fsm_init(alarmClock, state_setSystemHour);

	// Enable global interrupt
	sei();

	

	// Scheduler start
	scheduler_run();

	return 0;
}


