NASM = nasm
GCC = i686-linux-gnu-gcc
LD = i686-linux-gnu-ld
QEMU = qemu-system-i386

SRC_DIR = src
OBJ_DIR = obj
INCLUDE_DIR = include
ASM_DIR = asm

SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

ASM = $(wildcard $(ASM_DIR)/*.asm)
ASM_OBJ = $(filter-out $(OBJ_DIR)/boot.o, $(ASM:$(ASM_DIR)/%.asm=$(OBJ_DIR)/%.o))

KERNEL_ENTRY_OBJ = obj/kernel_entry.o

OUTPUT = everything.bin
FINAL_OUTPUT = SkibidiOS.bin

GCC_FLAGS = -ffreestanding -I include -m32 -nostdlib -fno-pie -fno-pic -c -Wall -Wextra -fno-omit-frame-pointer -mno-red-zone -O0
NASM_FLAGS = -f elf
LD_FLAGS = -T ld/linker.ld --oformat binary
QEMU_FLAGS = -drive format=raw,file="$(FINAL_OUTPUT)",index=0,if=floppy -m 128M

all: $(OBJ_DIR) $(OUTPUT)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

boot.bin: $(ASM_DIR)/boot.asm
	$(NASM) -f bin $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(GCC) $(GCC_FLAGS) $< -o $@

$(OBJ_DIR)/%.o: $(ASM_DIR)/%.asm | $(OBJ_DIR)
	$(if $(filter-out $(ASM_DIR)/boot.asm, $<), $(NASM) $(NASM_FLAGS) $< -o $@)

$(OUTPUT): boot.bin $(OBJ) $(ASM_OBJ) $(KERNEL_ENTRY_OBJ)
	$(LD) $(LD_FLAGS) -o $(OUTPUT) $(KERNEL_ENTRY_OBJ) $(filter-out $(KERNEL_ENTRY_OBJ), $(ASM_OBJ)) $(OBJ)

	cat boot.bin $(OUTPUT) > $(FINAL_OUTPUT)

run: $(FINAL_OUTPUT)
	$(QEMU) $(QEMU_FLAGS)

clean:
	rm -rf $(OBJ_DIR) $(OUTPUT) $(FINAL_OUTPUT)
