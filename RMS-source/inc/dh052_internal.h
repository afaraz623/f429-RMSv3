#ifndef DH052_INTERNAL_H
#define DH052_INTERNAL_H

#include <stdint.h>
#include "gpio.h"
#include "tim.h"

typedef struct 
{
    TIM_HandleTypeDef *pwm_tim_h;
    uint8_t lpwm_ch;
    uint8_t rpwm_ch;
    GPIO_TypeDef *port;
    uint16_t l_en;
    uint16_t r_en;
    TIM_HandleTypeDef *enc_tim_h;
} Motor;

void servo_Init(Motor *handle,TIM_HandleTypeDef *pwm_tim_h , uint8_t lpwm_ch, uint8_t rpwm_ch, GPIO_TypeDef * port, uint16_t l_en, uint16_t r_en, TIM_HandleTypeDef *enc_tim_h);
void servo_enable(Motor const * handle);
void servo_disable(Motor const * handle);
void servo_turn_left(Motor const * handle, uint16_t pwm);
void servo_turn_right(Motor const * handle, uint16_t pwm);
void servo_stop(Motor const * handle);
int32_t servo_encoder_count(Motor const *handle);

#endif