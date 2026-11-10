TARGET = mein_spiel
SRCS = main.c

include $(N64_INST)/include/n64.mk

all: $(TARGET).z64

$(TARGET).elf: $(SRCS:%.c=$(BUILD_DIR)/%.o)

clean:
	rm -rf $(BUILD_DIR) *.z64 *.elf

.PHONY: all clean
