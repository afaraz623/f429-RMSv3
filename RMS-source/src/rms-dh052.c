#include "dh052_internal.h"
#include "gpio.h"
#include "tim.h"
#include "cmsis_os.h"


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