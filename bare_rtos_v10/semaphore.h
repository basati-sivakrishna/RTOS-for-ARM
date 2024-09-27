/*
 * semaphore.h
 *
 *  Created on: 13-Sep-2024
 *      Author: basat
 */

#ifndef SEMAPHORE_H_
#define SEMAPHORE_H_

#include<stdlib.h>
#include "kernel.h"

typedef struct sempahore{
    int value;
    TCB* semaphore_owner_task;
    TCB* blocked_task;
}*semaphore_ptr_t;

typedef enum semaphore_status{
    semaphore_status_released,
    semaphore_status_acquired,
    semaphore_status_error
}semaphore_status_e;

semaphore_ptr_t sem_uart0;

semaphore_ptr_t create_semaphore(int init_value);
void semaphore_acquire(semaphore_ptr_t semaphore);
semaphore_status_e semaphore_release(semaphore_ptr_t semaphore);


#endif /* SEMAPHORE_H_ */
