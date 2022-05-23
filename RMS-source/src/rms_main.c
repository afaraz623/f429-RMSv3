#include <../../../f429-RMSv3-autogen/Core/Inc/main.h>
#include <../../../f429-RMSv3-autogen/Core/Inc/gpio.h>

extern void SystemClock_Config(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    while (1)
    {
        HAL_GPIO_TogglePin(led_1_GPIO_Port, led_1_Pin);
        HAL_Delay(2000);
        HAL_GPIO_TogglePin(led_2_GPIO_Port, led_2_Pin);
        HAL_Delay(2000);
    }
    
}