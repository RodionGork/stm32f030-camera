#include "stm32f030.h"

#include "util.h"

#define CPU_CLOCK_MHZ 48

#define CAPTURE_W 180
#define CAPTURE_H 180

void timer14SetupToggleOutputPB1(unsigned short prescale, unsigned short timeout) {
    REG_L(GPIOB_BASE, GPIO_MODER) &= ~(3 << (1 * 2));
    REG_L(GPIOB_BASE, GPIO_MODER) |= (2 << (1 * 2)); // PB1 alternate function (TIM14_CH1)
    REG_L(GPIOB_BASE, GPIO_AFRL) &= ~(0xF << (1 * 4));
    REG_L(GPIOB_BASE, GPIO_AFRL) |= (0 << (1 * 4)); // PB1 alternate function 0
    REG_L(RCC_BASE, RCC_APB1ENR) |= (1 << 8); // timer 14 enabled
    REG_S(TIM14_BASE, TIM_PSC) = prescale - 1; // divide by prescaler
    REG_S(TIM14_BASE, TIM_ARR) = timeout - 1;
    REG_S(TIM14_BASE, TIM_CCMR1) = 0x30; // toggle output compare
    REG_S(TIM14_BASE, TIM_CCER) = 1; // output compare enable
    REG_S(TIM14_BASE, TIM_CNT) = 0;
    REG_S(TIM14_BASE, TIM_CCR1) = timeout - 1;
    REG_S(TIM14_BASE, TIM_CR1) = 1; // start
}

void twoWireClk(char v) {
    pinOutputA(13, v);
}

void twoWireData(char v) {
    pinOutputA(14, v);
}

void twoWireAsOutput() {
    pinModeOutputA(14);
}

void twoWireAsInput() {
    pinModeInputA(14);
}

char twoWireRead() {
    return pinInputA(14);
}

void twoWireDelay() {
    int n = 50; while (n--) asm("nop");
}

void twoWireStart() {
    twoWireClk(1);
    twoWireDelay();
    twoWireData(1);
    twoWireDelay();
    twoWireData(0);
    twoWireDelay();
    twoWireClk(0);
    twoWireDelay();
}

void twoWireStop() {
    twoWireData(0);
    twoWireDelay();
    twoWireClk(1);
    twoWireDelay();
    twoWireData(1);
    twoWireDelay();
    twoWireRead();
    twoWireDelay();
}

int twoWireWriteByte(unsigned char b) {
    int i;
    for (i = 0; i < 8; i++) {
        twoWireData((b & 0x80) ? 1 : 0);
        b <<= 1;
        twoWireDelay();
        twoWireClk(1);
        twoWireDelay();
        twoWireClk(0);
    }
    twoWireAsInput();
    twoWireDelay();
    twoWireClk(1);
    twoWireDelay();
    i = twoWireRead();
    twoWireClk(0);
    twoWireDelay();
    twoWireAsOutput();
    return i;
}

unsigned char twoWireReadByte() {
    int i;
    unsigned char b = 0;
    twoWireAsInput();
    for (i = 0; i < 8; i++) {
        b <<= 1;
        twoWireDelay();
        twoWireClk(1);
        twoWireDelay();
        b |= twoWireRead();
        twoWireClk(0);
    }
    twoWireAsOutput();
    twoWireData(1);
    twoWireDelay();
    twoWireClk(1);
    twoWireDelay();
    twoWireClk(0);
    return b;
}

void twoWireInit() {
    pinModeOutputA(13);
    pinOutputA(13, 1);
    twoWireAsOutput();
    REG_L(GPIOA_BASE, GPIO_PUPDR) &= ~(3 << (2 * 14));
    REG_L(GPIOA_BASE, GPIO_PUPDR) |= (1 << (2 * 14));
    twoWireDelay();
}

int cameraReadByte(unsigned char addr) {
    unsigned char res;
    twoWireStart();
    twoWireWriteByte(0x42);
    twoWireWriteByte(addr);
    twoWireStop();
    twoWireStart();
    twoWireWriteByte(0x43);
    res = twoWireReadByte();
    twoWireStop();
    return res;
}

int cameraWriteByte(unsigned char addr, unsigned char v) {
    unsigned char res;
    int a = 0;
    twoWireStart();
    a |= twoWireWriteByte(0x42);
    a |= (twoWireWriteByte(addr) << 1);
    a |= (twoWireWriteByte(v) << 2);
    twoWireStop();
}

