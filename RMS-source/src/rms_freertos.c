#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "main.h"
#include "usart.h"
#include "tim.h"
#include "dh052_internal.h"
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <uxr/client/transport.h>
#include <rmw_microxrcedds_c/config.h>
#include <rmw_microros/rmw_microros.h>
#include <std_msgs/msg/string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>

/******************************* Aliases **************************************/
// 'ONE_PERIOD" is this val because im triggering both encoder inputs on both 
// rising and falling edges, for greater accuracy, so encoder val in  4x. 
// Thats why 65536 / 4 to get this 
#define ONE_PERIOD 16384  
#define HALF_PERIOD 8192
#define BUF_SIZE 256
/****************************** Global Variables ******************************/
int32_t encPrev = 0;
/********************************* Prototypes ********************************/
extern bool cubemx_transport_open(struct uxrCustomTransport * transport);
extern bool cubemx_transport_close(struct uxrCustomTransport * transport);
extern size_t cubemx_transport_write(struct uxrCustomTransport* transport, const uint8_t * buf, size_t len, uint8_t * err);
extern size_t cubemx_transport_read(struct uxrCustomTransport* transport, uint8_t* buf, size_t len, int timeout, uint8_t* err);
extern void microros_deallocate(void * pointer, void * state);
extern void * microros_allocate(size_t size, void * state);
extern void * microros_reallocate(void * pointer, size_t size, void * state);
extern void * microros_zero_allocate(size_t number_of_elements, size_t size_of_element, void * state);

int32_t unwrap_encoder(uint16_t encCount, int32_t * prev);

/* This task creates and handles the execution of ros node */ 
void start_uros_task(void *argument)
{
    rmw_uros_set_custom_transport(true, (void *) &huart3, cubemx_transport_open, cubemx_transport_close, cubemx_transport_write, cubemx_transport_read);

    rcl_allocator_t freeRTOS_allocator = rcutils_get_zero_initialized_allocator();
    freeRTOS_allocator.allocate = microros_allocate;
    freeRTOS_allocator.deallocate = microros_deallocate;
    freeRTOS_allocator.reallocate = microros_reallocate;
    freeRTOS_allocator.zero_allocate =  microros_zero_allocate;

    if (!rcutils_set_default_allocator(&freeRTOS_allocator)) 
    {
      printf("Error on default allocators (line %d)\n", __LINE__); 
    }

  // micro-ROS app
  std_msgs__msg__String msg;

  rcl_allocator_t allocator;
  allocator = rcl_get_default_allocator();

  rclc_support_t support;
  rclc_support_init(&support, 0, NULL, &allocator);

  rcl_node_t node;
  rclc_node_init_default(&node, "rms_node", "", &support);

  rcl_publisher_t publisher;
  rclc_publisher_init_default(&publisher, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String), "encoder_info");

  std_msgs__msg__String__init(&msg);
  msg.data.data = malloc(BUF_SIZE);
  msg.data.capacity = BUF_SIZE;

  __HAL_TIM_GetCounter(&htim3) = 32768; 
  
  for(;;)
  {
    snprintf(msg.data.data, msg.data.capacity, "Encoder Val: %ld, Encoder Dir: %d", __HAL_TIM_GetCounter(&htim3) / 4, __HAL_TIM_DIRECTION_STATUS(&htim3));
    msg.data.size = strlen(msg.data.data);
    rcl_publish(&publisher, &msg, NULL);
  }
}

void start_mtr_ctrl_task(void *argument)
{
  Motor mtr1;

  Motor_Init(&mtr1, TIM_CHANNEL_1, TIM_CHANNEL_2, GPIOA, MTR1_L_EN_Pin, MTR1_R_EN_Pin);

  Motor_enable(&mtr1);
  while (1)
  {
    Motor_turn_left(&mtr1, 100);
    osDelay(2000);

    Motor_stop(&mtr1);
    osDelay(1000);
    
    Motor_turn_right(&mtr1, 100);
    osDelay(2000);

    Motor_stop(&mtr1);
    osDelay(1000);
  }
}

int32_t unwrap_encoder(uint16_t encCount, int32_t * prev)
{
    int32_t c32 = (int32_t)encCount - HALF_PERIOD;
    int32_t dif = c32 - *prev; 

    int32_t mod_dif = ((dif + HALF_PERIOD) % ONE_PERIOD) - HALF_PERIOD;
    if(dif < -HALF_PERIOD)
        mod_dif += ONE_PERIOD; 

    int32_t unwrapped = *prev + mod_dif;
    *prev = unwrapped; //load previous value

    return unwrapped + HALF_PERIOD;
}