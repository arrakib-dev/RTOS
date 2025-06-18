#ifndef ALARM_FSM_H_
#define ALARM_FSM_H_
/* INCLUDES *****************************************************************/
#include "ses_scheduler.h"

/* TYPEDEFS ********************************************************************/

typedef struct fsm_s fsm_t; //< typedef for alarm clock state machine
typedef struct event_s event_t; //< event type for alarm clock fsm

/** return values */
enum return_values {
    RET_HANDLED,    //< event was handled
    RET_IGNORED,    //< event was ignored; not used in this implementation
    RET_TRANSITION, //< event was handled and a state transition occurred
    RET_ERROR		//< pointer containing event was empty (NULL)
};

/* signals used by the Alarm Clock FSM */
enum {
	ENTRY,				//< entry event
	EXIT,				//< exit event
	ROTARY_BUTT_PRESS,	//< rotary button push event
	PUSH_BUTT_PRESS,	//< push button push event
	ALARM_TIME,			//< alarm event
	TIMER_ELAPSED,		//< timing elapsed event
	NO_EVENT			//< no event handling required
};

typedef enum return_values fsm_return_status_t; //< typedef return value

/** typedef for state event handler functions */
typedef fsm_return_status_t (*state_t)(fsm_t *, const event_t *);

struct fsm_s {
    state_t state; //< current state, pointer to event handler
	bool isAlarmEnabled; //< flag for the alarm status
    time_t timeSet; //< multi-purpose var for system or alarm time
};

struct event_s {
    uint8_t signal; //< identifies the type of event
};



/* dispatches events to state machine, called in application*/
inline static void fsm_dispatch(fsm_t * fsm, const event_t * event) {

    const static event_t entryEvent = {.signal = ENTRY};
    const static event_t exitEvent = {.signal = EXIT};

    state_t last_state = fsm->state;
    fsm_return_status_t r = fsm->state(fsm, event);

    if (r == RET_TRANSITION) {
        last_state(fsm, &exitEvent); //< call exit action of last state
        fsm->state(fsm, &entryEvent); //< call entry action of new state
    }
}


/* sets and calls initial state of state machine */
inline static void fsm_init(fsm_t * fsm, state_t init) {
    //... other initialization
    event_t entryEvent = {.signal = ENTRY};
    fsm->state = init;
    fsm->state(fsm, &entryEvent);
}


fsm_t AlarmFSM;

/*(INIT) STATE FUNCTION PREDECLARATION *************************************************/

/**
 * system time hour setter state-function, also init function for the finite-state machnie
 *
 * @param fsm	pointer to the finite-state machine variable containing the state information
 * 
 * @param event pointer to an event variable containing the actual event to be handled
 *
 * @return		fsm status about the outcome of the state execution
 */
fsm_return_status_t state_setSystemTimeHour(fsm_t * fsm, const event_t * event);



#endif  /* ALARM_FSM_H_ */