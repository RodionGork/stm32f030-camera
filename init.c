void init(void);
int main(void);
void start(void)
{
    init();
}
void Default_Handler(void);

// The following are 'declared' in the linker script
extern unsigned char  INIT_DATA_VALUES;
extern unsigned char  INIT_DATA_START;
extern unsigned char  INIT_DATA_END;
extern unsigned char  BSS_START;
extern unsigned char  BSS_END;
extern unsigned char  BSS_END;
// the section "vectors" is placed at the beginning of flash 
// by the linker script
const void * Vectors[] __attribute__((section(".vectors"))) ={
    (void *)0x20001000,     /* Top of stack */ 
    init,           /* Reset Handler */
    Default_Handler,    /* NMI */
    Default_Handler,    /* Hard Fault */
    0,                    /* Reserved */
    0,                       /* Reserved */
    0,                       /* Reserved */
    0,                       /* Reserved */
    0,                       /* Reserved */
    0,                       /* Reserved */
    0,                       /* Reserved */
    Default_Handler,    /* SVC */
    0,                       /* Reserved */
    0,                       /* Reserved */
    Default_Handler,         /* PendSV */
    Default_Handler,         /* SysTick */        
    /* External interrupt handlers follow */
    Default_Handler,     /* PIO0_0 */
    Default_Handler,     /* PIO0_1 */
    Default_Handler,     /* PIO0_2 */
    Default_Handler,     /* PIO0_3 */
    Default_Handler,     /* PIO0_4 */
    Default_Handler,     /* PIO0_5 */
    Default_Handler,     /* PIO0_6 */
    Default_Handler,     /* PIO0_7 */
    Default_Handler,     /* PIO0_8 */
    Default_Handler,     /* PIO0_9 */
    Default_Handler,     /* PIO0_10 */
    Default_Handler,     /* PIO0_11 */
    Default_Handler,    /* PIO1_0 */
    Default_Handler,      /* C_CAN */
    Default_Handler,     /* SSP1 */
    Default_Handler,     /* I2C */
    Default_Handler,     /* CT16B0 */
    Default_Handler,     /* CT16B1 */
    Default_Handler,     /* CT32B0 */
    Default_Handler,     /* CT32B1 */
    Default_Handler,     /* SSP0 */
    Default_Handler,    /* UART */
    Default_Handler,     /* RESERVED */
    Default_Handler,     /* RESERVED */
    Default_Handler,     /* ADC */
    Default_Handler,     /* WDT */
    Default_Handler,     /* BOD */
    Default_Handler,     /* RESERVED */
    Default_Handler,     /* PIO3 */
    Default_Handler,     /* PIO2 */
    Default_Handler,     /* PIO1 */
    Default_Handler     /* PIO0 */
};

void init()
{
// do global/static data initialization
    unsigned char *src;
    unsigned char *dest;
    unsigned len;
    src= &INIT_DATA_VALUES;
    dest= &INIT_DATA_START;
    len= &INIT_DATA_END-&INIT_DATA_START;
    while (len--)
        *dest++ = *src++;
// zero out the uninitialized global/static variables
    dest = &BSS_START;
    len = &BSS_END - &BSS_START;
    while (len--)
        *dest++=0;
    main();
}

void Default_Handler()
{
    while(1);
}
