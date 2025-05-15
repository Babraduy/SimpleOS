NASM = nasm
GCC = i686-linux-gnu-gcc
LD = i686-linux-gnu-ld
QEMU = qemu-system-i386

SRC_DIR = src
OBJ_DIR = obj
INCLUDE_DIR = include
ASM_DIR = asm
BIN_DIR = bin

SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

ASM = $(wildcard $(ASM_DIR)/*.asm)
ASM_OBJ = $(filter-out $(OBJ_DIR)/boot.o, $(ASM:$(ASM_DIR)/%.asm=$(OBJ_DIR)/%.o))

KERNEL_ENTRY_OBJ = obj/kernel_entry.o

KERNEL_OUTPUT = kernel.bin
FINAL_OUTPUT = SimpleOS.bin
ISO_NAME = myos.iso
DISK_NAME = fat.img

GCC_FLAGS = -ffreestanding -I include -m32 -nostdlib -fno-pie -fno-pic -c -Wall -Wextra -fno-omit-frame-pointer -mno-red-zone -O0 -ffunction-sections
NASM_FLAGS = -f elf
LD_FLAGS = -T ld/linker.ld --oformat binary --gc-sections
QEMU_FLAGS = -drive format=raw,file="$(BIN_DIR)/$(FINAL_OUTPUT)",index=0,if=floppy -m 128M -boot order=a -drive format=raw,file="$(DISK_NAME)",index=0,if=ide 
QEMU_ISO_FLAGS = -cdrom $(ISO_NAME) -boot d -m 128M -hda $(DISK_NAME)

all: $(OBJ_DIR) $(BIN_DIR) $(KERNEL_OUTPUT)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BIN_DIR)/boot.bin: $(ASM_DIR)/boot.asm
	$(NASM) -f bin $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(GCC) $(GCC_FLAGS) $< -o $@

$(OBJ_DIR)/%.o: $(ASM_DIR)/%.asm | $(OBJ_DIR)
	$(if $(filter-out $(ASM_DIR)/boot.asm, $<), $(NASM) $(NASM_FLAGS) $< -o $@)

$(KERNEL_OUTPUT): $(BIN_DIR)/boot.bin $(OBJ) $(ASM_OBJ) $(KERNEL_ENTRY_OBJ)
	$(LD) $(LD_FLAGS) -o $(BIN_DIR)/$(KERNEL_OUTPUT) $(KERNEL_ENTRY_OBJ) $(filter-out $(KERNEL_ENTRY_OBJ), $(ASM_OBJ)) $(OBJ)

	cat $(BIN_DIR)/boot.bin $(BIN_DIR)/$(KERNEL_OUTPUT) > $(BIN_DIR)/$(FINAL_OUTPUT)

$(DISK_NAME):
	fallocate -l 10M $(DISK_NAME)
	mkfs.msdos -F 12 $(DISK_NAME)

iso: all $(DISK_NAME)
	mkdir -p iso/boot
	cp $(BIN_DIR)/SimpleOS.bin iso/boot/
	mkisofs -R -o $(ISO_NAME) -b boot/SimpleOS.bin -no-emul-boot -boot-load-size 4 -boot-info-table iso/

run: $(BIN_DIR)/$(FINAL_OUTPUT) $(DISK_NAME)
	$(QEMU) $(QEMU_FLAGS)

run-iso: iso $(DISK_NAME)
	$(QEMU) $(QEMU_ISO_FLAGS)

clean:
	rm -rf $(OBJ_DIR) $(KERNEL_OUTPUT) $(FINAL_OUTPUT) $(ISO_NAME) $(BIN_DIR) iso
