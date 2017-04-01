#include "stm32f030.h"

char hex[] = "0123456789ABCDEF";
short adc[8];

void uartEnable(int divisor) {
    REG_L(GPIOA_BASE, GPIO_MODER) &= ~(3 << (9 * 2));
    REG_L(GPIOA_BASE, GPIO_MODER) |= (2 << (9 * 2)); // PA9 alternate function (TX)
    REG_L(GPIOA_BASE, GPIO_AFRH) &= ~(0xF << ((9 - 8) * 4));
    REG_L(GPIOA_BASE, GPIO_AFRH) |= (1 << ((9 - 8) * 4)); // PA9 alternate function 1
    REG_L(RCC_BASE, RCC_AHB2ENR) |= (1 << 14); // UART clock
    REG_L(USART_BASE, USART_BRR) |= divisor;
    REG_L(USART_BASE, USART_CR1) |= 1; // UART enable
    //REG_L(USART_BASE, USART_CR1) |= (1 << 2); // UART receive enable
    REG_L(USART_BASE, USART_CR1) |= (1 << 3); // UART transmit enable
    
}

void adcEnable() {
    REG_L(GPIOA_BASE, GPIO_MODER) |= (3 << (5 * 2)); // PA5 as analog input
    
    REG_L(ADC_BASE, ADC_CR) &= ~1;
    REG_L(ADC_BASE, ADC_CR) |= (1 << 31); // start calibration
    while ((REG_L(ADC_BASE, ADC_CR) & (1 << 31)) != 0);
    
    REG_L(ADC_BASE, ADC_CR) |= 1;
    while ((REG_L(ADC_BASE, ADC_ISR) & 1) == 0);
    
    REG_L(ADC_BASE, ADC_SMPR) |= 7;
    
    REG_L(ADC_BASE, ADC_CCR) |= (1 << 22); //enable vrefint
}

void adcRead(int channels) {
    int i;
    for (i = 0; i < channels; i++) {
        REG_L(ADC_BASE, ADC_CR) |= (1 << 2);
        while ((REG_L(ADC_BASE, ADC_ISR) & (1 << 2)) == 0);
        adc[i] = REG_L(ADC_BASE, ADC_DR);
    }
    REG_L(ADC_BASE, ADC_ISR) |= (1 << 3);
}

void uartSend(int c) {
    REG_L(USART_BASE, USART_TDR) = c;
    while ((REG_L(USART_BASE, USART_ISR) & (1 << 6)) == 0);
}

void uartSends(char* s) {
    while (*s) {
        uartSend(*(s++));
    }
}

int intDiv(int a, int b) {
	int res = 0;
	int power = 1;
	while (a - b >= b) {
		b <<= 1;
		power <<= 1;
	}
	while (power > 0) {
		if (a - b >= 0) {
			a -= b;
			res += power;
		}
		b >>= 1;
		power >>= 1;
	}
	return res;
}

void uartSendHex(int x, int d) {
    while (d-- > 0) {
        uartSend(hex[(x >> (d * 4)) & 0xF]);
    }
}

void uartSendDec(int x) {
    static char s[10];
    int i, x1;
    i = 0;
    while (x > 0) {
        x1 = intDiv(x, 10);
        s[i++] = x - x1 * 10;
        x = x1;
    }
    if (i == 0) {
        s[i++] = 0;
    }
    while (i > 0) {
        uartSend('0' + s[--i]);
    }
}

void setupPll(int mhz) {
    int boost = mhz / 4 - 2;
    REG_L(RCC_BASE, RCC_CR) &= ~(1 << 24); // PLLON = 0
    while ((REG_L(RCC_BASE, RCC_CR) | (1 << 25)) == 0);
    REG_L(RCC_BASE, RCC_CFGR) = (boost & 0xF) << 18;
    REG_L(RCC_BASE, RCC_CR) |= (1 << 24); // PLLON = 1
    while ((REG_L(RCC_BASE, RCC_CR) | (1 << 25)) == 0);
    REG_L(RCC_BASE, RCC_CFGR) |= (1 << 1);
    while (((REG_L(RCC_BASE, RCC_CFGR) >> 2) & 0x3) != 2);
}

