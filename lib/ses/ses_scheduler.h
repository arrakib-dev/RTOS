#ifndef SCHEDULER_H_
#define SCHEDULER_H_

/* INCLUDES *****************************************************************/
#include <stdbool.h>
#include <stdint.h>

/* MACROS *********************************************************/
#define HOUR_PER_DAY        24
#define MIN_PER_HOUR        60
#define SEC_PER_MIN         60

/* TYPES ********************************************************************/

/**
 * Type of function pointer for tasks
 */
typedef void (* task_t)(void *);

/**
 * Task structure to schedule tasks
 */
typedef struct task_descriptor_s {
   task_t task;          ///< function pointer to call
   void *  param;        ///< pointer, which is passed to task when executed
   uint16_t expire;      ///< time offset in ms, after which to call the task
   uint16_t period;      ///< period of the timer after firing; 0 means exec once
   uint8_t execute:1;    ///< for internal use
   uint8_t reserved:7;   ///< reserved
   struct task_descriptor_s * next; ///< pointer to next task, internal use
} task_descriptor_t;

/**
 * type for tracking the system time in ms
 */
typedef uint32_t system_time_t;

/**
 * Human readable format for system time
 */
typedef struct {
   uint8_t hour;
   uint8_t minute;
   uint8_t second;
   uint16_t milli;
} time_t;


/* FUNCTION PROTOTYPES *******************************************************/

/**
 * Initializes the task scheduler. Uses hardware timer2 of the AVR.
 */
void scheduler_init(void);

/**
 * Runs scheduler in an infinite loop.
 */
void scheduler_run(void);

/**
 * Adds a new task to the scheduler.
 *
 * @param td   Pointer to taskDescriptor structure. The scheduler takes
 *             possesion of the memory pointed at by td until the task
 *             is removed by scheduler_remove or a non-periodic task is
 *             executed. td->expire and td->execute are written to by
 *             the task scheduler
 *
 * @return     false, if task is already present or invalid (NULL)
 *             true, if task was successfully added to scheduler and will be
 *             scheduled after td->expire ms
 */
bool scheduler_add(task_descriptor_t * td);

/**
 * Removes a timer task from the scheduler.
 *
 * @param td	pointer to task descriptor to remove
 * */
void scheduler_remove(const task_descriptor_t * td);

/**
 * Gets the current system time with 1ms resolution
 *
 * @return  current system time
 * */
system_time_t scheduler_getTime(void);

/**
 * Redefines the current system time 
 *
 * @param time system time format time definition
 * */
void scheduler_setTime(system_time_t time);

/**
 * Changes the human readable time format to system time format
 *
 * @param time human readable time format
 * 
 * @return human readable time format converted to system time format
 * */
system_time_t time_wrapper_2_system_time(time_t time);

/**
 * Changes the system time format to human readable time format
 *
 * @param time system time format
 * 
 * @return system time format converted to human readable time format
 * */
time_t system_time_wrapper_2_time(system_time_t sys_time);

#endif /* SCHEDULER_H_ */
