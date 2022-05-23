#include <main.h>
#include <cmsis_os.h>
#include "rms-microros.h"
#include <usart.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

//*******************************[Aliases}**************************************
#define BUF_SIZE 256

//******************************[Global Variables]******************************
int counter = 0;

//**************************[Global RCL Variables]******************************
rcl_publisher_t publisher;
std_msgs__msg__String msg;

//**************************[Prototypes]******************************
extern bool cubemx_transport_open(struct uxrCustomTransport * transport);
extern bool cubemx_transport_close(struct uxrCustomTransport * transport);
extern size_t cubemx_transport_write(struct uxrCustomTransport* transport, const uint8_t * buf, size_t len, uint8_t * err);
extern size_t cubemx_transport_read(struct uxrCustomTransport* transport, uint8_t* buf, size_t len, int timeout, uint8_t* err);
extern void microros_deallocate(void * pointer, void * state);
extern void * microros_allocate(size_t size, void * state);
extern void * microros_reallocate(void * pointer, size_t size, void * state);
extern void * microros_zero_allocate(size_t number_of_elements, size_t size_of_element, void * state);


void timer_callback(rcl_timer_t * timer, int64_t last_call_time)
{
	(void) last_call_time;
	if (timer != NULL) 
    {
        sprintf(msg.data.data, "Hello from micro-ROS #%d", counter++);
		msg.data.size = strlen(msg.data.data);
		rcl_publish(&publisher, &msg, NULL);
	}
}

void StartMicroRosTask(void *argument)
{
    rmw_uros_set_custom_transport(true, (void *) &huart3, cubemx_transport_open, cubemx_transport_close, cubemx_transport_write, cubemx_transport_read);

    rcl_allocator_t freeRTOS_allocator = rcutils_get_zero_initialized_allocator();
    freeRTOS_allocator.allocate = microros_allocate;
    freeRTOS_allocator.deallocate = microros_deallocate;
    freeRTOS_allocator.reallocate = microros_reallocate;
    freeRTOS_allocator.zero_allocate =  microros_zero_allocate;

    // Node main loop //
    rcl_allocator_t allocator = rcl_get_default_allocator();
    rclc_support_t support;

    //Create init_options
    rclc_support_init(&support, 0, NULL, &allocator);

    // Create node
    rcl_node_t node;
    rclc_node_init_default(&node, "RMS_node", "", &support);

    // Create publisher
    rclc_publisher_init_default(&publisher, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String), "/String_publisher");

    // Create timer
    rcl_timer_t timer;
    const unsigned int timer_timeout = 1000;
    rclc_timer_init_default( &timer, &support, RCL_MS_TO_NS(timer_timeout), timer_callback);

    // Create executor
    rclc_executor_t executor = rclc_executor_get_zero_initialized_executor();
    rclc_executor_init(&executor, &support.context, 1, &allocator);
    rclc_executor_add_timer(&executor, &timer);

    // Fill the array with a known sequence
    msg.data.data = (char * ) malloc(BUF_SIZE * sizeof(char));
    msg.data.size = 0;
    msg.data.capacity = BUF_SIZE;~

    rclc_executor_spin(&executor);
    
    // Distroying node
    rcl_publisher_fini(&publisher, &node);
    rcl_node_fini(&node);

    // Just in case the thread ends abruptly, this function makes sure the thread exits safely
    osThreadTerminate(NULL);
}