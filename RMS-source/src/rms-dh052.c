#include "dh052_internal.h"
#include "gpio.h"
#include "tim.h"
#include "cmsis_os.h"

// 'ONE_PERIOD" is this val because im triggering both encoder inputs on both 
// rising and falling edges, for greater accuracy, so encoder val in  4x. 
// Thats why 65536 / 4 to get this 
#define ONE_PERIOD 16384  
#define HALF_PERIOD 8192

void Motor_Init(Motor * self, uint8_t L_PWM_CH, uint8_t R_PWM_CH, GPIO_TypeDef * Port, uint16_t L_EN, uint16_t R_EN)
{
    self->Port = Port;
    self->L_EN = L_EN;
    self->R_EN = R_EN;
    self->L_PWM_CH = L_PWM_CH;
    self->R_PWM_CH = R_PWM_CH;
}

void Motor_enable(Motor const * self)
{   
    HAL_GPIO_WritePin(self->Port, self->L_EN, 1);
    HAL_GPIO_WritePin(self->Port, self->R_EN, 1);
}

void Motor_disable(Motor const * self)
{   
    HAL_GPIO_WritePin(self->Port, self->L_EN, 0);
    HAL_GPIO_WritePin(self->Port, self->R_EN, 0);
}

void Motor_turn_left(Motor const * self, uint16_t pwm)
{
    __HAL_TIM_SET_COMPARE(&htim8, self->L_PWM_CH, pwm);
    osDelay(1);
    __HAL_TIM_SET_COMPARE(&htim8, self->R_PWM_CH, 0);
}

void Motor_turn_right(Motor const * self, uint16_t pwm)
{
    __HAL_TIM_SET_COMPARE(&htim8, self->R_PWM_CH, pwm);
    osDelay(1);
    __HAL_TIM_SET_COMPARE(&htim8, self->L_PWM_CH, 0);
}

void Motor_stop(Motor const * self)
{
    __HAL_TIM_SET_COMPARE(&htim8, self->L_PWM_CH, 0);
    __HAL_TIM_SET_COMPARE(&htim8, self->R_PWM_CH, 0);
}

int32_t Motor_encoder(Motor const * self, uint16_t encCount, int32_t * prev)
{
    int32_t c32 = (int32_t)self->encCount - HALF_PERIOD; //remove half period to determine (+/-) sign of the wrap
    int32_t dif = c32 - *prev; //core concept: prev + (current - prev) = current

    //wrap difference from -HALF_PERIOD to HALF_PERIOD. modulo prevents differences after the wrap from having an incorrect result
    int32_t mod_dif = ((dif + HALF_PERIOD) % ONE_PERIOD) - HALF_PERIOD;
    if(dif < -HALF_PERIOD)
        mod_dif += ONE_PERIOD; //account for mod of negative number behavior in C

    int32_t unwrapped = *prev + mod_dif;
    *prev = unwrapped; //load previous value

    return unwrapped + HALF_PERIOD; //remove the shift we applied at the beginning, and return
}