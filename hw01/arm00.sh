arm-none-eabi-gcc -mthumb -mcpu=cortex-m3 -nodefaultlibs -nostartfiles -Wl,-Map=hw01.map -o hw01 hw01.o gpio.o
