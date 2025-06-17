/*INCLUDES *******************************************************************/
#include <stdlib.h>

#include "ses_timer.h"
#include "ses_scheduler.h"
#include "util/atomic.h"
#include "ses_led.h"

/* PRIVATE VARIABLES *********************************************************/

/**
 * We make sure that the list is accessed only within atomic sections
 * protected by a memory barrier --> no volatile necessary
 */
static task_descriptor_t * taskList = NULL;


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




