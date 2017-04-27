.include "macros.i"
.include "stm32f103.i"

.syntax unified
.thumb

.equ CPU_CLK_MHZ, 48

.section .vectors
.align
.long __stack_top
.long reset_handler

.text
.align

crlf_str:
    .asciz "\r\n"
.align

.thumb_func
reset_handler:
    
    ldr r0, = CPU_CLK_MHZ
    bl init_with_pll
    
    initAfioAndGpios 0x7
    
    ldr r0, = (CPU_CLK_MHZ * 1000000 / 921600)
    bl init_uart
    
    ldr r1, = 1
    ldr r0, = 2
    bl init_timer2_toggling
    
    ldr r0, = 100000
    bl delay_r0
    
    bl init_camera
    
    pinMode GPIOB_BASE, 7, (PIN_MODE_OUT + PIN_CNF_O_PP)
    
    ldr r3, = 0
    
    blink:

    pinOutput GPIOB_BASE, 7, 1
    ldr r0, = 0x100000
    bl delay_r0
    pinOutput GPIOB_BASE, 7, 0
    ldr r0, = 0x300000
    bl delay_r0
    
    bl camera_capture_frame
    
    b blink

.thumb_func
delay_r0:
    push {lr}
    delay_loop:
    subs r0, 1
    bne delay_loop
    pop {pc}

.thumb_func
init_timer2_toggling:
    push {r1, lr}
    // r0 - timeout, r1 - prescaler
    sub r1, 1
    sub r0, 1
    push {r0}
    
    // timer 2 pins partially remapped
    store32masked (AFIO_BASE + AFIO_MAPR), (3 << 8), (1 << 8)
    pinMode GPIOA_BASE, 15, (PIN_MODE_OUT_FAST + PIN_CNF_O_APP)
    store32masked (RCC_BASE + RCC_APB1ENR), (1 << 0), (1 << 0)
    store32reg (TIM2_BASE + TIM_PSC), r1
    pop {r0}
    store32reg (TIM2_BASE + TIM_ARR), r0
    store32reg (TIM2_BASE + TIM_CCR1), r0
    store32imm (TIM2_BASE + TIM_CNT), 0
    store32imm (TIM2_BASE + TIM_CCMR1), 0x30
    store32imm (TIM2_BASE + TIM_CCER), 1
    store32imm (TIM2_BASE + TIM_CR1), 1
    
    pop {r1, pc}

