V=1
SOURCE_DIR=src
BUILD_DIR=build
include $(N64_INST)/include/n64.mk

all: ultraracer_test.z64

ultraracer_test.z64: $(BUILD_DIR)/main.o
	$(N64_BINDIR)/mips64-elf-ld -o $(BUILD_DIR)/ultraracer_test.elf $(BUILD_DIR)/main.o $(LDFLAGS)
	$(N64_BINDIR)/mips64-elf-objcopy -O binary $(BUILD_DIR)/ultraracer_test.elf $(BUILD_DIR)/ultraracer_test.bin
	$(N64_BINDIR)/chksum64 $(BUILD_DIR)/ultraracer_test.bin
	mv $(BUILD_DIR)/ultraracer_test.bin ultraracer_test.z64

clean:
	rm -rf $(BUILD_DIR) *.z64
