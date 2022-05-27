#ifndef DH052_INTERNAL_H
#define DH052_INTERNAL_H

#include <stdint.h>
#include "gpio.h"

typedef struct 
{
    uint8_t L_PWM_CH;
    uint8_t R_PWM_CH;
    GPIO_TypeDef * Port;
    uint16_t L_EN;
    uint16_t R_EN;
} Motor;

void Motor_Init(Motor * self, uint8_t L_PWM_CH, uint8_t R_PWM_CH, GPIO_TypeDef * Port, uint16_t L_EN, uint16_t R_EN);
void Motor_enable(Motor const * self);
void Motor_disable(Motor const * self);
void Motor_turn_left(Motor const * self, uint16_t pwm);
void Motor_turn_right(Motor const * self, uint16_t pwm);
void Motor_stop(Motor const * self);

#endif