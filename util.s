.include "macros.i"
.include "stm32f103.i"

.syntax unified
.thumb

.data
uart_buffer:
    .skip 11
uart_buffer_end:
    .skip 1

.text
.align

.global init_with_pll
.thumb_func
init_with_pll:
    push {r1, lr}
    
    // r0 is desired frequency in MHz
    lsr r0, 2
    sub r1, r0, 2
    and r1, 0xF
    lsl r1, 18
    
    // disable pll
    store32masked (RCC_BASE + RCC_CR), (1 << 24), 0
    wait_pll_stopped:
    load32r0 (RCC_BASE + RCC_CR)
    tst r0, (1 << 25)
    bne wait_pll_stopped
    
    // load multiplier
    ldr r6, = (RCC_BASE + RCC_CFGR)
    ldr r0, [r6]
    and r0, ~(0xF << 18)
    orr r0, r1
    str r0, [r6]
    
    // enable pll
    store32masked (RCC_BASE + RCC_CR), (1 << 24), (1 << 24)
    wait_pll_running:
    load32r0 (RCC_BASE + RCC_CR)
    tst r0, (1 << 25)
    beq wait_pll_running
    
    // switch to pll
    store32masked (RCC_BASE + RCC_CFGR), 0x03, 2
    wait_pll_2:
    load32r0 (RCC_BASE + RCC_CFGR)
    and r0, (0x3 << 2)
    cmp r0, (2 << 2)
    bne wait_pll_2
    
    pop {r1, pc}

.global init_uart
.thumb_func
init_uart:
    push {lr}
    
    // r0 holds the divisor
    push {r0}
    
    // enable output pin
    pinMode GPIOA_BASE, 9, (PIN_MODE_OUT + PIN_CNF_O_APP)
    // no pin remapping
    store32masked (AFIO_BASE + AFIO_MAPR), (1 << 2), 0
    // enable usart clock
    store32masked (RCC_BASE + RCC_APB2ENR), (1 << 14), (1 << 14)
    
    ldr r6, = (USART_BASE + USART_BRR)
    pop {r0}
    str r0, [r6]
    
    // enable usart
    store32masked (USART_BASE + USART_CR1), (1 << 13), (1 << 13)
    // enable transmitter
    store32masked (USART_BASE + USART_CR1), (1 << 3), (1 << 3)
    
    pop {pc}

.global uart_send
.thumb_func
uart_send:
    push {r1, lr}
    
    ldr r6, = (USART_BASE + USART_SR)
    uart_send_1:
    ldr r1, [r6]
    ands r1, (1 << 7)
    beq uart_send_1
    
    ldr r6, = (USART_BASE + USART_DR)
    str r0, [r6]
    
    pop {r1, pc}

.global uart_send_str
.thumb_func
uart_send_str:
    push {r1, lr}
    mov r1, r0
    uart_send_str_next:
    ldrb r0, [r1]
    tst r0, r0
    beq uart_send_str_done
    bl uart_send
    add r1, 1
    b uart_send_str_next
    uart_send_str_done:
    pop {r1, pc}

.global uart_send_hex
.thumb_func
uart_send_hex:
    push {r2, lr}
    
    ldr r6, = uart_buffer
    ldr r2, = 0
    strb r2, [r6, r1]
    
    uart_send_hex_1:
    sub r1, 1
    and r2, r0, 0xF
    add r2, '0'
    cmp r2, '9'
    bls uart_send_hex_2
    add r2, 'A' - ('9' + 1)
    uart_send_hex_2:
    lsr r0, 4
    strb r2, [r6, r1]
    tst r1, r1
    bne uart_send_hex_1
    
    ldr r0, = uart_buffer
    bl uart_send_str
    
    pop {r2, pc}

.global uart_send_dec
.thumb_func
uart_send_dec:
    push {r1, r2, r3, r4, lr}
    
    ldr r6, = uart_buffer_end
    ldr r4, = 0
    strb r4, [r6]
    ldr r4, = 10
    
    uart_send_dec_1:
    mov r2, r0
    udiv r0, r4
    mul r3, r0, r4
    sub r3, r2, r3
    add r3, '0'
    sub r6, 1
    strb r3, [r6]
    tst r0, r0
    bne uart_send_dec_1
    
    mov r0, r6
    bl uart_send_str
    
    pop {r1, r2, r3, r4, pc}
    
