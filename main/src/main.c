#include "ses_led.h"
#include "ses_scheduler.h"
#include "ses_button.h"
#include "ses_display.h"
#include "ses_usbserial.h"
#include "Alarm_fsm.h"

/* MACRO *********************************************************/
// task period time in ms:
#define BUTTON_TASK_EXEC_MS			5	// 5ms period time for the button debouncer task
#define FSM_TASK_EXEC_MS			1	// 1ms period time for the FSM task running the finite-state machine

/* VARIABLES *****************************************************/

// FSM event variables
volatile event_t timerEvent;
volatile event_t pushBtnEvent;
volatile event_t rotBtnEvent;

/*TASK FUNCTION DEFINITION *************************************************/

/**
* Button_Task: executes the button debouncing function (button_checkState)
*/
void ButtonDebouncer_Task(void* p){

	button_checkState();

}


/**
* FSM_Task: dispatches the finite-State machine 
*
* @param p receives an fsm_t pointer type pointing to the finite-state machine variable
*/
void FSM_Task(void * p){
	fsm_t* fsm = (fsm_t*)p;
	event_t alarmEvent;

	// get the current system time in human readable format
	time_t actTime = system_time_wrapper_2_time(scheduler_getTime());

	// green led blinks synchronously with the second counter
	if(actTime.second % 2 == 0)
		led_greenOn();
	else
		led_greenOff();
	
	// check the timer event
	fsm_dispatch(fsm, (const event_t*)&timerEvent);
	timerEvent.signal = NO_EVENT;
	
	// check the push button event
	fsm_dispatch(fsm, (const event_t*)&pushBtnEvent);
	pushBtnEvent.signal = NO_EVENT;

	// check the rotary button event
	fsm_dispatch(fsm, (const event_t*)&rotBtnEvent);
	rotBtnEvent.signal = NO_EVENT;

	// check the alarm event
	alarmEvent.signal = (actTime.hour == fsm->timeSet.hour && actTime.minute == fsm->timeSet.minute && actTime.second == fsm->timeSet.second) ? ALARM_TIME : NO_EVENT;
	fsm_dispatch(fsm, (const event_t*)&alarmEvent);


}

/**
* RedLED_Toggler_Task: toggles the red LED per call 
*/
void RedLED_Toggler_Task(){

	led_redToggle();

}

/**
* Timer_Task: sets an event for the FSM signaling the predefined time elapsed 
*/
void Timer_Task(){
	timerEvent.signal = TIMER_ELAPSED;
}


/**
* PushButtonCallback: called by the button debouncer if a valid push button press occured
*						and sets an event for the FSM
*/
void PushButtonCallback(){
	pushBtnEvent.signal = PUSH_BUTT_PRESS;

}

/**
* RotaryButtonCallback: called by the button debouncer if a valid rotary button press occured
*						and sets an event for the FSM
*/
void RotaryButtonCallback(){
	rotBtnEvent.signal = ROTARY_BUTT_PRESS;

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
	button_setRotaryButtonCallback(RotaryButtonCallback);

	// FSM initialization
	fsm_init((fsm_t*)&AlarmFSM, state_setSystemTimeHour);

	// Task descriptors for the LED task and the ButtonDebouncer task
	task_descriptor_t ButtonDebouncer_task, FSM_task;

	// ButtonDebouncer task initialization
	ButtonDebouncer_task.task 	= ButtonDebouncer_Task;
	//ButtonDebouncer_task.param;	// parameter is empty here 
	ButtonDebouncer_task.expire = BUTTON_TASK_EXEC_MS;
	ButtonDebouncer_task.period = BUTTON_TASK_EXEC_MS;
	scheduler_add(&ButtonDebouncer_task);

	// FSM task initialization
	FSM_task.task 	= FSM_Task;
	FSM_task.param  = &AlarmFSM;
	FSM_task.expire = FSM_TASK_EXEC_MS;
	FSM_task.period = FSM_TASK_EXEC_MS;
	scheduler_add(&FSM_task);

	scheduler_init();

	// Enable global interrupt
	sei();

	// Scheduler start
	scheduler_run();

	return 0;
}
