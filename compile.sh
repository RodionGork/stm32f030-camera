arm-linux-gnueabi-as -mcpu=cortex-m3 -mthumb -g -c main.s -o main.o
arm-linux-gnueabi-as -mcpu=cortex-m3 -mthumb -g -c util.s -o util.o
arm-linux-gnueabi-as -mcpu=cortex-m3 -mthumb -g -c cameraio.s -o cameraio.o
arm-linux-gnueabi-ld -T stm32f103-64k.ld -nostdlib -Map=test.map -o test.elf main.o util.o cameraio.o
arm-linux-gnueabi-objcopy test.elf -O ihex test.hex
arm-linux-gnueabi-objdump -d test.elf > test.lst
rm *.o
rm *.map
rm *.elf
