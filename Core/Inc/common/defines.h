#ifndef _DEFINES_H
#define _DEFINES_H

#define SYSCLK_FREQ_HZ          (64000000UL)
#define APB1_PERI_CLK_FREQ_HZ   (32000000UL)
#define APB2_PERI_CLK_FREQ_HZ   (SYSCLK_FREQ_HZ)
#define APB1_TIM_CLK_FREQ_HZ    (SYSCLK_FREQ_HZ)
#define APB2_TIM_CLK_FREQ_HZ    (SYSCLK_FREQ_HZ)  

#define DIV_8   (0x3U)

#define GEN_ERROR   (-1)
#define GEN_OK      (0)

#endif