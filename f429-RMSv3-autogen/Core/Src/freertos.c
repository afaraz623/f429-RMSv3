/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for uros_task */
osThreadId_t uros_taskHandle;
uint32_t uros_task_Buffer[ 3000 ];
osStaticThreadDef_t defaultTaskControlBlock;
const osThreadAttr_t uros_task_attributes = {
  .name = "uros_task",
  .cb_mem = &defaultTaskControlBlock,
  .cb_size = sizeof(defaultTaskControlBlock),
  .stack_mem = &uros_task_Buffer[0],
  .stack_size = sizeof(uros_task_Buffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for motor_control */
osThreadId_t motor_controlHandle;
uint32_t motor_control_Buffer[ 128 ];
osStaticThreadDef_t motor_control_tControlBlock;
const osThreadAttr_t motor_control_attributes = {
  .name = "motor_control",
  .cb_mem = &motor_control_tControlBlock,
  .cb_size = sizeof(motor_control_tControlBlock),
  .stack_mem = &motor_control_Buffer[0],
  .stack_size = sizeof(motor_control_Buffer),
  .priority = (osPriority_t) osPriorityNormal1,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void start_uros_task(void *argument);
void start_mtr_ctrl_task(void *argument);

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

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of uros_task */
  uros_taskHandle = osThreadNew(start_uros_task, NULL, &uros_task_attributes);

  /* creation of motor_control */
  motor_controlHandle = osThreadNew(start_mtr_ctrl_task, NULL, &motor_control_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_start_uros_task */
/**
  * @brief  Function implementing the uros_task thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_start_uros_task */
__weak void start_uros_task(void *argument)
{
  /* USER CODE BEGIN start_uros_task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END start_uros_task */
}

/* USER CODE BEGIN Header_start_mtr_ctrl_task */
/**
* @brief Function implementing the motor_control_t thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_start_mtr_ctrl_task */
__weak void start_mtr_ctrl_task(void *argument)
{
  /* USER CODE BEGIN start_mtr_ctrl_task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END start_mtr_ctrl_task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