void cameraInit() {
    cameraWriteByte(0x12, 0x80);
    cameraWriteByte(0x0C, 0x08);
    cameraWriteByte(0x11, 0x82);
    cameraWriteByte(0x12, 0x10);
    uartSends("Camera initialization: ");
    uartSendHex(cameraReadByte(0x0A), 2);
    uartSend('.');
    uartSendHex(cameraReadByte(0x0B), 2);
    uartSend('.');
    uartSendHex(cameraReadByte(0x2A), 2);
    uartSend('.');
    uartSendHex(cameraReadByte(0x2B), 2);
    uartSend('.');
    uartSendHex(cameraReadByte(0x2D), 2);
    uartSend('.');
    uartSendHex(cameraReadByte(0x2E), 2);
    uartSend('\n');
}

void collectFrame() {
    // pa8=pclk, pa11=vscync, pa12=href
    int a, cnt = 0, x, y;
    
    spiEnable(1);
    spiExchange(2);
    spiExchange(0);
    spiExchange(0);
    
    y = CAPTURE_H;
    
    waitVsyncStart:
    a = REG_L(GPIOA_BASE, GPIO_IDR);
    if ((a & (1 << 11)) == 0) goto waitVsyncStart;
    
    waitVsyncEnd:
    a = REG_L(GPIOA_BASE, GPIO_IDR);
    if ((a & (1 << 11)) != 0) goto waitVsyncEnd;
    
    nextLine:
    
    x = CAPTURE_W + 70;
    
    waitHrefStart:
    a = REG_L(GPIOA_BASE, GPIO_IDR);
    if ((a & (1 << 12)) == 0) goto waitHrefStart;
    
    nextPixel:
    
    waitPclkHi1:
    a = REG_L(GPIOA_BASE, GPIO_IDR);
    if ((a & (1 << 8)) == 0) goto waitPclkHi1;
    
    cnt += 1;
    
    waitPclkLo1:
    a = REG_L(GPIOA_BASE, GPIO_IDR);
    if ((a & (1 << 8)) != 0) goto waitPclkLo1;
    
    waitPclkHi2:
    a = REG_L(GPIOA_BASE, GPIO_IDR);
    if ((a & (1 << 8)) == 0) goto waitPclkHi2;
    
    cnt += 1;
    if (x <= CAPTURE_W) {
    REG_B(SPI1_BASE, SPI_DR) = (unsigned char) a;
    }
    
    waitPclkLo2:
    a = REG_L(GPIOA_BASE, GPIO_IDR);
    if ((a & (1 << 8)) != 0) goto waitPclkLo2;
    
    if (--x) goto nextPixel;
    
    waitHrefEnd:
    a = REG_L(GPIOA_BASE, GPIO_IDR);
    if ((a & (1 << 12)) != 0) goto waitHrefEnd;
    
    if (--y) goto nextLine;

    spiEnable(0);
    spiDelay();
    uartSendDec(cnt);
    uartSends("\n!");
    uartSendDec(CAPTURE_W);
    uartSend('.');
    uartSendDec(CAPTURE_H);
    spiEnable(1);
    spiExchange(3);
    spiExchange(0);
    spiExchange(0);
    for (cnt = 0; cnt < CAPTURE_H * CAPTURE_W; cnt++) {
        uartSend('.');
        uartSendHex(spiExchange(0), 2);
    }
    spiEnable(0);
    uartSend('\n');
}

int main() {
    int i;
    setupPll(CPU_CLOCK_MHZ);
    REG_L(RCC_BASE, RCC_AHBENR) |= (1 << 17) | (1 << 18) | (1 << 22); // ports A, B, F enabled
    
    pinModeOutputF(0);
    
    timer14SetupToggleOutputPB1(1, 2);
    
    spiSetup();
    
    twoWireInit();
    
    uartEnable(CPU_CLOCK_MHZ * 1000000 / 460800);
    uartSends("Started...\n");
    
    cameraInit();
    while(1) {
        i = 10000; while (i--);
        pinOutputF(0, 1);
        uartSendHex(cameraReadByte(0x00), 2);
        uartSend('\n');
        collectFrame();
        pinOutputF(0, 0);
    }    
}

