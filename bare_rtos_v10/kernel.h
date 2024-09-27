/*
 * kernel.h
 *
 *  Created on: 09-Sep-2024
 *      Author: basat
 */

#ifndef KERNEL_H_
#define KERNEL_H_
#include <stdint.h>


#define STACK_SIZE 200
#define TASKS_NUM 10
#define STACK_ALLIGN int32_t
#define XPSR_OFFSET          1
#define PC_OFFSET            2
#define LR_OFFSET            3
#define R12_OFFSET           4
#define R3_OFFSET            5
#define R2_OFFSET            6
#define R1_OFFSET            7
#define R0_OFFSET            8
#define R11_OFFSET           9
#define R10_OFFSET           10
#define R9_OFFSET            11
#define R8_OFFSET            12
#define R7_OFFSET            13
#define R6_OFFSET            14
#define R5_OFFSET            15
#define R4_OFFSET            16
#define KERNEL_STACK_SIZE 100
#define ICSR 0xE000ED04

#define max_priority 9

typedef int32_t* StackPtr;
typedef void *    pVoid;
typedef void (*P2FUNC)(void);

typedef enum task_state{
    READY = 0,
    RUNNING,
    BLOCKED
//    BLOCKED_FOR_SEMAPHORE,
//    BLOCKED_FOR_BOTH
//    BLOCKED_FOR_SEMAPHORE
}task_state_e;


typedef struct TCB
{
    uint8_t ID;
    uint8_t Priority;
    P2FUNC  TaskCode;

    /*-----Stack pointers-------*/
    StackPtr Sptr;
/*
 * Dont change the sequence of the elements in TCB
 * In pendsv handler I am restoring stack  by incrementig the TCB* by 8 bytes
 */
    task_state_e State;
    int delay_value;
    int c_time;
    /*-----Memory Pointer-------*/
    StackPtr TopStack;
    StackPtr EndStack;

    /*-----Pointer to Next and Previous TCB----*/
    struct TCB *Next_Task;

    /*-----Pointer to Current Queue---*/
    pVoid CurrQueue;

}TCB;
typedef TCB* Task_ptr_handle_t;


typedef struct
{
    TCB * Front,*Rear;
    int32_t No_Tasks;
}TCBLinkedList;


 TCBLinkedList ready_list;
 TCBLinkedList block_list;
  TCB * pCurrentTask;
  TCB * pNextTask;
  TCB* backup_task;

 struct queue* head_node;



 Task_ptr_handle_t bare_rtos_Addtask(P2FUNC TaskCode,uint8_t ID,uint8_t Priority,uint32_t StackSize, uint32_t task_time);
 void bare_rtos_add_to_list(TCBLinkedList * Queue,TCB* Task_n);
 void bare_rtos_delete_from_list(TCBLinkedList* queue, TCB* Task_n);
 void bare_rtos_stack_init(uint32_t StackSize,StackPtr Stack,TCB *Task,uint8_t Flag);

 void scheduler(void);
 void launch_os(void);

 void user_mode(void);
 void ps_delay(int n);
extern void task_unblock(Task_ptr_handle_t task_handle);
extern void task_block(Task_ptr_handle_t task_handle);
extern void task_yield(void);

#endif /* KERNEL_H_ */
