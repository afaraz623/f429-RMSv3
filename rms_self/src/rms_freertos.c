#include "FreeRTOS.h"
#include "queue.h"
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
#define BUF_SIZE 128

/****************************** Global Variables ******************************/
static int32_t g_enc_val = 0;
static uint16_t g_rpm = 0;

/********************************* Prototypes ********************************/
extern bool cubemx_transport_open(struct uxrCustomTransport * transport);
extern bool cubemx_transport_close(struct uxrCustomTransport * transport);
extern size_t cubemx_transport_write(struct uxrCustomTransport* transport, const uint8_t * buf, size_t len, uint8_t * err);
extern size_t cubemx_transport_read(struct uxrCustomTransport* transport, uint8_t* buf, size_t len, int timeout, uint8_t* err);
extern void microros_deallocate(void * pointer, void * state);
extern void * microros_allocate(size_t size, void * state);
extern void * microros_reallocate(void * pointer, size_t size, void * state);
extern void * microros_zero_allocate(size_t number_of_elements, size_t size_of_element, void * state);
void servo_Init(Motor *handle,TIM_HandleTypeDef *pwm_tim_h , uint8_t lpwm_ch, uint8_t rpwm_ch, GPIO_TypeDef * port, uint16_t l_en, uint16_t r_en, TIM_HandleTypeDef *enc_tim_h, TIM_HandleTypeDef *enc_ic_tim_h);
extern void servo_enable(Motor const * handle);
extern void servo_disable(Motor const * handle);
extern void servo_turn_left(Motor const * handle, uint16_t pwm);
extern void servo_turn_right(Motor const * handle, uint16_t pwm);
extern void servo_stop(Motor const * handle);
extern int32_t servo_encoder_count(Motor const *handle);
extern uint16_t servo_rpm(Motor *handle);


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
  
  while(1)
  {
    snprintf(msg.data.data, msg.data.capacity, "Enc Count: %ld, Rpm: %d", g_enc_val, g_rpm);
    msg.data.size = strlen(msg.data.data);
    rcl_publish(&publisher, &msg, NULL);
    osDelay(100);
  }
}

void start_mtr_ctrl_task(void *argument)
{
  Motor mtr1;
  servo_Init(&mtr1, &htim8, TIM_CHANNEL_1, TIM_CHANNEL_2, GPIOA, MTR1_L_EN_Pin, MTR1_R_EN_Pin, &htim3, &htim4);

  servo_enable(&mtr1);
  while (1)
  {
    g_enc_val = servo_encoder_count(&mtr1);
    g_rpm = servo_rpm(&mtr1);
    servo_turn_left(&mtr1, 200); 
  }
}
