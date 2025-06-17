#ifndef SES_FSM_H_
#define SES_FSM_H_


#include "ses_scheduler.h"

typedef struct fsm_s fsm_t; //< typedef for alarm clock state machine
struct fsm_s {
    state_t state; //< current state, pointer to event handler
    bool isAlarmEnabled; //< flag for the alarm status
    time_t timeSet; //< multi-purpose var for system or alarm time
};

typedef struct event_s event_t; //< event type for alarm clock fsm
struct event_s {
    uint8_t signal; //< identifies the type of event
};

/** return values */
enum return_values {
    RET_HANDLED, //< event was handled
    RET_IGNORED, //< event was ignored; not used in this implementation
    RET_TRANSITION //< event was handled and a state transition occurred
};

typedef enum return_values fsm_return_status_t; //< typedef return value

/** typedef for state event handler functions */
typedef fsm_return_status_t (*state_t)(fsm_t *, const event_t *);

enum events_list {
    ENTRY,  // state entry event
    EXIT,   // state exit event
    PUSHBUTTON_PRESSED, // push button pressed event, 
    ROTARYBUTTON_PRESSED, // rortary button pressed,
    ALARM_TRIGGERED, // system time reached alarm time
    TIMEOUT     // defined time out (5 sec)
};


/* dispatches events to state machine, called in application*/
inline static void fsm_dispatch(fsm_t * fsm, const event_t * event) {
    const static event_t entryEvent = {.signal = ENTRY};
    const static event_t exitEvent = {.signal = EXIT};
    state_t s = fsm->state;
    fsm_return_status_t r = fsm->state(fsm, event);
    if (r == RET_TRANSITION) {
    s(fsm, &exitEvent); //< call exit action of last state
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

#endif