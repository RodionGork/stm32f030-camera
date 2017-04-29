/*
pclk with xclk=12mhz
vsync = 7.5 hz
qcif = 198744 / 1176 pixels
qvga = 264992 / 1882 pixels
*/
.include "macros.i"
.include "stm32f103.i"

.syntax unified
.thumb

.data
camera_buffer:
    .skip 19100
.align


.text
.align

.global init_camera
.thumb_func
init_camera:
    push {lr}
    bl init_camera_pins
    // reset all registers
    ldr r0, = 0x12
    ldr r1, = 0x80
    // scale enabled
    bl camera_write_byte
    ldr r0, = 0x0C
    ldr r1, = 0x08
    // disable double clock
    bl camera_write_byte
    ldr r0, = 0x11
    ldr r1, = 0x82 // or 0x00 for 12 mhz
    // output qcif
    bl camera_write_byte
    ldr r0, = 0x12
    ldr r1, = 0x10 // or 0x08 for QCIF
    bl camera_write_byte
    pop {pc}

.thumb_func
init_camera_pins:
    push {lr}
    pinMode GPIOC_BASE, 14, (PIN_MODE_OUT_SLOW + PIN_CNF_O_PP)
    bl camera_twi_clk_hi
    bl camera_twi_as_output
    ldr r0, = 0
    bl camera_twi_data
    bl camera_twi_delay
    pop {pc}

.thumb_func
camera_twi_as_output:
    push {lr}
    pinMode GPIOC_BASE, 15, (PIN_MODE_OUT_SLOW + PIN_CNF_O_PP)
    pop {pc}

.thumb_func
camera_twi_as_input:
    push {lr}
    pinMode GPIOC_BASE, 15, (PIN_MODE_IN + PIN_CNF_I_FLT)
    pop {pc}

.thumb_func
camera_twi_delay:
    push {r0, lr}
    ldr r0, = 1000
    delay_loop:
    subs r0, 1
    bne delay_loop
    pop {r0, pc}

.thumb_func
camera_twi_clk_hi:
    push {lr}
    store32imm (GPIOC_BASE + GPIO_BSRR), (1 << 14)
    pop {pc}

.thumb_func
camera_twi_clk_lo:
    push {lr}
    store32imm (GPIOC_BASE + GPIO_BSRR), (1 << (14 + 16))
    pop {pc}

.thumb_func
camera_twi_data:
    push {r1, lr}
    ldr r6, = (GPIOC_BASE + GPIO_BSRR)
    ldr r1, = (1 << 15)
    tst r0, 1
    bne camera_twi_data_1
    lsl r1, 16
    camera_twi_data_1:
    str r1, [r6]
    pop {r1, pc}

.thumb_func
camera_twi_read:
    push {lr}
    ldr r6, = (GPIOC_BASE + GPIO_IDR)
    ldr r0, [r6]
    lsr r0, 15
    and r0, 1
    pop {pc}

.thumb_func
camera_twi_start:
    push {lr}
    bl camera_twi_clk_hi
    bl camera_twi_delay
    ldr r0, = 1
    bl camera_twi_data
    bl camera_twi_delay
    ldr r0, = 0
    bl camera_twi_data
    bl camera_twi_delay
    bl camera_twi_clk_lo
    bl camera_twi_delay
    pop {pc}

.thumb_func
camera_twi_stop:
    push {lr}
    ldr r0, = 0
    bl camera_twi_data
    bl camera_twi_delay
    bl camera_twi_clk_hi
    bl camera_twi_delay
    ldr r0, = 1
    bl camera_twi_data
    bl camera_twi_delay
    pop {pc}

.thumb_func
camera_twi_write_byte:
    push {r1, r2, lr}
    mov r2, r0
    ldr r1, = 0x80
    
    write_byte_rep:
    ldr r0, =0
    tst r2, r1
    beq write_byte_1
    ldr r0, =1
    write_byte_1:
    bl camera_twi_data
    bl camera_twi_delay
    bl camera_twi_clk_hi
    bl camera_twi_delay
    bl camera_twi_clk_lo
    lsrs r1, 1
    bcc write_byte_rep
    
    bl camera_twi_as_input
    bl camera_twi_delay
    bl camera_twi_clk_hi
    bl camera_twi_delay
    bl camera_twi_read
    mov r1, r0
    bl camera_twi_clk_lo
    bl camera_twi_delay
    bl camera_twi_as_output
    mov r0, r1
    
    pop {r1, r2, pc}

