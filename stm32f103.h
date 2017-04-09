#ifndef __STM32F030_H_
#define __STM32F030_H_

#define REG_L(X,Y) ((volatile unsigned long*)((void*)((X) + (Y))))[0]
#define REG_S(X,Y) ((volatile unsigned short*)((void*)((X) + (Y))))[0]
#define REG_B(X,Y) ((volatile unsigned char*)((void*)((X) + (Y))))[0]

// base addresses

#define TIM2_BASE 0x40000000

#define AFIO_BASE 0x40010000
#define GPIOA_BASE 0x40010800
#define GPIOB_BASE 0x40010C00
#define GPIOC_BASE 0x40011000
#define GPIOD_BASE 0x40011400
#define GPIOE_BASE 0x40011800
#define GPIOF_BASE 0x40011C00
#define GPIOG_BASE 0x40012000

#define USART_BASE 0x40013800

#define RCC_BASE 0x40021000

// offsets

#define RCC_CR 0x00
#define RCC_CFGR 0x04
#define RCC_AHBENR 0x14
#define RCC_APB2ENR 0x18
#define RCC_APB1ENR 0x1C

#define AFIO_MAPR 0x04

#define GPIO_CRL 0x00
#define GPIO_CRH 0x04
#define GPIO_IDR 0x08
#define GPIO_BSRR 0x10

#define USART_SR 0x00
#define USART_DR 0x04
#define USART_BRR 0x08
#define USART_CR1 0x0C
#define USART_CR2 0x10

#define TIM_CR1 0x00
#define TIM_CCMR1 0x18
#define TIM_CCER 0x20
#define TIM_CNT 0x24
#define TIM_PSC 0x28
#define TIM_ARR 0x2C
#define TIM_CCR1 0x34

// other defines

#define PIN_MODE_IN 0
#define PIN_MODE_OUT 1
#define PIN_MODE_OUT_FAST 3
#define PIN_MODE_OUT_SLOW 2

#define PIN_CNF_I_ANA 0
#define PIN_CNF_I_FLT 1
#define PIN_CNF_I_PULL 2

#define PIN_CNF_O_PP 0
#define PIN_CNF_O_OD 1
#define PIN_CNF_O_APP 2
#define PIN_CNF_O_AOD 3

#endif


