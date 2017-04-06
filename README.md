# stm32f030f4-camera

Attempt to use OV7670 camera with stm32f030f4

http://embeddedprogrammer.blogspot.ru/2012/07/hacking-ov7670-camera-module-sccb-cheat.html

Camera pins to MCU

- 3V3 - to VCC
- GND - to GND
- SIOC - to PA1 (7)
- SIOD - to PA2 (8) via Resistor 1k
- XCLK - to PA7 (13)
- HREF - to PA4 (10)
- VSYNC - to PA5 (11)
- PCLK - to PA6 (12)
- RESET - to VCC
- PWDN - to GND

- LED on PA0 (6)