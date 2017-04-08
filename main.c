#include "stm32f030.h"

#include "util.h"

#define PIN_INPUT_A(X) ((REG_B(GPIOA_BASE, GPIO_IDR) >> (X)) & 1)

#define CAMERA_VSYNC (PIN_INPUT_A(5))
#define CAMERA_HREF (PIN_INPUT_A(2))
#define CAMERA_PCLK (PIN_INPUT_A(6))

void timer17SetupToggleOutput(unsigned short prescale, unsigned short timeout) {
    REG_L(GPIOA_BASE, GPIO_MODER) &= ~(3 << (7 * 2));
    REG_L(GPIOA_BASE, GPIO_MODER) |= (2 << (7 * 2)); // PA6 alternate function (TIM17_CH1)
    REG_L(GPIOA_BASE, GPIO_AFRL) &= ~(0xF << (7 * 4));
    REG_L(GPIOA_BASE, GPIO_AFRL) |= (5 << (7 * 4)); // PA6 alternate function 5
    REG_L(RCC_BASE, RCC_APB2ENR) |= (1 << 18); // timer 16 enabled
    REG_S(TIM17_BASE, TIM_PSC) = prescale - 1; // divide by prescaler
    REG_S(TIM17_BASE, TIM_ARR) = timeout - 1;
    REG_S(TIM17_BASE, TIM_CCMR1) = 0x30; // toggle output compare
    REG_S(TIM17_BASE, TIM_CCER) = 1; // output compare enable
    REG_S(TIM17_BASE, TIM_CNT) = 0;
    REG_S(TIM17_BASE, TIM_CCR1) = timeout - 1;
    REG_S(TIM17_BASE, TIM_BDTR) = 0xC000; // disable output override
    REG_S(TIM17_BASE, TIM_CR1) = 1; // start
}

void pinModeOutputA(int i) {
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
    REG_L(GPIOF_BASE, GPIO_MODER) |= (1 << (i * 2));
}

void pinOutputF(int i, char v) {
    if (v == 0) {
        i += 16;
    }
    REG_L(GPIOF_BASE, GPIO_BSRR) = (1 << i);
}

void pinModeInputF(int i) {
    REG_L(GPIOF_BASE, GPIO_MODER) &= ~(3 << (i * 2));
}

char pinInputF(int i) {
    return (char) ((REG_L(GPIOF_BASE, GPIO_IDR) >> i) & 1);
}

void twoWireClk(char v) {
    pinOutputF(1, v);
}

void twoWireData(char v) {
    pinOutputF(0, v);
}

void twoWireAsOutput() {
    pinModeOutputF(0);
}

void twoWireAsInput() {
    pinModeInputF(0);
}

char twoWireRead() {
    return pinInputF(0);
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
    pinModeOutputF(1);
    pinOutputF(1, 1);
    twoWireAsOutput();
    REG_L(GPIOF_BASE, GPIO_PUPDR) |= (1 << 2 * 0);
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
    cameraWriteByte(0x11, 0x00);
    cameraWriteByte(0x12, 0x08);
    uartSends("Camera initialization: ");
    uartSendHex(cameraReadByte(0x0A), 2);
    uartSend('.');
    uartSendHex(cameraReadByte(0x0B), 2);
    uartSend('\n');
}

unsigned char line[640];

void collectFrame() {
    int y;
    char sample, prev, cur;
    int pixCnt = 0;
    short lptr, i;
    for (y = 0; y < 144; y++) {
        do {
            sample = REG_B(GPIOA_BASE, GPIO_IDR);
        } while ((sample & 0x10) == 0);
        prev = 0;
        lptr = 0;
        do {
            
            cur = (sample & 0x40);
            if (cur != 0 && prev == 0) {
                pixCnt += 1;
                line[lptr++] = (sample & 0xF);
            }
            prev = cur;
            
            sample = REG_B(GPIOA_BASE, GPIO_IDR);
        } while ((sample & 0x10) != 0);
        for (i = 1; i < 176 * 2; i += 2) {
            uartSendHex(line[i], 1);
        }
        uartSend('.');
    }
    uartSends("---\r\n");
    uartSendDec(pixCnt);
    uartSends("---\r\n");
}

int main() {
    char vsyncPrev, vsync, n;
    setupPll(48);
    REG_L(RCC_BASE, RCC_AHBENR) |= (1 << 17) | (1 << 18) | (1 << 22);
    
    pinModeOutputB(1);
    twoWireInit();
    
    //uartEnable(48000000 / 921600);
    uartEnable(48000000 / 3000000);
    uartSends("Started...\n");
    
    
    timer17SetupToggleOutput(1, 3);
    
    cameraInit();
    vsyncPrev = 0;
    n = 0;
    while(1) {
        while (vsyncPrev == 0 || vsync != 0) {
            vsyncPrev = vsync;
            vsync = REG_B(GPIOA_BASE, GPIO_IDR) & 0x20;
        }
        collectFrame();
        n += 1;
        pinOutputB(1, n & 1);
        vsyncPrev = 0;
    }    
}

