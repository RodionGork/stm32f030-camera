#include "stm32f103.h"

#include "util.h"

#define CLOCK_MHZ 36

#define FRAME_START 17
#define FRAME_WIDTH 120

#define PIN_INPUT_A(X) ((REG_B(GPIOA_BASE, GPIO_IDR) >> (X)) & 1)

#define CAMERA_VSYNC (PIN_INPUT_A(5))
#define CAMERA_HREF (PIN_INPUT_A(2))
#define CAMERA_PCLK (PIN_INPUT_A(6))

void timer2SetupToggleOutput(unsigned short prescale, unsigned short timeout) {
    REG_L(RCC_BASE, RCC_APB2ENR) |= (1 << 0); // AFIO clock
    REG_L(AFIO_BASE, AFIO_MAPR) &= ~(7 << 24); // JTAG/SW bits clear
    REG_L(AFIO_BASE, AFIO_MAPR) |= 4 << 24; // disable JTAG/SW
    REG_L(AFIO_BASE, AFIO_MAPR) &= ~(3 << 8); // no TIM2 remap
    REG_L(AFIO_BASE, AFIO_MAPR) |= 1 << 8; // TIM2 CH1/2 remapped
    pinMode(GPIOA_BASE, 15, PIN_MODE_OUT, PIN_CNF_O_APP);
    REG_L(RCC_BASE, RCC_APB1ENR) |= (1 << 0); // timer 2 enabled
    REG_S(TIM2_BASE, TIM_PSC) = prescale - 1; // divide by prescaler
    REG_S(TIM2_BASE, TIM_ARR) = timeout - 1;
    REG_S(TIM2_BASE, TIM_CNT) = 0;
    REG_S(TIM2_BASE, TIM_CCR1) = timeout - 1;
    REG_S(TIM2_BASE, TIM_CCMR1) = 0x30; // toggle output compare
    REG_S(TIM2_BASE, TIM_CCER) = 1; // output compare enable
    REG_S(TIM2_BASE, TIM_CR1) = 1; // start

}

void twoWireClk(char v) {
    pinOutput(GPIOC_BASE, 14, v);
}

void twoWireData(char v) {
    pinOutput(GPIOC_BASE, 15, v);
}

void twoWireAsOutput() {
    pinMode(GPIOC_BASE, 15, PIN_MODE_OUT_SLOW, PIN_CNF_O_PP);
}

void twoWireAsInput() {
    pinMode(GPIOC_BASE, 15, PIN_MODE_IN, PIN_CNF_I_FLT);
}

char twoWireRead() {
    return pinInput(GPIOC_BASE, 15);
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
    pinMode(GPIOC_BASE, 14, PIN_MODE_OUT_SLOW, PIN_CNF_O_PP);
    pinOutput(GPIOC_BASE, 14, 1);
    twoWireAsOutput();
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
    cameraWriteByte(0x0C, 0x08);
    cameraWriteByte(0x11, 0x00);
    cameraWriteByte(0x12, 0x08);
    uartSends("Camera initialization: ");
    uartSendHex(cameraReadByte(0x0A), 2);
    uartSend('.');
    uartSendHex(cameraReadByte(0x0B), 2);
    uartSend('\n');
}

unsigned char frame[144*120];
unsigned char line[640];

void collectFrame() {
    int x, y;
    char sample, prev, cur;
    int pixCnt = 0;
    short lptr, i, fptr;
    fptr = 0;
    for (y = 0; y < 144; y++) {
        do {
            sample = REG_B(GPIOB_BASE, GPIO_IDR);
        } while ((sample & 0x02) == 0);
        prev = 0;
        lptr = 0;
        do {
            sample = REG_B(GPIOB_BASE, GPIO_IDR);
            cur = (sample & 0x01);
            if (cur != 0 && prev == 0) {
                line[lptr++] = REG_B(GPIOA_BASE, GPIO_IDR);
                pixCnt += 1;
            }
            prev = cur;
        } while ((sample & 0x02) != 0);
        for (i = FRAME_START * 2 + 1; i < (FRAME_START + FRAME_WIDTH) * 2 + 1; i += 2) {
            frame[fptr++] = line[i];
        }
    }
    uartSends("FRAME: ");
    uartSendDec(pixCnt);
    uartSends("\r\n");
    fptr = 0;
    for (y = 0; y < 144; y++) {
        uartSendDec(y);
        uartSends(":");
        for (x = 0; x < FRAME_WIDTH; x++) {
            uartSend(' ');
            uartSendDec(frame[fptr++]);
        }
        uartSends("\r\n");
    }
}

int main() {
    char vsyncPrev, vsync;
    int n;
    setupPll(CLOCK_MHZ);
    REG_L(RCC_BASE, RCC_APB2ENR) |= (1 << 2) | (1 << 3) | (1 << 4); // clock to ports A/B/C
    
    pinMode(GPIOB_BASE, 7, PIN_MODE_OUT_SLOW, PIN_CNF_O_PP);
    twoWireInit();
    
    timer2SetupToggleOutput(1, 3);
    
    uartEnable(CLOCK_MHZ * 1000000 / 921600);
    uartSends("Started...\n");
    
    cameraInit();
    
    vsyncPrev = 0;
    n = 0;
    while(1) {
        while (vsyncPrev == 0 || vsync != 0) {
            vsyncPrev = vsync;
            vsync = REG_B(GPIOB_BASE, GPIO_IDR) & 0x04;
        }
        collectFrame();
        n += 1;
        pinOutput(GPIOB_BASE, 7, n & 1);
        vsyncPrev = 0;
    }    
}

