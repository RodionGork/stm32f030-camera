.macro store32imm addr, val
    ldr r6, = \addr
    ldr r0, = \val
    str r0, [r6]
.endm

.macro store32reg addr, reg
    ldr r6, = \addr
    str \reg, [r6]
.endm

.macro store32masked addr, mask, val
    ldr r6, = \addr
    ldr r0, [r6]
    and r0, ~ \mask
    orr r0, (\val & \mask)
    str r0, [r6]
.endm

.macro load32r0 addr
    ldr r6, = \addr
    ldr r0, [r6]
.endm

