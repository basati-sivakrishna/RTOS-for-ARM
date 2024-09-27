/*
 * semaphore.c
 *
 *  Created on: 13-Sep-2024
 *      Author: basat
 */
#include "semaphore.h"
#include <stdlib.h>
#include <./inc/tm4c123gh6pm.h>

semaphore_ptr_t create_semaphore(int init_value)
{
    semaphore_ptr_t ret_semaphore = (semaphore_ptr_t)malloc(sizeof(semaphore_ptr_t));
    ret_semaphore->value = init_value;
    return ret_semaphore;
}

void semaphore_acquire(semaphore_ptr_t semaphore)
{
    while(semaphore->value <=0)
        {
        ;
        }
//    if(semaphore->value <= 0)
//    {
//        //block the task
//        semaphore->blocked_task = pCurrentTask;
//        task_block(pCurrentTask);
//        NVIC_INT_CTRL_R |=  NVIC_INT_CTRL_PEND_SV;
//        return;
//    }
    __asm("CPSID I\n");
    semaphore->value  = 0;
    semaphore->semaphore_owner_task = pCurrentTask;
    __asm("CPSIE I\n");

}

semaphore_status_e semaphore_release(semaphore_ptr_t semaphore)
{
    if(semaphore->semaphore_owner_task == pCurrentTask)
    {
        __asm("CPSID I\n");

    semaphore->value  = 1;
//    task_unblock(semaphore->blocked_task);
//    NVIC_INT_CTRL_R |=  NVIC_INT_CTRL_PEND_SV;
    __asm("CPSIE I\n");

    return semaphore_status_released;
    }
    return semaphore_status_error;
}



