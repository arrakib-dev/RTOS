#include "ses_led.h"
#include "ses_scheduler.h"
#include "ses_button.h"
#include "ses_display.h"
#include "ses_usbserial.h"
#include "Alarm_fsm.h"

/* MACRO *********************************************************/
// task period time in ms:
#define TIMER_TASK_EXEC_MS			5000    // 5000ms = 5s timing for the timer task
#define REDLED_TOGGLER_TASK_EXEC_MS	125     // 125ms 

/* EXTERN FUNCTION DECLARATIONS *********************************/
//these functions are defined in another file but here are used too 
extern void RedLED_Toggler_Task();
extern void Timer_Task();



/* STATE FUNCTION PREDECLARATIONS *********************************************************/

/**
 * system time minute setter state-function
 *
 * @param fsm	pointer to the finite-state machine variable containing the state information
 * 
 * @param event pointer to an event variable containing the actual event to be handled
 *
 * @return		fsm status about the outcome of the state execution
 */
static fsm_return_status_t state_setSystemTimeMinute(fsm_t * fsm, const event_t * event);

/**
 * state function for normal operation with disabled alarm
 *
 * @param fsm	pointer to the finite-state machine variable containing the state information
 * 
 * @param event pointer to an event variable containing the actual event to be handled
 *
 * @return		fsm status about the outcome of the state execution
 */
static fsm_return_status_t state_normalOperationAlarmDisabled(fsm_t * fsm, const event_t * event);

/**
 * state function for normal operation with enabled alarm
 *
 * @param fsm	pointer to the finite-state machine variable containing the state information
 * 
 * @param event pointer to an event variable containing the actual event to be handled
 *
 * @return		fsm status about the outcome of the state execution
 */
static fsm_return_status_t state_normalOperationAlarmEnabled(fsm_t * fsm, const event_t * event);

/**
 * state function for alarm 
 *
 * @param fsm	pointer to the finite-state machine variable containing the state information
 * 
 * @param event pointer to an event variable containing the actual event to be handled
 *
 * @return		fsm status about the outcome of the state execution
 */
static fsm_return_status_t state_Alarm(fsm_t * fsm, const event_t * event);

/**
 * state function for alarm time hour setting
 *
 * @param fsm	pointer to the finite-state machine variable containing the state information
 * 
 * @param event pointer to an event variable containing the actual event to be handled
 *
 * @return		fsm status about the outcome of the state execution
 */
static fsm_return_status_t state_setAlarmTimeHour(fsm_t * fsm, const event_t * event);

/**
 * state function for alarm time minute setting
 *
 * @param fsm	pointer to the finite-state machine variable containing the state information
 * 
 * @param event pointer to an event variable containing the actual event to be handled
 *
 * @return		fsm status about the outcome of the state execution
 */
static fsm_return_status_t state_setAlarmTimeMinute(fsm_t * fsm, const event_t * event);



// state function definitions

fsm_return_status_t state_setSystemTimeHour(fsm_t * fsm, const event_t * event){

    // parameter check
	if(fsm == NULL || event == NULL)
		return RET_ERROR;

    // state transitions and entry/exit events
	switch(event->signal){
		case ENTRY:
			fsm->timeSet.hour   = 0;
			fsm->timeSet.minute = 0;
			break;

		case ROTARY_BUTT_PRESS:
			++fsm->timeSet.hour;
			fsm->timeSet.hour %= HOUR_PER_DAY;
			break;

		case PUSH_BUTT_PRESS:
			fsm->state = state_setSystemTimeMinute;
			return RET_TRANSITION;
			break;

	}

    // display the time and the current state
	display_clear();
	display_setCursor(0,0);
	fprintf(displayout, "Set System Time: Hour\n");
	fprintf(displayout, "%02d:%02d\n", fsm->timeSet.hour, fsm->timeSet.minute);
	display_update();

	return RET_IGNORED;
}


fsm_return_status_t state_setSystemTimeMinute(fsm_t * fsm, const event_t * event){

    // parameter check
	if(fsm == NULL || event == NULL)
		return RET_ERROR;

    // state transitions and entry/exit events
	switch(event->signal){
		case ROTARY_BUTT_PRESS:
			++fsm->timeSet.minute;
			fsm->timeSet.minute %= MIN_PER_HOUR;
			break;

		case PUSH_BUTT_PRESS:
			fsm->state = state_normalOperationAlarmDisabled;
			return RET_TRANSITION;
			break;

		case EXIT:
			scheduler_setTime(time_wrapper_2_system_time(fsm->timeSet));
			fsm->timeSet.hour   = 0;
			fsm->timeSet.minute = 0;
			break;

	}

   // display the time and the current state
	display_clear();
	display_setCursor(0,0);
	fprintf(displayout, "Set System Time: Minute\n");
	fprintf(displayout, "%02d:%02d\n", fsm->timeSet.hour, fsm->timeSet.minute);
	display_update();

	return RET_IGNORED;
}


fsm_return_status_t state_normalOperationAlarmDisabled(fsm_t * fsm, const event_t * event){

    // parameter check
	if(fsm == NULL || event == NULL)
		return RET_ERROR;


	fsm->isAlarmEnabled = false;

    // state transitions and entry/exit events
	switch(event->signal){
		case ENTRY:
			led_yellowOff();
			break;

		case ROTARY_BUTT_PRESS:
			fsm->state = state_normalOperationAlarmEnabled;
			return RET_TRANSITION;
			break;

		case PUSH_BUTT_PRESS:
			fsm->state = state_setAlarmTimeHour;
			return RET_TRANSITION;
			break;

	}

   // display the time and the current state
	time_t actTime = system_time_wrapper_2_time(scheduler_getTime());
	display_clear();
	display_setCursor(0,0);
	fprintf(displayout, "Clock, alarm disabled\n");
	fprintf(displayout, "%02d:%02d:%02d\n", actTime.hour, actTime.minute, actTime.second);
	display_update();

	return RET_IGNORED;
}


