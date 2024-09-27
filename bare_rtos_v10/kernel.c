/*
 * kernel.c
 *
 *  Created on: 09-Sep-2024
 *      Author: basat
 */
#include<kernel.h>
#include<stdlib.h>
#include<stdint.h>
#include <./inc/tm4c123gh6pm.h>

extern void print_error(char*, char*, int);


Task_ptr_handle_t bare_rtos_Addtask(P2FUNC TaskCode,uint8_t ID,uint8_t Priority,uint32_t StackSize, uint32_t task_time)
{
    if(ready_list.No_Tasks < TASKS_NUM )
        {
            TCB *TaskN = (TCB*) malloc(1 * sizeof(TCB));
            StackPtr TaskStack = (StackPtr) malloc(StackSize * sizeof(int));

            if (TaskN != NULL)
            {
                TaskN->ID = ID;
                if(Priority > max_priority || Priority < 0)
                    Priority = max_priority;
                TaskN->Priority = Priority;
                TaskN->TaskCode = TaskCode;
                TaskN->CurrQueue = &ready_list;
                TaskN->State = READY;
                TaskN->delay_value = 0;
                TaskN->c_time = 1;
                #if(check_program == 5)
                TaskN->c_time = task_time;
                #endif
                bare_rtos_add_to_list(&ready_list, TaskN);
                bare_rtos_stack_init(StackSize, TaskStack, TaskN, 0);
            }
            return TaskN;
        }
    else
    {
        print_error("More tasks", __FILE__, __LINE__);
    }
    return NULL;

}


void bare_rtos_add_to_list(TCBLinkedList* Queue,TCB *Task_n)
{
    if(Task_n != NULL)
    {
        /*-------Insert New Element----*/
        if(Queue->No_Tasks == 0)
        {
            Queue->Front = Task_n;
            Queue->Rear  = Task_n;
            Task_n->Next_Task = NULL;
        }
        else
        {
            TCB* curr = Queue->Front;
            TCB* prev = curr;
            while(curr && curr->Priority <= Task_n->Priority)
            {
                prev = curr;
                curr = curr->Next_Task;
            }
            if(curr == Queue->Front)
            {
                Queue->Front = Task_n;
                Task_n->Next_Task = curr;
            }
            else
            {
                prev->Next_Task = Task_n;
                Task_n->Next_Task = curr;
            }
        }
        Queue->No_Tasks++;
    }
    else
        print_error("NULL POINTER\n", __FILE__, __LINE__);
}

void bare_rtos_delete_from_list(TCBLinkedList* queue, TCB* Task_n)
{
    if(Task_n != NULL && queue != NULL)
    {
        TCB* curr = queue->Front;
        TCB* prev = curr;
        while(curr && curr != Task_n)
        {
            prev = curr;
            curr = curr->Next_Task;
        }
        if(!curr)  print_error("Trying to delete wrong task\n", __FILE__, __LINE__);
        if(prev == curr)
            queue->Front = curr->Next_Task;
        else
        prev->Next_Task = curr->Next_Task;

//        free(curr);
        queue->No_Tasks--;
    }
    else
        print_error("NULL POINTER\n", __FILE__, __LINE__);

}


void bare_rtos_stack_init(uint32_t StackSize,StackPtr Stack,TCB *Task,uint8_t Flag)
{
    /*----------Init Stack Pointer to point to block below registers-----*/
    Task->Sptr =(int32_t*) (&Stack[STACK_SIZE - R4_OFFSET]);
    Task->TopStack = Stack + StackSize;
    Task->EndStack = Stack;

    /*--------Set T bit to 1------*/
    Stack[STACK_SIZE - XPSR_OFFSET] = (1<<24);

    /*-------Program Counter initialization---*/
    Stack[STACK_SIZE - PC_OFFSET] =(uint32_t) (Task->TaskCode);

    /*--------Init R0->R12-----*/
    Stack[STACK_SIZE - LR_OFFSET] = 0xAAAAAAAA;
    Stack[STACK_SIZE - R12_OFFSET] = 0xAAAAAAAA;
    Stack[STACK_SIZE - R3_OFFSET] = 0xAAAAAAAA;
    Stack[STACK_SIZE - R2_OFFSET] = 0xAAAAAAAA;
    Stack[STACK_SIZE - R1_OFFSET] = 0xAAAAAAAA;
    Stack[STACK_SIZE - R0_OFFSET] = 0xAAAAAAAA;
    Stack[STACK_SIZE - R11_OFFSET] = 0xAAAAAAAA;
    Stack[STACK_SIZE - R10_OFFSET] = 0xAAAAAAAA;
    Stack[STACK_SIZE - R9_OFFSET] = 0xAAAAAAAA;
    Stack[STACK_SIZE - R8_OFFSET] = 0xAAAAAAAA;
    Stack[STACK_SIZE - R7_OFFSET] = 0xAAAAAAAA;
    Stack[STACK_SIZE - R6_OFFSET] = 0xAAAAAAAA;
    Stack[STACK_SIZE - R5_OFFSET] = 0xAAAAAAAA;
    Stack[STACK_SIZE - R4_OFFSET] = 0xAAAAAAAA;
}



