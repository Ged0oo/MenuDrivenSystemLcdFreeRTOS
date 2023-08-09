/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/

#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "LCD_interface.h"
#include "queue.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define STAGE_ONE	1
#define STAGE_TWO	2

#define FIRST_FRAME         	1
#define SECOND_FRAME         	2
#define THIRD_FRAME       		3
#define FOURTH_FRAME        	4
#define FIFTH_FRAME        		5

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

extern GPIO_ConfigType DecrementButton;
extern GPIO_ConfigType ChoiceButton;
extern GPIO_ConfigType IncrementButton;

uint8_t CurrentFrame = FIRST_FRAME;

uint8_t *FirstScreenFrames[6][2] =
{
		{"**  Mood One  **",	"*  Temprature  *"},
		{"**  Mood Two  **",	"* AirCondition *"},
		{"*  Mood Three  *",	"**  Pressure  **"},
		{"*  Mood  Four  *",	"**  Humadity  **"},
		{"*  Mood  Five  *",	"* GasDetection *"}
};

uint8_t *ScoundScreenFrames[6][2] =
{
		{"Temperature Mode",	"Temperature => *"},
		{"AirConditionMode",	"AirCondition=>**"},
		{" Pressure  Mode ",	"Pressure => ****"},
		{"* HumadityMood *",	"Humadity => ****"},
		{"**  Gas Mood  **",	"GasDetecte => *"}
};

/* USER CODE END Variables */
/* Definitions for task1 */
osThreadId_t task1Handle;
const osThreadAttr_t task1_attributes = {
  .name = "task1",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for task2 */
osThreadId_t task2Handle;
const osThreadAttr_t task2_attributes = {
  .name = "task2",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal1,
};
/* Definitions for myQueue01 */
osMessageQueueId_t myQueue01Handle;
const osMessageQueueAttr_t myQueue01_attributes = {
  .name = "myQueue01"
};
/* Definitions for myBinarySem01 */
osSemaphoreId_t myBinarySem01Handle;
const osSemaphoreAttr_t myBinarySem01_attributes = {
  .name = "myBinarySem01"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

void LcdSetFrame(uint8_t stage, uint8_t frame);

/* USER CODE END FunctionPrototypes */

void vTaskOne(void *argument);
void vTaskTwo(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of myBinarySem01 */
  myBinarySem01Handle = osSemaphoreNew(1, 0, &myBinarySem01_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of myQueue01 */
  myQueue01Handle = osMessageQueueNew (6, sizeof(uint8_t), &myQueue01_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */

  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of task1 */
  task1Handle = osThreadNew(vTaskOne, NULL, &task1_attributes);

  /* creation of task2 */
  task2Handle = osThreadNew(vTaskTwo, NULL, &task2_attributes);

  /* USER CODE BEGIN RTOS_THREADS */



  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_vTaskOne */
/**
  * @brief  Function implementing the task1 thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_vTaskOne */
void vTaskOne(void *argument)
{
  /* USER CODE BEGIN vTaskOne */

	BaseType_t RetVal = pdTRUE; //has the status of sending message
	uint8_t IncrementButtonState = 0; //has the status of the increment Button
	uint8_t DecrementButtonState = 0; //has the status of the decrement Button
	uint8_t ChoiceButtonState = 0;								 //has the status if context to secound task or no

  /* Infinite loop */
	for(;;)
	{
		LcdSetFrame(STAGE_ONE, CurrentFrame);

		ChoiceButtonState = MGPIO_u8ReadPortPin(&ChoiceButton);
		IncrementButtonState = MGPIO_u8ReadPortPin(&IncrementButton);
		DecrementButtonState = MGPIO_u8ReadPortPin(&DecrementButton);

		if(GPIO_HIGH == IncrementButtonState)
		{
			if(CurrentFrame == FIFTH_FRAME) CurrentFrame = FIRST_FRAME;
			else CurrentFrame++;
		}
		if(GPIO_HIGH == DecrementButtonState)
		{
			if(CurrentFrame == FIRST_FRAME) CurrentFrame = FIFTH_FRAME;
			else CurrentFrame-- ;
		}
		if(GPIO_HIGH == ChoiceButtonState)
		{
			RetVal = xQueueSendToFront(myQueue01Handle, (void *)&CurrentFrame, HAL_MAX_DELAY);
			osSemaphoreAcquire(myBinarySem01Handle, HAL_MAX_DELAY);
		}
		HAL_Delay(10);
	}
  /* USER CODE END vTaskOne */
}

/* USER CODE BEGIN Header_vTaskTwo */
/**
* @brief Function implementing the task2 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_vTaskTwo */
void vTaskTwo(void *argument)
{
  /* USER CODE BEGIN vTaskTwo */

	BaseType_t RetVal = pdTRUE;
	uint8_t message = 0;
	uint8_t choice = 0;

  /* Infinite loop */
	for(;;)
	{
		RetVal= xQueueReceive(myQueue01Handle, &message, HAL_MAX_DELAY);
		if(pdTRUE == RetVal)
			LcdSetFrame(STAGE_TWO, message);

		choice = MGPIO_u8ReadPortPin(&ChoiceButton);
		HAL_Delay(100);

		if(GPIO_HIGH == choice)
			osSemaphoreRelease(myBinarySem01Handle);

		else RetVal = xQueueSendToFront(myQueue01Handle, (void *)&message, HAL_MAX_DELAY);
	}
  /* USER CODE END vTaskTwo */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */


void LcdSetFrame(uint8_t stage, uint8_t frame)
{
	frame--;
	switch(stage)
	{
		case STAGE_ONE :
			lcd_4bit_send_string_pos(&lcd_1, 1, 1, FirstScreenFrames[frame][0]);
			lcd_4bit_send_string_pos(&lcd_1, 2, 1, FirstScreenFrames[frame][1]);
			break;

		case STAGE_TWO :
			lcd_4bit_send_string_pos(&lcd_1, 1, 1, ScoundScreenFrames[frame][0]);
			lcd_4bit_send_string_pos(&lcd_1, 2, 1, ScoundScreenFrames[frame][1]);
			break;
	}
}

/* Task to be created. */


/* USER CODE END Application */

