#include <../../../f429-RMSv3-autogen/Core/Inc/main.h>
#include <../../../f429-RMSv3-autogen/Core/Inc/gpio.h>

// Prototypes
extern void SystemClock_Config(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    while (1)
    {

    }
    
}