#include "dh052_internal.h"

/******************************* Aliases **************************************/
/* 
   "ONE_PERIOD" is this value because the counter counts on "both" edges of both 
   channels making one pulse counted as 4 for greater accuracy. Thats the this is
   divided by 4.
*/ 
#define ONE_PERIOD 16384  
#define HALF_PERIOD 8192

/****************************** Global Variables ******************************/
static int32_t g_prev = 0;

void servo_Init(Motor *handle,TIM_HandleTypeDef *pwm_tim_h , uint8_t lpwm_ch, uint8_t rpwm_ch, GPIO_TypeDef * port, uint16_t l_en, uint16_t r_en, TIM_HandleTypeDef *enc_tim_h)
{
    handle->pwm_tim_h = pwm_tim_h;
    handle->port = port;
    handle->l_en = l_en;
    handle->r_en = r_en;
    handle->lpwm_ch = lpwm_ch;
    handle->rpwm_ch = rpwm_ch;
    handle->enc_tim_h = enc_tim_h;
}

void servo_enable(Motor const *handle)
{   
    HAL_GPIO_WritePin(handle->port, handle->l_en, 1);
    HAL_GPIO_WritePin(handle->port, handle->r_en, 1);
}

void servo_disable(Motor const *handle)
{   
    HAL_GPIO_WritePin(handle->port, handle->l_en, 0);
    HAL_GPIO_WritePin(handle->port, handle->r_en, 0);
}

void servo_turn_left(Motor const *handle, uint16_t pwm)
{
    __HAL_TIM_SET_COMPARE(handle->pwm_tim_h, handle->lpwm_ch, pwm);
    __HAL_TIM_SET_COMPARE(handle->pwm_tim_h, handle->rpwm_ch, 0);
}

void servo_turn_right(Motor const *handle, uint16_t pwm)
{
    __HAL_TIM_SET_COMPARE(handle->pwm_tim_h, handle->rpwm_ch, pwm);
    __HAL_TIM_SET_COMPARE(handle->pwm_tim_h, handle->lpwm_ch, 0);
}

void servo_stop(Motor const *handle)
{
    __HAL_TIM_SET_COMPARE(handle->pwm_tim_h, handle->lpwm_ch, 0);
    __HAL_TIM_SET_COMPARE(handle->pwm_tim_h, handle->rpwm_ch, 0);
}

int32_t servo_encoder_count(Motor const *handle)
{
    int32_t c32 =  (int32_t)handle->enc_tim_h->Instance->CNT - HALF_PERIOD;
    int32_t dif = c32 - g_prev; 

    int32_t mod_dif = ((dif + HALF_PERIOD) % ONE_PERIOD) - HALF_PERIOD;
    if(dif < -HALF_PERIOD)
        mod_dif += ONE_PERIOD; 

    int32_t unwrapped = g_prev + mod_dif;
    g_prev = unwrapped; //load previous value

    return unwrapped + HALF_PERIOD;
}