.thumb_func
camera_twi_read_byte:
    push {r1, r2, lr}
    
    bl camera_twi_as_input
    ldr r2, =0
    ldr r1, =8
    
    read_byte_next_bit:
    bl camera_twi_delay
    bl camera_twi_clk_hi
    bl camera_twi_delay
    bl camera_twi_read
    lsl r2, 1
    orr r2, r0
    bl camera_twi_clk_lo
    subs r1, 1
    bne read_byte_next_bit
    
    bl camera_twi_as_output
    ldr r0, = 1
    bl camera_twi_data
    bl camera_twi_delay
    bl camera_twi_clk_hi
    bl camera_twi_delay
    bl camera_twi_clk_lo
    mov r0, r2
    
    pop {r1, r2, pc}

.thumb_func
camera_write_byte:
    push {r3, lr}
    // r0 is address, r1 is value
    mov r3, r0
    bl camera_twi_start
    ldr r0, = 0x42
    bl camera_twi_write_byte
    mov r0, r3
    bl camera_twi_write_byte
    mov r0, r1
    bl camera_twi_write_byte
    mov r3, r0
    bl camera_twi_stop
    mov r0, r3
    pop {r3, pc}

.thumb_func
camera_read_byte:
    push {r1, lr}
    // r0 is address
    mov r1, r0
    bl camera_twi_start
    ldr r0, = 0x42
    bl camera_twi_write_byte
    mov r0, r1
    bl camera_twi_write_byte
    bl camera_twi_stop
    bl camera_twi_start
    ldr r0, = 0x43
    bl camera_twi_write_byte
    bl camera_twi_read_byte
    mov r1, r0
    bl camera_twi_stop
    mov r0, r1
    pop {r1, pc}

.global camera_capture_frame
.thumb_func
camera_capture_frame:
    push {r1, r3, r4, r7, lr}
    
    ldr r3, = 128
    ldr r7, = camera_buffer
    ldr r6, = (GPIOA_BASE + GPIO_IDR)
    
    wait_vsync_start:
    ldr r0, [r6]
    tst r0, (1 << 11)
    beq wait_vsync_start
    
    wait_vsync_end:
    ldr r0, [r6]
    tst r0, (1 << 11)
    bne wait_vsync_end
    
    next_line:
    
    wait_href_start:
    ldr r0, [r6]
    tst r0, (1 << 12)
    beq wait_href_start
    
    ldr r4, = 128
    
    next_pixel:
    
    wait_pclk_hi1:
    ldr r0, [r6]
    tst r0, (1 << 13)
    beq wait_pclk_hi1
    
    wait_pclk_lo1:
    ldr r0, [r6]
    tst r0, (1 << 13)
    bne wait_pclk_lo1
    
    wait_pclk_hi2:
    ldr r0, [r6]
    tst r0, (1 << 13)
    beq wait_pclk_hi2
    
    strb r0, [r7]
    add r7, 1
    
    wait_pclk_lo2:
    ldr r0, [r6]
    tst r0, (1 << 13)
    bne wait_pclk_lo2
    
    subs r4, 1
    bne next_pixel
    
    wait_href_end:
    ldr r0, [r6]
    tst r0, (1 << 12)
    bne wait_href_end
    
    subs r3, 1
    bne next_line
    
    ldr r0, = '*'
    bl uart_send
    ldr r0, = '\r'
    bl uart_send
    ldr r0, = '\n'
    bl uart_send

    ldr r7, = camera_buffer
    ldr r3, = 0
    ldr r1, = 2
    
    send_pixel:
    ldr r0, [r7, r3]
    ldr r1, = 2
    bl uart_send_hex
    ldr r0, = '.'
    bl uart_send
    add r3, 1
    cmp r3, (128 * 128)
    bne send_pixel
    ldr r0, = '\r'
    bl uart_send
    ldr r0, = '\n'
    bl uart_send
    
    pop {r1, r3, r4, r7, pc}

