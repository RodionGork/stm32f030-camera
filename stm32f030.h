#ifndef __STM32F030_H_
#define __STM32F030_H_

#define REG_L(X,Y) ((volatile long*)((void*)((X) + (Y))))[0]
#define REG_S(X,Y) ((volatile unsigned short*)((void*)((X) + (Y))))[0]
#define REG_B(X,Y) ((volatile unsigned char*)((void*)((X) + (Y))))[0]

#define RCC_BASE 0x40021000

#define GPIOA_BASE 0x48000000
#define GPIOB_BASE 0x48000400
#define GPIOF_BASE 0x48001400

#define USART_BASE 0x40013800
#define ADC_BASE 0x40012400
#define SPI1_BASE 0x40013000

#define TIM1_BASE 0x40012C00
#define TIM3_BASE 0x40000400
#define TIM14_BASE 0x40002000
#define TIM16_BASE 0x40014400
#define TIM17_BASE 0x40014800

#define RCC_CR 0x00
#define RCC_CFGR 0x04
#define RCC_AHBENR 0x14
#define RCC_AHB2ENR 0x18
#define RCC_AHB1ENR 0x1C
#define RCC_APB2ENR RCC_AHB2ENR
#define RCC_APB1ENR RCC_AHB1ENR

#define GPIO_MODER 0x00
#define GPIO_PUPDR 0x0C
#define GPIO_IDR 0x10
#define GPIO_BSRR 0x18
#define GPIO_AFRL 0x20
#define GPIO_AFRH 0x24

#define ADC_ISR 0x00
#define ADC_CR 0x08
#define ADC_CCR 0x308
#define ADC_DR 0x40
#define ADC_SMPR 0x14

#define ADC_CHSELR 0x28

#define SPI_CR1 0x00
#define SPI_CR2 0x04
#define SPI_SR 0x08
#define SPI_DR 0x0C

#define USART_CR1 0x00
#define USART_BRR 0x0C
#define USART_TDR 0x28
#define USART_ISR 0x1C

#define TIM_CR1 0x00
#define TIM_SMCR 0x08
#define TIM_EGR 0x14
#define TIM_CCMR1 0x18
#define TIM_CCER 0x20
#define TIM_CNT 0x24
#define TIM_PSC 0x28
#define TIM_ARR 0x2C
#define TIM_CCR1 0x34
#define TIM_BDTR 0x44
#define TIM_OR 0x50

#endif


