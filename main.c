#include "stm32f030.h"

#include "util.h"

int main() {
    setupPll(20);
    int n, i;
    REG_L(RCC_BASE, RCC_AHBENR) |= (1 << 17);
    
    REG_L(GPIOA_BASE, GPIO_MODER) |= 1;
    
    uartEnable(20000000 / 921600);
    uartSends("Started...");
    
    i = 0;
    
    while(1) {
        REG_L(GPIOA_BASE, GPIO_BSRR) = (1 << 0);
        n=220000; while(--n);
        REG_L(GPIOA_BASE, GPIO_BSRR) = (1 << 16);
        n=1600000; while(--n);
        uartSendDec(i);
        uartSends("\n");
        i += 1;
    }    
}

