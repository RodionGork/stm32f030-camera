arm-linux-gnueabi-gcc -mcpu=cortex-m0 -mthumb -g -c init.c -o init.o
arm-linux-gnueabi-gcc -mcpu=cortex-m0 -mthumb -g -c util.c -o util.o
arm-linux-gnueabi-gcc -mcpu=cortex-m0 -mthumb -g -c main.c -o main.o
arm-linux-gnueabi-ld -T stm32f030-16k.ld -nostdlib -Map=test.map -o test.elf main.o init.o util.o
arm-linux-gnueabi-objcopy test.elf -O ihex test.hex
rm *.o
rm *.map
rm *.elf
