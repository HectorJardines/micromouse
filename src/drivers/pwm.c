#include "../inc/drivers/pwm.h"
#include "../inc/common/defines.h"

#define NO_OF_PWM_CFGS          (2U)
#define PWM_TIMER_FREQ          (SYSCLK_FREQ_HZ)
#define PWM_PERIOD_FREQ_HZ      (5000U) // 5KHz timer freq (highest the l293d is rated for)
#define PWM_TIMER_PSC           (15U)
#define PWM_TIMER_TICK_COUNTS   ((PWM_TIMER_FREQ / PWM_PERIOD_FREQ_HZ) / (PWM_TIMER_PSC + 1))
#define PWM_TIMER_PERIOD        (PWM_TIMER_TICK_COUNTS - 1)

struct pwm_channel_confg {
    bool enabled;
    volatile uint32_t *const ccmrx;
    volatile uint32_t *const ccrx;
};

struct pwm_channel_confg pwm_congfigs[] = {
    [PWM_L293D_LEFT] = {.enabled = false, .ccmrx = &TIM2->CCMR1, .ccrx = &TIM2->CCR1},
    [PWM_L293D_RIGHT] = {.enabled = false, .ccmrx = &TIM2->CCMR1, .ccrx = &TIM2->CCR2}
};

static bool all_pwm_channels_disbaled(void)
{
    for (uint8_t channel = 0; channel < NO_OF_PWM_CFGS; ++channel)
    {
        if (pwm_congfigs[channel].enabled == ENABLE)
            return false;
    }
    return true;
}

static bool initialized = false;
void pwm_init(void)
{
    if (initialized)
        return;
    // enable timer peripheral clock
    RCC->APB1ENR |= (RCC_APB1ENR_TIM2EN);
    // configure capture compare channels 1 & 2 as output
    TIM2->CCMR1 &= ~(TIM_CCMR1_CC1S | TIM_CCMR1_CC2S);
    // OC1/2 are set to active low (changes to active HIGH when TIMx_CNT > TIMx_CCR1)
    TIM2->CCER |= (TIM_CCER_CC1P | TIM_CCER_CC2P);
    // channel 1 inactive as long as TIMx_CNT < TIMx_CCR1; else active (upcounting)
    TIM2->CCMR1 |= ((0x7U << TIM_CCMR1_OC1M_Pos) | (0x7U << TIM_CCMR1_OC2M_Pos));
    //configure 20KHz (common freq for small brushed DC motors)
    // tim_cnt_frq = f_sys/(arr * (psc + 1) ) => 8000000 / ( (99 + 1) * (3 + 1) )
    TIM2->ARR = (PWM_TIMER_PERIOD);
    TIM2->PSC = (PWM_TIMER_PSC);

    initialized = true;
}

static bool pwm_enabled = false;
static void pwm_enable(bool EnOrDi)
{
    if (pwm_enabled != EnOrDi && EnOrDi == true) {
            // enable capture compare output
            TIM2->CCER |= TIM_CCER_CC1E;
            // enable counter
            TIM2->CR1 |= TIM_CR1_CEN;
    }
    else if (pwm_enabled != EnOrDi && EnOrDi == false) {
        TIM2->CR1 &= ~(TIM_CR1_CEN);
    }
    pwm_enabled = EnOrDi;
}

void pwm_channel_enable(pwm_channel_e pwm, uint8_t EnOrDi)
{
    if (pwm_congfigs[pwm].enabled != ENABLE) {
        // preload is enabled
        *pwm_congfigs[pwm].ccmrx |= (TIM_CCMR1_OC1PE);
        TIM2->CR1 |= (TIM_CR1_ARPE);
        TIM2->CR1 &= ~(TIM_CR1_CMS | TIM_CR1_DIR);

        pwm_congfigs[pwm].enabled = EnOrDi;

        if (EnOrDi == ENABLE) {
            pwm_enable(ENABLE);
        }
        else if (all_pwm_channels_disbaled())
            pwm_enable(DISABLE);       
    }
}

void pwm_set_duty_cycle(pwm_channel_e pwm, uint8_t duty)
{
    if (duty > 100)
        return;
    bool enable = duty > 0;
    if (enable)
        *pwm_congfigs[pwm].ccrx = duty;
    pwm_enable(pwm, enable);
}