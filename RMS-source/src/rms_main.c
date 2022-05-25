#include "main.h"
#include "gpio.h"
#include "cmsis_os.h"
#include "dma.h"
#include "usart.h"
#include "tim.h"

/***************************** Prototypes ****************************/
extern void SystemClock_Config(void);
extern void MX_FREERTOS_Init(void);

int main(void)
{
    // Reset of all peripherals, Initializes the Flash interface and the Systick
    HAL_Init();

    // Configure the system clock
    SystemClock_Config();

    // Initialize all configured peripherals
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_USART3_UART_Init();
    MX_TIM3_Init();
    HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);

    // Call init function for freertos objects (in freertos.c)
    osKernelInitialize();  
    MX_FREERTOS_Init();

    //Start scheduler
    osKernelStart();    
}