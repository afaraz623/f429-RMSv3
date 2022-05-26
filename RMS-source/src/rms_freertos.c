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
#include <std_msgs/msg/int32.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>

/******************************* Aliases **************************************/
#define ENC_VAL (TIM3->CNT>>2) // shifting the actual val by 2bits
#define BUF_SIZE 256

/****************************** Global Variables ******************************/
int32_t encPrev = 0;
int32_t val = 0;

/********************************* Prototypes ********************************/
extern bool cubemx_transport_open(struct uxrCustomTransport * transport);
extern bool cubemx_transport_close(struct uxrCustomTransport * transport);
extern size_t cubemx_transport_write(struct uxrCustomTransport* transport, const uint8_t * buf, size_t len, uint8_t * err);
extern size_t cubemx_transport_read(struct uxrCustomTransport* transport, uint8_t* buf, size_t len, int timeout, uint8_t* err);
extern void microros_deallocate(void * pointer, void * state);
extern void * microros_allocate(size_t size, void * state);
extern void * microros_reallocate(void * pointer, size_t size, void * state);
extern void * microros_zero_allocate(size_t number_of_elements, size_t size_of_element, void * state);


/* This task creates and handles the execution of node 'RMS' */ 
void start_uros_task(void *argument)
{
    rmw_uros_set_custom_transport(true, (void *) &huart3, cubemx_transport_open, cubemx_transport_close, cubemx_transport_write, cubemx_transport_read);

    rcl_allocator_t freeRTOS_allocator = rcutils_get_zero_initialized_allocator();
    freeRTOS_allocator.allocate = microros_allocate;
    freeRTOS_allocator.deallocate = microros_deallocate;
    freeRTOS_allocator.reallocate = microros_reallocate;
    freeRTOS_allocator.zero_allocate =  microros_zero_allocate;

     if (!rcutils_set_default_allocator(&freeRTOS_allocator)) {
      printf("Error on default allocators (line %d)\n", __LINE__); 
  }

  // micro-ROS app

  rcl_publisher_t publisher;
  std_msgs__msg__Int32 msg;
  rclc_support_t support;
  rcl_allocator_t allocator;
  rcl_node_t node;

  allocator = rcl_get_default_allocator();

  //create init_options
  rclc_support_init(&support, 0, NULL, &allocator);

  // create node
  rclc_node_init_default(&node, "rms_node", "", &support);

  // create publisher
  rclc_publisher_init_default(
    &publisher,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32), "numbers");

  msg.data = 0;

  for(;;)
  {
    rcl_ret_t ret;

	msg.data = val;
    ret = rcl_publish(&publisher, &msg, NULL);
    if (ret != RCL_RET_OK)
    {
      printf("Error publishing (line %d)\n", __LINE__); 
    }
    
    osDelay(100);
  }
}

void start_mtr_ctrl_task(void *argument)
{
  Motor mtr1;

  Motor_Init(&mtr1, TIM_CHANNEL_1, TIM_CHANNEL_2, GPIOA, MTR1_L_EN_Pin, MTR1_R_EN_Pin);

  while (1)
  {  
    Motor_enable(&mtr1);

    Motor_turn_left(&mtr1, 200);
    osDelay(2000);

    Motor_turn_left(&mtr1, 400);
    osDelay(2000);

    Motor_turn_left(&mtr1, 600);
    osDelay(2000);

    Motor_turn_left(&mtr1, 800);
    osDelay(2000);

    Motor_turn_left(&mtr1, 1000);
    osDelay(2000);

    Motor_disable(&mtr1);
    osDelay(2000);
  }
  
}