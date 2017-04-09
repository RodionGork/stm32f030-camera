#include "stm32f103.h"

char hex[] = "0123456789ABCDEF";

void pinMode(int base, char num, char mode, char cnf) {
    int* p = (int*) (void*) (base + (num < 8 ? GPIO_CRL : GPIO_CRH));
    num &= 0x7;
    char offs = num * 4;
    int v = *p;
    v &= ~(0xF << offs);
    v |= (mode | (cnf << 2)) << offs;
    *p = v;
}

void pinOutput(int base, char num, char v) {
    if (v == 0) {
        num += 16;
    }
    REG_L(base, GPIO_BSRR) |= 1 << num;
}

char pinInput(int base, char num) {
    return (REG_L(base, GPIO_IDR) >> num) & 1;
}

void uartEnable(int divisor) {
    REG_L(RCC_BASE, RCC_APB2ENR) |= (1 << 0); // AFIO clock
    pinMode(GPIOA_BASE, 9, PIN_MODE_OUT, PIN_CNF_O_APP);
    REG_L(AFIO_BASE, AFIO_MAPR) &= ~(1 << 2); // no UART1 remap
    REG_L(RCC_BASE, RCC_APB2ENR) |= (1 << 14); // UART clock
    REG_L(USART_BASE, USART_BRR) |= divisor;
    REG_L(USART_BASE, USART_CR1) |= (1 << 13); // UART enable
    REG_L(USART_BASE, USART_CR1) |= (1 << 3); // UART transmit enable
    
}

void uartSend(int c) {
    while ((REG_L(USART_BASE, USART_SR) & (1 << 7)) == 0);
    REG_L(USART_BASE, USART_DR) = c;
}

void uartSends(char* s) {
    while (*s) {
        uartSend(*(s++));
    }
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
        x1 = x / 10;
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

