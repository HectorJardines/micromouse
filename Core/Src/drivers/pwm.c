#include "../inc/drivers/pwm.h"
#include "../inc/common/defines.h"
#include "../Inc/common/assert_handler.h"
#include "../inc/common/log.h"

#define NO_OF_PWM_CFGS          (2U)
#define PWM_TIMER_FREQ          (APB1_TIM_CLK_FREQ_HZ)
#define PWM_PERIOD_FREQ_HZ      (20000U)
#define PWM_TIMER_PSC           (0U) // Resolution of 100 ticks is too small
#define PWM_TIMER_TICK_COUNTS   ((PWM_TIMER_FREQ / PWM_PERIOD_FREQ_HZ) / (PWM_TIMER_PSC + 1)) // 3200 / 1 = 3200 ticks
#define PWM_TIMER_ARR           (PWM_TIMER_TICK_COUNTS - 1)
#define MAX_DUTY_VALUE          (PWM_TIMER_ARR)
#define MIN_DUTY_VALUE          (960U)

#define GET_DUTY_CYLE_MAGNITUDE(duty) (duty * -1U)

struct pwm_channel_confg {
    bool enabled;
    volatile uint32_t *const ccmrx;
    volatile uint32_t *const ccrx;
};

struct pwm_channel_confg pwm_configs[] = {
    [PWM_TB6612FNG_LEFT] = {.enabled = false, .ccmrx = &TIM2->CCMR2, .ccrx = &TIM2->CCR3},
    [PWM_TB6612FNG_RIGHT] = {.enabled = false, .ccmrx = &TIM2->CCMR2, .ccrx = &TIM2->CCR4}
};

static bool initialized = false;
void pwm_init(void)
{
    ASSERT(!initialized, ASSERT_DRIVER_LEVEL);
    // enable timer peripheral clock
    RCC->APB1ENR |= (RCC_APB1ENR_TIM2EN);
    // configure capture compare channels 3 & 4 as output
    TIM2->CCMR2 &= ~(TIM_CCMR2_CC3S | TIM_CCMR2_CC4S);
    // configure preload register enable
    TIM2->CCMR2 |= (TIM_CCMR2_OC3PE | TIM_CCMR2_OC4PE);
    // OC3/2 are set to active low (changes to active HIGH when TIMx_CNT > TIMx_CCR3)
    TIM2->CCER &= ~(TIM_CCER_CC3P | TIM_CCER_CC4P);
    // channel 3 ACTIVE as long as TIMx_CNT < TIMx_CCR3; else inactive (upcounting)
    TIM2->CCMR2 |= ((0x6U << TIM_CCMR2_OC3M_Pos) | (0x6U << TIM_CCMR2_OC4M_Pos));
    // enable timer ARR buffering
    TIM2->CR1 |= (TIM_CR1_ARPE);
    // edge aligned, upcounting mode
    TIM2->CR1 &= ~(TIM_CR1_CMS | TIM_CR1_DIR);
    //configure 20KHz (common freq for small brushed DC motors)
    // tim_cnt_frq = f_sys/(arr * (psc + 1) ) => 8000000 / ( (99 + 1) * (3 + 1) )
    TIM2->ARR = (PWM_TIMER_ARR);
    TIM2->PSC = (PWM_TIMER_PSC);

    TIM2->EGR |= TIM_EGR_UG;
    // enable capture compare on channels 3 and 4
    TIM2->CCER |= (TIM_CCER_CC3E | TIM_CCER_CC4E);
    TIM2->CNT = 0;

    initialized = true;
}

static bool pwm_enabled = false;

/**
 * @brief Enables/disables the pwm timer peripheral as pwm channels go in and out of use
 * 
 * @param EnOrDi indicates whether the peripheral should be EN or DI
 */
static void pwm_timer_ctl(bool EnOrDi)
{
    if (pwm_enabled != EnOrDi && EnOrDi == ENABLE) {
            // enable counter
            TIM2->CR1 |= TIM_CR1_CEN;
    }
    else if (pwm_enabled != EnOrDi && EnOrDi == DISABLE) {
        TIM2->CR1 &= ~(TIM_CR1_CEN);
    }
    pwm_enabled = EnOrDi;
}

static bool pwm_all_channels_disabled(void) {
    return (pwm_configs[PWM_TB6612FNG_LEFT].enabled == DISABLE) && (pwm_configs[PWM_TB6612FNG_RIGHT].enabled == DISABLE);
}

/**
 * @brief Enables/disables pwm timer peripheral based on the current state and pwm duty cycle values
 * 
 * Only disables the timer peripheral once both pwm channels are not in use, indicated by the current 
 * duty cycle of each pwm channel. Enables pwm timer peripheral when at least one of the channels is 
 * in use.
 * 
 * @param pwm pwm channel to enable/disable
 * @param EnOrDi boolean indicating whether to disable or enable peripheral
 */
static void pwm_channel_enable(pwm_channel_e pwm, uint8_t EnOrDi)
{
    if (pwm_configs[pwm].enabled == EnOrDi) {
        return;
    }

    if (EnOrDi == ENABLE) {
        pwm_timer_ctl(ENABLE);
        LOG("PWM CHANNEL %d ENABLED\n", pwm);
    }
    else {
        if (pwm_all_channels_disabled()) {
            pwm_timer_ctl(DISABLE);
            LOG("PWM CHANNELS IDLE, DISABLED...\n");
        }
    }
    pwm_configs[pwm].enabled = EnOrDi;
}

static uint32_t limit_duty_cycle(int32_t duty) {
    uint32_t duty_cycle;
    if (duty < 0)
        duty_cycle = GET_DUTY_CYLE_MAGNITUDE(duty);
    else
        duty_cycle = duty;

    if (duty_cycle > MAX_DUTY_VALUE) {
        LOG("DUTY CYCLE VALUE EXCEEDS 100: SETTING DUTY = 3199\n");
        duty_cycle = MAX_DUTY_VALUE;
    }
    else if (duty_cycle < MIN_DUTY_VALUE) {
        LOG("DUTY CYCLE VALUE LOWER THAN 30: SETTING DUTY = 960\n");
        duty_cycle = MIN_DUTY_VALUE;
    }

    return duty_cycle;
}
void pwm_set_duty_cycle(pwm_channel_e pwm, int32_t duty)
{
    uint32_t duty_cycle = limit_duty_cycle(duty);
    bool enable = duty_cycle > 0;
    if (enable)
        *pwm_configs[pwm].ccrx = duty_cycle;
    pwm_channel_enable(pwm, enable);
}