void scheduler(void)
{
    if(pCurrentTask->ID == 99)
        return;
    if(pCurrentTask->State == BLOCKED)
    {
        pCurrentTask = backup_task;
        if(pCurrentTask->ID == 99)
          pCurrentTask = ready_list.Front;
        pNextTask = pCurrentTask;
        return;
    }
    volatile uint8_t curr_task_priority = pCurrentTask->Priority;
    pCurrentTask = pCurrentTask->Next_Task;
    if(pCurrentTask->ID == 99)
      pCurrentTask = ready_list.Front;
    while(curr_task_priority < pCurrentTask->Priority)
    {
        pCurrentTask = pCurrentTask->Next_Task;
        if(pCurrentTask->ID == 99)
          pCurrentTask = ready_list.Front;
    }
    pNextTask = pCurrentTask;


#if(check_program == 5)
    NVIC_ST_CTRL_R &= ~(NVIC_ST_CTRL_ENABLE);
    if(pCurrentTask->Next_Task->ID != 99)
        NVIC_ST_RELOAD_R = (((pCurrentTask->Next_Task->c_time)*16000)) - 1;
    else
        NVIC_ST_RELOAD_R = (((ready_list.Front->c_time)*16000)) - 1;

    NVIC_ST_CTRL_R |= NVIC_ST_CTRL_CLK_SRC | NVIC_ST_CTRL_ENABLE;
#endif
}


void __attribute__((naked)) PendSV_Handler(void)
{
    /*------Context Switch-----*/
                /*3] Save R4-R11 to Stack*/
                /*4] Save new Sp to Stack Pointer in TCB*/
                /*------Before Pushing to Stack Set MSP to PSP Location----*/
                __asm volatile ("MRS R0,PSP");
                __asm volatile ("MOV SP,R0");
                /*-----Push To Task Stack-------*/
                __asm volatile ("PUSH {R4-R11}");
                __asm volatile ("LDR R0, =pCurrentTask");
                __asm volatile ("LDR R1,[R0]");
                __asm volatile ("STR SP,[R1,#8]");

            /*-----Switch To Kernel Stack----*/
            __asm volatile ("LDR R1,=kernelStackPtr");
            __asm volatile ("LDR SP,[R1]");

            __asm volatile ("PUSH {R0,LR}");
            //-----Recall stack frame of function is destroyed after function call i.e SP points to same location before func execution
            __asm volatile ("BL  scheduler");
            __asm volatile ("POP  {R0,LR}");
            /*------Context Switch of Previous Task-----*/
           /*1] Save R4-R11 to Stack*/
           /*2] Save new Sp to Stack Pointer in TCB*/
           /*--------Context Restore of Next Task------*/
           __asm volatile ("LDR R0, =pNextTask");
           __asm volatile ("LDR R1,[R0]");
           __asm volatile ("LDR SP,[R1,#8]");
           __asm volatile ("POP {R4-R11}");
           __asm volatile ("MRS R0 , MSP");
           __asm volatile ("MSR PSP, R0");
           __asm volatile ("BX LR");
}

__attribute__((naked))void SysTick_Handler(void)
{
    TCB* temp = block_list.Front;
    while(temp)
    {
        if(temp->delay_value > 0)
        {
            temp->delay_value--;
            if(temp->delay_value == 0)
            {
                __asm volatile ("PUSH {R0,LR}");
                task_unblock(temp);
                __asm volatile ("POP  {R0,LR}");
            }
        }
        temp = temp->Next_Task;
    }
    NVIC_INT_CTRL_R |=  NVIC_INT_CTRL_PEND_SV;
    __asm volatile("BX LR");
}


void user_mode(void)
{
    __asm volatile("LDR R0,=kernelStackPtr");
    __asm volatile("LDR R1,[R0]");
    __asm volatile("MSR MSP ,R1");
    pCurrentTask  = ready_list.Front;
    /*-----Set Stack Pointer to PSP---*/
    __asm volatile("MRS R0 , CONTROL");
    __asm volatile("ORR R0,R0,#2");
    __asm volatile("MSR CONTROL,R0");
}

void launch_os(void)
{
    /*----Disable Global Interrupts----*/
    __asm("CPSID I\n");
    //Interrupt disable using asm
    /*--------Load Address of the Current TCB Into R0-----*/
    __asm volatile ("LDR R0, =pCurrentTask");
    /*--------Helping Variables-----*/
    __asm volatile ("LDR R2 ,[R0]");
    __asm volatile ("LDR SP,[R2,#8]");
    /*-------Restore Registers-----*/
    __asm volatile ("POP {R4-R11}");
    __asm volatile ("POP {R0-R3}");
    __asm volatile ("POP {R12}");
    /*-----Skip LR-----*/
    __asm volatile ("ADD SP,SP,#4");
    /*----Create New Start Location----*/
    __asm volatile ("POP {LR}");
    __asm volatile ("ADD SP,SP,#4");
    /*----Restore Stack Pointer----*/
    __asm("CPSIE I\n");                  //Interrupt enable using asm
    __asm volatile ("BX LR");

}


void ps_delay(int n)
{
    pCurrentTask->delay_value = n;
    task_block(pCurrentTask);
    NVIC_INT_CTRL_R |=  NVIC_INT_CTRL_PEND_SV;
}