fsm_return_status_t state_normalOperationAlarmEnabled(fsm_t * fsm, const event_t * event){

    // parameter check
	if(fsm == NULL || event == NULL)
		return RET_ERROR;

	fsm->isAlarmEnabled = true;

    // state transitions and entry/exit events
	switch(event->signal){
		case ENTRY:
			led_yellowOn();
			break;

		case ROTARY_BUTT_PRESS:
			fsm->state = state_normalOperationAlarmDisabled;
			return RET_TRANSITION;
			break;

		case PUSH_BUTT_PRESS:
			fsm->state = state_setAlarmTimeHour;
			return RET_TRANSITION;
			break;

		case ALARM_TIME:
			fsm->state = state_Alarm;
			return RET_TRANSITION;
			break;

	}

   // display the time and the current state
	time_t actTime = system_time_wrapper_2_time(scheduler_getTime());
	display_clear();
	display_setCursor(0,0);
	fprintf(displayout, "Clock, alarm enabled\n");
	fprintf(displayout, "%02d:%02d:%02d\n", actTime.hour, actTime.minute, actTime.second);
	display_update();

	return RET_IGNORED;
}



fsm_return_status_t state_Alarm(fsm_t * fsm, const event_t * event){

    // parameter check
	if(fsm == NULL || event == NULL)
		return RET_ERROR;

    // variables for the 5s timer task and the red LED task 
    static task_descriptor_t Timer_task, RedLED_Toggler_task;

	// Timer_task (re)initialization
	Timer_task.task 	= Timer_Task;
	//Timer_task.param;	    // not used here
	Timer_task.expire 	= TIMER_TASK_EXEC_MS;
	Timer_task.period 	= 0;

	// RedLED_Toggler_task (re)initialization
	RedLED_Toggler_task.task 	= RedLED_Toggler_Task;
	//RedLED_Toggler_task.param;	// not used here	
	RedLED_Toggler_task.expire 	= REDLED_TOGGLER_TASK_EXEC_MS;
	RedLED_Toggler_task.period 	= REDLED_TOGGLER_TASK_EXEC_MS;

    // state transitions and entry/exit events
	switch(event->signal){
		case ENTRY:
        	scheduler_add(&RedLED_Toggler_task);
			scheduler_add(&Timer_task);
            led_redOn();
			break;

		case ROTARY_BUTT_PRESS:
			fsm->state = state_normalOperationAlarmEnabled;
			return RET_TRANSITION;
			break;

		case PUSH_BUTT_PRESS:
			fsm->state = state_normalOperationAlarmEnabled;
			return RET_TRANSITION;
			break;

		case TIMER_ELAPSED:
			fsm->state = state_normalOperationAlarmEnabled;
			return RET_TRANSITION;
			break;

		case EXIT:
			scheduler_remove(&Timer_task);
			scheduler_remove(&RedLED_Toggler_task);
            led_redOff();
			break;

	}

   // display the time and the current state
	time_t actTime = system_time_wrapper_2_time(scheduler_getTime());
	display_clear();
	display_setCursor(0,0);
	fprintf(displayout, "Alarm\n");
	fprintf(displayout, "%02d:%02d:%02d\n", actTime.hour, actTime.minute, actTime.second);
	display_update();

	return RET_IGNORED;

}


fsm_return_status_t state_setAlarmTimeHour(fsm_t * fsm, const event_t * event){

    // parameter check
	if(fsm == NULL || event == NULL)
		return RET_ERROR;

    // state transitions and entry/exit events
	switch(event->signal){
		case ROTARY_BUTT_PRESS:
			++fsm->timeSet.hour;
			fsm->timeSet.hour %= HOUR_PER_DAY;
			break;

		case PUSH_BUTT_PRESS:
			fsm->state = state_setAlarmTimeMinute;
			return RET_TRANSITION;
			break;

	}

   // display the time and the current state
	display_clear();
	display_setCursor(0,0);
	fprintf(displayout, "Set Alarm Time:Hour\n");
	fprintf(displayout, "%02d:%02d\n", fsm->timeSet.hour, fsm->timeSet.minute);
	display_update();

	return RET_IGNORED;
}


fsm_return_status_t state_setAlarmTimeMinute(fsm_t * fsm, const event_t * event){

    // parameter check
	if(fsm == NULL || event == NULL)
		return RET_ERROR;

    // state transitions and entry/exit events
	switch(event->signal){
		case ROTARY_BUTT_PRESS:
			++fsm->timeSet.minute;
			fsm->timeSet.minute %= MIN_PER_HOUR;
			break;

		case PUSH_BUTT_PRESS:
			if(fsm->isAlarmEnabled)
				fsm->state = state_normalOperationAlarmEnabled;
			else
				fsm->state = state_normalOperationAlarmDisabled;

			return RET_TRANSITION;
			break;

	}

   // display the time and the current state
	display_clear();
	display_setCursor(0,0);
	fprintf(displayout, "Set Alarm Time: Minute\n");
	fprintf(displayout, "%02d:%02d\n", fsm->timeSet.hour, fsm->timeSet.minute);
	display_update();

	return RET_IGNORED;
}
