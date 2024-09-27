
/*
 * This version has priority scheduling algo
 * Task blocking using psdelay function is done
 *  semaphore will be implemeted
 * no queue enhancement
 */

/*
 * %%%%%%%%% PROJECT NAME -> DEVOLOPING RTOS BASED ENVIRONMENT FOR TIVA TM4C123GH6PM
 * PROJECT MEMBERS -> BASATI SIVAKRISHNA
 */

#include <./inc/tm4c123gh6pm.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <inc/hw_memmap.h>
#include <inc/hw_types.h>
#include <driverlib/gpio.h>
#include <driverlib/pin_map.h>
#include <driverlib/sysctl.h>
#include <driverlib/uart.h>
#include <driverlib/systick.h>
#include <driverlib/interrupt.h>
#include <bare_rtos_queue.h>
#include "kernel.h"
#include "semaphore.h"

/*
 * Task blocking and unblocking with power saving delay working fine
 * Atleast one running task should be there in the ready_list
 * Semaphore yet to implement
 */
/*
 * check_program variable is used to check the features developed

 * check_program = 1 -> basic task switching (with preemptive scheduling algorithm) //debug
 * check_program = 2 -> Intra task communication (Queue)  //debug
 * check_program = 3 -> power saving delay function(ps_delay)  //serial monitor
 * check_program = 4 -> Co-operative scheduling algorithm //debug
 * check_program = 5 -> Custom time period algorithm      //debug

 */

#define check_program 1

int32_t     KernelStack[KERNEL_STACK_SIZE];
int32_t    *kernelStackPtr = &KernelStack[KERNEL_STACK_SIZE -1 ];

Task_ptr_handle_t task_handle_1;
Task_ptr_handle_t task_handle_2;
Task_ptr_handle_t task_handle_3;
Task_ptr_handle_t task_handle_4;
Task_ptr_handle_t task_handle_5;
Task_ptr_handle_t task_handle_6;
Task_ptr_handle_t task_handle_7;


long long int task_1_var = 0, task_2_var = 0, task_3_var = 0;
int queue_var_3 = 0;
int queue_var_2 = 0;
int queue_var_1 = 0;
void printstring(char * str);

void print_error( char* msg,  char *file, int line)
{
    printstring("\x1b[31m" "Error in -> File name: " "\x1b[0m");
    printstring("\x1b[31m");
    printstring(file);
    printstring("\x1b[0m\t");
    printstring("\x1b[31m Line number: ");
    char line_num[4];
    sprintf(line_num, "%d", line);
    printstring(line_num);
    printstring("\x1b[0m");
    printstring("\n\r");
    while(1);

}

void task_unblock(Task_ptr_handle_t task_handle)
{
__asm("CPSID I\n");
    task_handle->State = READY;
    bare_rtos_delete_from_list(&block_list, task_handle);
    bare_rtos_add_to_list(&ready_list, task_handle);
__asm("CPSIE I\n");
}

void task_block(Task_ptr_handle_t task_handle)
{
__asm("CPSID I\n");
task_handle->State = BLOCKED;
backup_task = task_handle->Next_Task;
bare_rtos_delete_from_list(&ready_list, task_handle);
bare_rtos_add_to_list(&block_list, task_handle);
__asm("CPSIE I\n");
}



void task_yield(void)
{
       NVIC_INT_CTRL_R |=  NVIC_INT_CTRL_PEND_SV;
       __asm("CPSID I\n");
}

void delay_ms( int n)
{
    volatile int i, j ;

    for(i = 0; i < n ; i ++)
    {
        for(j = 0; j < 3180; j++);
    }
}


void UART0_Transmitter(unsigned char data)
{
    while((UART0_FR_R & (1<<5)) != 0); /* wait until Tx buffer not full */
    UART0_DR_R = data;                  /* before giving it another byte */
}

void printstring(char *str)
{
  while(*str)
    {
        UART0_Transmitter(*(str++));
    }
}

void uart_init(void)
{
         SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
         SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
         GPIOPinConfigure(GPIO_PA0_U0RX);
         GPIOPinConfigure(GPIO_PA1_U0TX);
         GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
         UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
         printstring("Initialization done\n\r");
}

void task_0(void)
{

    while(1)
    {
        delay_ms(10);
    }
}

void task_1(void)
{
        SYSCTL_RCGCGPIO_R |= (1<<4);
        GPIO_PORTF_DIR_R |= (1 << 1) | (1 << 2);
        GPIO_PORTF_DEN_R |= (1 << 1) | (1 << 2);
        GPIO_PORTF_DATA_R |= (1<<1) | (1 << 2);
    while(1)
           {
        semaphore_acquire(sem_uart0);
        printstring("\x1b[33m""#################################### TASK-01 ################################################# \n\r"  "\x1b[0m");
        semaphore_release(sem_uart0);
        GPIO_PORTF_DATA_R ^= ((1<<1) |(1<<2));
        ps_delay(1000);
           }
}

void task_2(void)
{
    while(1)
    {
        semaphore_acquire(sem_uart0);
        printstring("\x1b[31m" "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ TASK-02 @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ \n\r" "\x1b[0m");
        semaphore_release(sem_uart0);
        ps_delay(1000);
     }
}

void task_3(void)
{
    while(1)
    {
        semaphore_acquire(sem_uart0);
        printstring("\x1b[35m" "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% TASK-03 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n\r" "\x1b[0m");
        semaphore_release(sem_uart0);
        ps_delay(1000);
     }
}

void task_4(void)
{
    while(1)
    {
        delay_ms(10); //dummy task
     }
}

void task_5(void)
{
    while(1)
    {
        printstring("\x1b[35m" "Task-05\n\r" "\x1b[0m");
        delay_ms(1000);
    }
}

void task_6(void)
{
    while(1)
    {
          printstring("task_06\n\r");
          delay_ms(500);
    }
}


void systic_setup(void)
{
    SysTickPeriodSet(16000-1);

#if(check_program != 4)
    SysTickIntEnable();
    SysTickEnable();
#endif
}

int main (void)
{
    uart_init();
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, 0X0E);
GPIO_PORTF_DATA_R = 0X00;
#if(check_program == 2)

    head_node = bare_rtos_queue_create(10);
    head_node->element = 10;
    bare_rtos_queue_insert(30, head_node);
    bare_rtos_queue_insert(40, head_node);
    bare_rtos_queue_insert(60, head_node);
    bare_rtos_queue_insert(70, head_node);
    bare_rtos_queue_insert(80, head_node);
//
    int get_val = 0;
    get_val = bare_rtos_queue_get(2, head_node);
    bare_rtos_queue_insert_indx(23, 2, head_node);
#endif

    bare_rtos_Addtask(task_0, 99, 10, 50, 1);
    task_handle_1 = bare_rtos_Addtask(task_1, 2, 1, 500, 2);
    task_handle_2 = bare_rtos_Addtask(task_2, 3, 1, 500, 4);
    task_handle_3 = bare_rtos_Addtask(task_3, 4, 1, 500, 6);
    task_handle_4 = bare_rtos_Addtask(task_4, 5, 1, 500, 8);
//    task_handle_5 = bare_rtos_Addtask(task_5, 6, 1, 500, 8);
//    task_handle_1 = bare_rtos_Addtask(task_6, 7, 7, 500, 8);
//    task_handle_1 = bare_rtos_Addtask(task_6, 7, 8, 500, 8);

    sem_uart0 = create_semaphore(1);

    __asm("CPSID I\n");
    systic_setup();
    user_mode();
    launch_os();
    while(1)
    {

    }

return 0;
}
