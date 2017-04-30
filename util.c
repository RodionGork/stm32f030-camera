#include "stm32f030.h"

char hex[] = "0123456789ABCDEF";
short adc[8];

void uartEnable(int divisor) {
    REG_L(GPIOA_BASE, GPIO_MODER) &= ~(3 << (9 * 2));
    REG_L(GPIOA_BASE, GPIO_MODER) |= (2 << (9 * 2)); // PA9 alternate function (TX)
    REG_L(GPIOA_BASE, GPIO_AFRH) &= ~(0xF << ((9 - 8) * 4));
    REG_L(GPIOA_BASE, GPIO_AFRH) |= (1 << ((9 - 8) * 4)); // PA9 alternate function 1
    REG_L(RCC_BASE, RCC_APB2ENR) |= (1 << 14); // UART clock
    REG_L(USART_BASE, USART_BRR) |= divisor;
    REG_L(USART_BASE, USART_CR1) |= 1; // UART enable
    //REG_L(USART_BASE, USART_CR1) |= (1 << 2); // UART receive enable
    REG_L(USART_BASE, USART_CR1) |= (1 << 3); // UART transmit enable
    
}

void uartSend(int c) {
    while ((REG_L(USART_BASE, USART_ISR) & (1 << 7)) == 0);
    REG_L(USART_BASE, USART_TDR) = c;
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
    while ((REG_L(RCC_BASE, RCC_CR) & (1 << 25)) != 0);
    REG_L(RCC_BASE, RCC_CFGR) = (boost & 0xF) << 18;
    REG_L(RCC_BASE, RCC_CR) |= (1 << 24); // PLLON = 1
    while ((REG_L(RCC_BASE, RCC_CR) & (1 << 25)) == 0);
    REG_L(RCC_BASE, RCC_CFGR) |= (1 << 1);
    while (((REG_L(RCC_BASE, RCC_CFGR) >> 2) & 0x3) != 2);
}

void pinModeOutputA(int i) {
    REG_L(GPIOA_BASE, GPIO_MODER) &= ~(3 << (i * 2));
    REG_L(GPIOA_BASE, GPIO_MODER) |= (1 << (i * 2));
}

void pinOutputA(int i, char v) {
    if (v == 0) {
        i += 16;
    }
    REG_L(GPIOA_BASE, GPIO_BSRR) = (1 << i);
}

void pinModeInputA(int i) {
    REG_L(GPIOA_BASE, GPIO_MODER) &= ~(3 << (i * 2));
}

char pinInputA(int i) {
    return (char) ((REG_L(GPIOA_BASE, GPIO_IDR) >> i) & 1);
}

void pinModeOutputB(int i) {
    REG_L(GPIOB_BASE, GPIO_MODER) &= ~(3 << (i * 2));
    REG_L(GPIOB_BASE, GPIO_MODER) |= (1 << (i * 2));
}

void pinOutputB(int i, char v) {
    if (v == 0) {
        i += 16;
    }
    REG_L(GPIOB_BASE, GPIO_BSRR) = (1 << i);
}

void pinModeInputB(int i) {
    REG_L(GPIOB_BASE, GPIO_MODER) &= ~(3 << (i * 2));
}

char pinInputB(int i) {
    return (char) ((REG_L(GPIOB_BASE, GPIO_IDR) >> i) & 1);
}

void pinModeOutputF(int i) {
    REG_L(GPIOF_BASE, GPIO_MODER) &= ~(3 << (i * 2));
    REG_L(GPIOF_BASE, GPIO_MODER) |= (1 << (i * 2));
}

void pinOutputF(int i, char v) {
    if (v == 0) {
        i += 16;
    }
    REG_L(GPIOF_BASE, GPIO_BSRR) = (1 << i);
}

void spiDelay() {
    int n = 50;
    while (n--) {
        asm("nop");
    }
}

void spiEnable(int v) {
    pinOutputB(6, v ? 0 : 1);
    spiDelay();
}

int spiExchange(int v) {
    while ((REG_B(SPI1_BASE, SPI_SR) & 2) == 0);
    REG_B(SPI1_BASE, SPI_DR) = v;
    while ((REG_B(SPI1_BASE, SPI_SR) & 1) == 0);
    return REG_B(SPI1_BASE, SPI_DR);
}

void spiSetup() {
    int i;
    REG_L(RCC_BASE, RCC_APB2ENR) |= (1 << 12); // spi1 enabled
    for (i = 3; i <= 5; i++) {
        REG_L(GPIOB_BASE, GPIO_MODER) &= ~(3 << (i * 2));
        REG_L(GPIOB_BASE, GPIO_MODER) |= (2 << (i * 2)); // PBi alternate function
        REG_L(GPIOB_BASE, GPIO_AFRL) &= ~(0xF << (i * 4));
        REG_L(GPIOB_BASE, GPIO_AFRL) |= (0 << (i * 4)); // PBi alternate function 0
    }
    pinModeOutputB(6);
    pinOutputB(6, 1);
    REG_L(SPI1_BASE, SPI_CR2) = 0x1700;
    REG_L(SPI1_BASE, SPI_CR1) = 0x34C;
    
    spiDelay();
    spiEnable(1);
    spiExchange(1);
    spiExchange(0x41);
    spiEnable(0);
    spiDelay();
}

