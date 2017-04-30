#include "stm32f030.h"

#include "util.h"

#define PIN_INPUT_A(X) ((REG_B(GPIOA_BASE, GPIO_IDR) >> (X)) & 1)

#define CAMERA_VSYNC (PIN_INPUT_A(5))
#define CAMERA_HREF (PIN_INPUT_A(2))
#define CAMERA_PCLK (PIN_INPUT_A(6))

void timer14SetupToggleOutput(unsigned short prescale, unsigned short timeout) {
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
}

void spiDelay() {
    int n = 500000;
    while (n--) {
        asm("nop");
    }
}

int spiExchange(int v) {
    while ((REG_B(SPI1_BASE, SPI_SR) & 2) == 0);
    //spiDelay();
    REG_B(SPI1_BASE, SPI_DR) = v;
    while ((REG_B(SPI1_BASE, SPI_SR) & 1) == 0);
    //spiDelay();
    return REG_B(SPI1_BASE, SPI_DR);
}

void spiEnable(int v) {
    pinOutputB(6, v ? 0 : 1);
}

int spiExch(int v) {
    int m = 0x80;
    int r = 0;
    while (m) {
        pinOutputB(5, (v & m) ? 1 : 0);
        m >>= 1;
        spiDelay();
        pinOutputB(3, 1);
        r = (r << 1) | ((REG_B(GPIOB_BASE, GPIO_IDR) >> 4) & 1);
        spiDelay();
        pinOutputB(3, 0);
    }
    spiDelay();
    return r;
}

void spiTest() {
    int nss = 6, sck = 3;
    int n, v, m;
    pinModeOutputB(6);
    pinOutputB(6, 1);
    
    pinModeOutputB(3);
    pinOutputB(3, 0);
    pinModeOutputB(5);
    pinOutputB(5, 1);
    spiDelay();
    pinOutputB(6, 0);
    
    spiDelay();
    uartSendHex(spiExch(1), 2);
    uartSendHex(spiExch(0x41), 2);
    pinOutputB(6, 1);
    spiDelay();
    pinOutputB(6, 0);
    
    uartSendHex(spiExch(5), 2);
    uartSendHex(spiExch(0), 2);
    uartSends("\r\n----\r\n");
}

int main() {
    char vsyncPrev, vsync, n;
    setupPll(48);
    REG_L(RCC_BASE, RCC_AHBENR) |= (1 << 17) | (1 << 18) | (1 << 22);
    
//    pinModeOutputB(1);
//    twoWireInit();
    
    uartEnable(48000000 / 115200);
    uartSends("Started...\n");
    
    spiSetup();
    
    spiEnable(1);
    spiExchange(1);
    spiExchange(0x41);
    spiEnable(0);
    spiDelay();
    
    /*
    spiEnable(1);
    spiExchange(5);
    uartSendHex(spiExchange(0), 2);
    spiEnable(0);
    spiDelay();
    */
    
    spiEnable(1);
    spiExchange(2);
    spiExchange(0);
    spiExchange(0);
    spiExchange(0x31);
    spiExchange(0x41);
    spiExchange(0x59);
    spiEnable(0);
    spiDelay();
    
    spiEnable(1);
    spiExchange(3);
    spiExchange(0);
    spiExchange(0);
    for (n = 0; n < 8; n++) {
        uartSendHex(spiExchange(0), 2);
    }
    spiEnable(0);
    spiDelay();
    
    uartSends("\r\n");
    
    //spiTest();
    
    timer14SetupToggleOutput(48000, 1000);
    int i;
    while(1) {
        i = 1000000;
        while (i--);
        uartSendHex(REG_L(TIM14_BASE, TIM_CNT), 4);
        uartSends("\r\n");
    }    
}

