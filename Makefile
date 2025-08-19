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
       -mcpu=cortex-m4 -mthumb -mfloat-abi=softfp -mfpu=fpv4-sp-d16 \
       -I$(LIBOPENCM3)/include \
       -I$(LIBOPENCM3)/lib \
       -D$(TARGETUP)
#-mfloat-abi=hard -mfpu=fpv4-sp-d16

LDFLAGS=-T$(LDSCRIPT) -nostartfiles \
        -L$(LIBOPENCM3)/lib \
        -lopencm3_$(TARGET) #\
		#//-u _printf_float

SRC=blink.c syscalls.c
OBJ=$(SRC:.c=.o)

all: $(PROJECT).bin

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(PROJECT).elf: $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@

flash: $(PROJECT).bin
	st-flash write $(PROJECT).bin 0x8000000

gdb: $(PROJECT).elf
	$(GDB) $(PROJECT).elf \
		-ex "target extended-remote localhost:3333" \
		-ex "monitor reset halt" \
		-ex "load"

libs:
	make -C $(LIBOPENCM3) TARGETS="stm32/f4" FP_FLAGS="-mfloat-abi=softfp -mfpu=fpv4-sp-d16"

clean:
	rm -f *.elf *.bin *.o

.PHONY: all clean flash

