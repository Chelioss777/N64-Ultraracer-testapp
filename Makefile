TARGET = mein_spiel

# Nutzen des installierten Ubuntu Cloud-Compilers
CC = mips64-linux-gnu-gcc
OBJCOPY = mips64-linux-gnu-objcopy

# Hardware-Flags für den VR4300 Prozessor des N64
CFLAGS = -std=gnu99 -O2 -G0 -Wall -march=vr4300 -mtune=vr4300 -ffreestanding -nostdlib

all: $(TARGET).z64

$(TARGET).elf: main.c
	$(CC) $(CFLAGS) main.c -o $(TARGET).elf

$(TARGET).z64: $(TARGET).elf
	$(OBJCOPY) -O binary $(TARGET).elf $(TARGET).z64

clean:
	rm -f *.z64 *.elf *.o

.PHONY: all clean
