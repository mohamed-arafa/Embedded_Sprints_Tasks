/*
 * FreeRTOS Kernel V10.2.0
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/* 
	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used.
*/


/*
 * Creates all the demo application tasks, then starts the scheduler.  The WEB
 * documentation provides more details of the demo application tasks.
 * 
 * Main.c also creates a task called "Check".  This only executes every three 
 * seconds but has the highest priority so is guaranteed to get processor time.  
 * Its main function is to check that all the other tasks are still operational.
 * Each task (other than the "flash" tasks) maintains a unique count that is 
 * incremented each time the task successfully completes its function.  Should 
 * any error occur within such a task the count is permanently halted.  The 
 * check task inspects the count of each task to ensure it has changed since
 * the last time the check task executed.  If all the count variables have 
 * changed all the tasks are still executing error free, and the check task
 * toggles the onboard LED.  Should any task contain an error at any time 
 * the LED toggle rate will change from 3 seconds to 500ms.
 *
 */

/* Standard includes. */
#include <stdlib.h>
#include <stdio.h>

/* Scheduler includes. */

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"

#include "task.h"
#include "lpc21xx.h"
#include "semphr.h"
/* Peripheral includes. */
#include "serial.h"
#include "GPIO.h"
 #include "event_groups.h"
 
/*-----------------------------------------------------------*/

/* Constants to setup I/O and processor. */
#define mainBUS_CLK_FULL	( ( unsigned char ) 0x01 )

/* Constants for the ComTest demo application tasks. */
#define mainCOM_TEST_BAUD_RATE	( ( unsigned long ) 115200 )


/*
 * Configure the processor for use with the Keil demo board.  This is very
 * minimal as most of the setup is managed by the settings in the project
 * file.
 */
static void prvSetupHardware( void );
/*-----------------------------------------------------------*/


int LED_state= PIN_IS_LOW;
int counter=0;

SemaphoreHandle_t Toggle_LED;
EventGroupHandle_t Toggle_Event;



void button_tracker( void *pvParameters )
{
	
		
	while(1)
	{
		if(GPIO_read(PORT_0,PIN0)==PIN_IS_HIGH)
		{	
			counter++;
		}	
		else if(counter !=0 && GPIO_read(PORT_0,PIN0)==PIN_IS_LOW)
		{	
			// here indicates a falling edge 
			counter = 0;

			//give set bit 0 in the event group
			xEventGroupSetBits(Toggle_Event,(1<<0));
		}
		else 
		{
			//do nothing
		}
		
		vTaskDelay (pdMS_TO_TICKS(50));
	}
	
}

void LED_Toggle(void* pvParameters)
{
	while(1)
	{
		// read the EventGroup bits, 
		// if there is new event will toggle LED and clear the event flag. 
		// but if there is no event this task will be blocked until there is an new flag.

		int temp= xEventGroupWaitBits(Toggle_Event,1,pdTRUE,pdFALSE,10000);

		GPIO_write(PORT_0,PIN1,(LED_state^0X01));
		LED_state= LED_state^0X01;		// toggle the led state
		vTaskDelay(pdMS_TO_TICKS(50));
	}
	
}



/*
// succeded to toggle the led using semaphore :) @_@


void button_tracker( void *pvParameters )
{
	
	while(1)
	{
		//take led toggle semaphore
		xSemaphoreTake(Toggle_LED,0);
		if(GPIO_read(PORT_0,PIN0)==PIN_IS_HIGH)
			counter++;
		else if(counter !=0 && GPIO_read(PORT_0,PIN0)==PIN_IS_LOW)
		{
			counter = 0;
			//give LED_togel semaphore
			xSemaphoreGive(Toggle_LED);
		}
		else 
		{
			//do nothing
		}
		
		vTaskDelay (pdMS_TO_TICKS(50));
	}
	
}

void LED_Toggle(void* pvParameters)
{
	while(1)
	{
		// take semaphore
		xSemaphoreTake(Toggle_LED,50000);

		GPIO_write(PORT_0,PIN1,(LED_state^0X01));
		LED_state= LED_state^0X01;

		//give semaphore
		xSemaphoreGive(Toggle_LED);
		vTaskDelay(pdMS_TO_TICKS(50));
	}
	
}
*/




/*######################################################################################################################*/
/*######################################################################################################################*/
/*
 * Application entry point:
 * Starts all the other tasks, then starts the scheduler. 
 */
 
int main( void )
{
	GPIO_write(PORT_0,1,0);
	LED_state=0;
	prvSetupHardware();
	//Toggle_LED=xSemaphoreCreateBinary();
	Toggle_Event = xEventGroupCreate();
	xTaskCreate( LED_Toggle, /* Pointer to the function that implements the task. */
							 "led toggling",/* Text name for the task. This is to facilitate debugging only. */
							 100, /* Stack depth - small microcontrollers will use much less stack than this. */
							 NULL, /* This example does not use the task parameter. */
							 1, 		/* This task will run at priority 1. */
							 NULL ); /* This example does not use the task handle. */
						
	xTaskCreate( button_tracker, /* Pointer to the function that implements the task. */
							 "button tracker",/* Text name for the task. This is to facilitate debugging only. */
							 100, /* Stack depth - small microcontrollers will use much less stack than this. */
							 NULL, /* This example does not use the task parameter. */
							 2, 		/* This task will run at priority 1. */
							 NULL ); /* This example does not use the task handle. */							

	vTaskStartScheduler();

	for( ;; );
}
/*-----------------------------------------------------------*/
/*######################################################################################################################################*/
/*######################################################################################################################################*/
static void prvSetupHardware( void )
{
	/* Perform the hardware setup required.  This is minimal as most of the
	setup is managed by the settings in the project file. */

	/* Configure UART */
	xSerialPortInitMinimal(mainCOM_TEST_BAUD_RATE);

	/* Configure GPIO */
	GPIO_init();

	/* Setup the peripheral bus to be the same as the PLL output. */
	VPBDIV = mainBUS_CLK_FULL;
}
/*-----------------------------------------------------------*/


