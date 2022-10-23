#include "dh052_internal.h"

/******************************* Aliases **************************************/
/* 
   "ONE_PERIOD" is this value because the counter counts on "both" edges of both 
   channels. Hence, a single count is pulsed 4 times for greater accuracy. Thats 
   why it's divided by 4.
*/ 
#define ONE_PERIOD 16384  
#define HALF_PERIOD 8192
#define MIN_TO_MS 60000UL
#define IDLE 0
#define BUSY 1

/****************************** Global Variables ******************************/
static int32_t g_prev = 0;
static uint16_t g_rpm = 0;
static uint8_t g_ic_state = IDLE;
static uint16_t g_dif = 0;
static volatile uint16_t g_ts1 = 0, g_ts2 = 0;

void servo_Init(Motor *handle,TIM_HandleTypeDef *pwm_tim_h , uint8_t lpwm_ch, uint8_t rpwm_ch, GPIO_TypeDef * port, uint16_t l_en, uint16_t r_en, TIM_HandleTypeDef *enc_tim_h, TIM_HandleTypeDef *enc_ic_tim_h)
{
    handle->pwm_tim_h = pwm_tim_h;
    handle->port = port;
    handle->l_en = l_en;
    handle->r_en = r_en;
    handle->lpwm_ch = lpwm_ch;
    handle->rpwm_ch = rpwm_ch;
    handle->enc_tim_h = enc_tim_h;
    handle->enc_ic_tim_h = enc_ic_tim_h;
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
    int32_t c32 =  ((volatile int32_t)handle->enc_tim_h->Instance->CNT / 4) - HALF_PERIOD;
    int32_t dif = c32 - g_prev; 

    int32_t mod_dif = ((dif + HALF_PERIOD) % ONE_PERIOD) - HALF_PERIOD;
    if(dif < -HALF_PERIOD)
        mod_dif += ONE_PERIOD; 

    int32_t unwrapped = g_prev + mod_dif;
    g_prev = unwrapped; //load previous value

    // NO PRECAUTIONS TAKEN TO DEAL WITH BUFFER OVERFLOW FOR NOW!
    return unwrapped + HALF_PERIOD;
}

uint16_t servo_rpm(Motor *handle)
{
    return g_rpm;
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim == &htim4 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
    {
        if (g_ic_state == IDLE)
        {
            HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, 1);
            g_ts1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1); // read the first value
            g_ic_state = BUSY;  // set the first captured as true
        }
        else   // If the first rising edge is captured, now we will capture the second edge
        {
            HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, 0);
            g_ts2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);  // read second value

            if (g_ts2 > g_ts1)
            {
                g_dif = g_ts2 - g_ts1;
            }
            else if (g_ts1 > g_ts2)
            {
                g_dif = (0xffff - g_ts1) + g_ts2;
            }

            g_rpm = MIN_TO_MS / (g_dif * 1000 * 0.0005f);

            __HAL_TIM_SET_COUNTER(htim, 0);  // reset the counter
            g_ic_state = IDLE; // set it back to false
        }
    }
}