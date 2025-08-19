PROJECT=blink
TARGET=stm32f4
TARGETUP=STM32F4
CHIP=stm32f446re

CC=arm-none-eabi-gcc
LD=arm-none-eabi-ld
OBJCOPY=arm-none-eabi-objcopy
SIZE=arm-none-eabi-size
GDB=arm-none-eabi-gdb

LDSCRIPT=stm32f446re.ld
LIBOPENCM3=libopencm3
BUILD=build

CFLAGS=-Os -g -Wall -Wextra -std=c11 -ffreestanding --specs=nano.specs \
       -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 \
       -I$(LIBOPENCM3)/include \
       -I$(LIBOPENCM3)/lib \
       -D$(TARGETUP)

LDFLAGS=-T$(LDSCRIPT) -nostartfiles \
        -L$(LIBOPENCM3)/lib \
        -lopencm3_$(TARGET)

SRC=blink.c syscalls.c
OBJ=$(SRC:.c=.o)

all: $(PROJECT).bin

$(PROJECT).elf: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $@ $(LDFLAGS)

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@

flash: $(PROJECT).bin
	st-flash write $(PROJECT).bin 0x8000000

gdb:
	$(GDB) $(PROJECT).elf \
		-ex "target extended-remote localhost:3333" \
		-ex "monitor reset halt" \
		-ex "load"

clean:
	rm -f *.elf *.bin *.o

.PHONY: all clean flash

