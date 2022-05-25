#include "rms-microros.h"
#include "cmsis_os.h"
#include "main.h"
#include "usart.h"
#include "tim.h"
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>

/******************************* Aliases **************************************/
#define BUF_SIZE 256

/****************************** Global Variables ******************************/


/***********************`*** Global RCL Variables ******************************/
rcl_publisher_t my_pub_1;
std_msgs__msg__String pub_msg_1;

/********************************* Prototypes ********************************/
extern bool cubemx_transport_open(struct uxrCustomTransport * transport);
extern bool cubemx_transport_close(struct uxrCustomTransport * transport);
extern size_t cubemx_transport_write(struct uxrCustomTransport* transport, const uint8_t * buf, size_t len, uint8_t * err);
extern size_t cubemx_transport_read(struct uxrCustomTransport* transport, uint8_t* buf, size_t len, int timeout, uint8_t* err);
extern void microros_deallocate(void * pointer, void * state);
extern void * microros_allocate(size_t size, void * state);
extern void * microros_reallocate(void * pointer, size_t size, void * state);
extern void * microros_zero_allocate(size_t number_of_elements, size_t size_of_element, void * state);

/****************************** Callbacks ***********************************/
void my_timer_callback(rcl_timer_t *timer, int64_t last_call_time)
{
    rcl_ret_t rc;
    UNUSED(last_call_time);
    if(timer != NULL)
    {
        snprintf(pub_msg_1.data.data, pub_msg_1.data.capacity,"%d", (TIM3->CNT>>2));
        pub_msg_1.data.size = strlen(pub_msg_1.data.data);
        rc = rcl_publish(&my_pub_1, &pub_msg_1, NULL);
        if(rc != RCL_RET_OK)
            Error_Handler();
    }
}

/* This task creates and handles the execution of node 'RMS' */ 
void StartMicroRosTask(void *argument)
{
    rmw_uros_set_custom_transport(true, (void *) &huart3, cubemx_transport_open, cubemx_transport_close, cubemx_transport_write, cubemx_transport_read);

    rcl_allocator_t freeRTOS_allocator = rcutils_get_zero_initialized_allocator();
    freeRTOS_allocator.allocate = microros_allocate;
    freeRTOS_allocator.deallocate = microros_deallocate;
    freeRTOS_allocator.reallocate = microros_reallocate;
    freeRTOS_allocator.zero_allocate =  microros_zero_allocate;

    rcl_allocator_t allocator = rcl_get_default_allocator();
    rclc_support_t support;
    rcl_ret_t rc;
    
    rc = rclc_support_init(&support, 0, NULL, &allocator);
    if(rc != RCL_RET_OK)
            Error_Handler();

    rcl_node_t my_node;
    rc = rclc_node_init_default(&my_node, "test_node", "RMS", &support);
    if(rc != RCL_RET_OK)
            Error_Handler();

    const char *topic_name_1 = "topic_1";
    const rosidl_message_type_support_t *my_type_support = ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String);

    rc = rclc_publisher_init_default(&my_pub_1, &my_node, my_type_support, topic_name_1);
    if(rc != RCL_RET_OK)
            Error_Handler();

    rcl_timer_t my_timer; 
    const unsigned int timer_timeout = 10; // in ms
    rc = rclc_timer_init_default( &my_timer, &support, RCL_MS_TO_NS(timer_timeout), my_timer_callback);
    if(rc != RCL_RET_OK)
            Error_Handler();

    // Assign message to publisher
    std_msgs__msg__String__init(&pub_msg_1);    
    const unsigned int PUB_MSG_BUFFER = BUF_SIZE;
    pub_msg_1.data.data = malloc(PUB_MSG_BUFFER);
    pub_msg_1.data.capacity = PUB_MSG_BUFFER;

    rclc_executor_t executor = rclc_executor_get_zero_initialized_executor();
    // total number of handles = #timers + #subscriptions
    unsigned int num_handles = 1; // as this program is using a single timer and no subscriptions at the moment
    rclc_executor_init(&executor, &support.context, num_handles, &allocator);
    rclc_executor_add_timer(&executor, &my_timer);
    if(rc != RCL_RET_OK)
            Error_Handler();

    rclc_executor_spin(&executor);

    // cleaning up
    rc = rclc_executor_fini(&executor);
    rc += rcl_publisher_fini(&my_pub_1, &my_node);
    rc += rcl_timer_fini(&my_timer);
    rc += rcl_node_fini(&my_node);
    rc += rclc_support_fini(&support);
    std_msgs__msg__String__fini(&pub_msg_1);

    if(rc != RCL_RET_OK)
            Error_Handler();

    // Just in case the thread ends abruptly, this function makes sure the thread exits safely
    osThreadTerminate(NULL);
}