/*INCLUDES *******************************************************************/
#include <stdlib.h>

#include "ses_timer.h"
#include "ses_scheduler.h"
#include "util/atomic.h"
#include "ses_led.h"

/* MACROS *********************************************************/

#define SEC_2_MILLISEC      (uint16_t)1000                                  // 1s = 1000ms
#define MINUTE_2_MILLISEC   (uint32_t)(SEC_PER_MIN  * SEC_2_MILLISEC)       // 1min = 60 * 1000ms = 60000ms
#define HOUR_2_MILLISEC     (uint32_t)(MIN_PER_HOUR * MINUTE_2_MILLISEC)    // 1hour = 60 * 60 * 1000ms = 3600000ms

#define MILLISEC_PER_DAY    (uint32_t)(HOUR_PER_DAY * HOUR_2_MILLISEC)

/* PRIVATE VARIABLES *********************************************************/

/**
 * We make sure that the list is accessed only within atomic sections
 * protected by a memory barrier --> no volatile necessary
 */
static task_descriptor_t * taskList = NULL;

static volatile system_time_t curr_sys_time = 0;

/*FUNCTION DEFINITION *************************************************/

static void scheduler_update(void) {

    // Iterator pointer which points to the currently considered task
    task_descriptor_t * taskListIterator = taskList;
    
    while(taskListIterator != NULL){

        // 1ms elapsed -> decrease expire by 1
        taskListIterator->expire--;

        /* if expire reaches 0, the task must be executed 
        and the expire value must be reset */
        if(taskListIterator->expire == 0){
            taskListIterator->execute = true;
            taskListIterator->expire  = taskListIterator->period;
        }
        
        // Next iteration
        taskListIterator = taskListIterator->next;
    }

    // system time update
    curr_sys_time = (curr_sys_time >= MILLISEC_PER_DAY ) ? 0 : curr_sys_time + 1;

}

void scheduler_init() {

    timer0_start();
    timer0_setCallback(scheduler_update);
}

void scheduler_run() {

    // Iterator pointer which points to the currently considered task
    task_descriptor_t * taskListIterator = taskList;

    // Superloop
    while(1){

        // taskList iteration loop
        while(taskListIterator != NULL){
            
            if(taskListIterator->execute){
                /* If the considered task is candidate for execution
                then the corresponding task function will be called*/
                taskListIterator->task(taskListIterator->param);
                
                taskListIterator->execute = false;

                /* If the considered task must be performed only once (non-periodic task)
                then it can be removed here from the taskList*/
                if(taskListIterator->period == 0){
                    scheduler_remove(taskListIterator);
                }

            }

            // Next iteration
            taskListIterator = taskListIterator->next;
        }

        // taskListIterator "reset" to the first task element
        taskListIterator = taskList;
        
    }

}

bool scheduler_add(task_descriptor_t * toAdd) {
    // Check the parameter validity
    if(toAdd == NULL){
        return 0;
    }

    // This new task will be at the end of the taskList -> there is no "next" task in the list
    toAdd->next = NULL;
    // The new tast is not executed at this moment
    toAdd->execute = false;

    // Special case: there is no task in the taskList yet
    if(taskList == NULL){
        taskList = toAdd;
    }
    else{
        /* If there are some task in the list, then the end of the list must be found 
        using an iterator pointer which points to the currently considered task */
        task_descriptor_t * taskListIterator = taskList;

        while(taskListIterator->next != NULL){

            // Check is the new task already in the taskList or not
            if(toAdd == taskListIterator->next){
                return 0;
            }
            // Next iteration
            taskListIterator = taskListIterator->next;
        }
        
        /* Now the "taskListIterator" points to the last task
        The new task can be concatenated here */
        taskListIterator->next = toAdd;

    }

    return 1;

}

void scheduler_remove(const task_descriptor_t * toRemove) {
    
    // Check the parameter validity and that taskList is not empty 
    if(toRemove == NULL || taskList == NULL){
        return;
    }

    // Special case: the first list element is the one to be removed
    if(taskList == toRemove){
        // Delete the first task item from the list by "bypassing" 
        taskList = taskList->next;

    }
    else{
        // Search the task to be removed iterating through the taskList
        // Iterator pointer which points to the currently considered task
        task_descriptor_t * taskListIterator = taskList;

        while(taskListIterator->next != NULL){

            if(taskListIterator->next == toRemove){
                // Task to be removed is founded

                // Delete the founded task from the list by "bypassing"
                taskListIterator->next = taskListIterator->next->next;
                // Exit from the cycle
                break;
            }

            // Next iteration
            taskListIterator = taskListIterator->next;

        }

    }

    return;
}

system_time_t scheduler_getTime(void){
    return curr_sys_time;
}


void scheduler_setTime(system_time_t time){
    /* check the received time parameter 
        greater than MILLISEC_PER_DAY -> system_time will be initialized to 0
        otherwise system_time will be equal with the received time parameter 
    */
    curr_sys_time = (time >= MILLISEC_PER_DAY ) ? 0 : time;

}


system_time_t time_wrapper_2_system_time(time_t time){

    return (system_time_t)(time.hour * HOUR_2_MILLISEC + time.minute * MINUTE_2_MILLISEC + time.second * SEC_2_MILLISEC + time.milli);

}

time_t system_time_wrapper_2_time(system_time_t sys_time){
    time_t temp;

    // hour part:
    temp.hour = (uint32_t)sys_time / HOUR_2_MILLISEC;       // get the number of whole hours
    sys_time -= (uint32_t)temp.hour * HOUR_2_MILLISEC;      // substract from sys_time the whole hours

    // minute part:
    temp.minute = (uint32_t)sys_time / MINUTE_2_MILLISEC;   // get the number of whole minutes
    sys_time -= (uint32_t)temp.minute * MINUTE_2_MILLISEC;  // substract from sys_time the whole minutes

    // second part:
    temp.second = (uint32_t)sys_time / SEC_2_MILLISEC;      // get the number of whole seconds
    sys_time -= (uint32_t)temp.second * SEC_2_MILLISEC;     // substract from sys_time the whole seconds

    // remaining part is time in millisec
    temp.milli = sys_time;

    return temp;
}