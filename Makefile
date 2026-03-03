CC := x86_64-elf-gcc 
LD := x86_64-elf-ld

kernel_source_files := $(shell find src/impl/kernel -name *.c)
kernel_object_files := $(patsubst src/impl/kernel/%.c, build/kernel/%.o, $(kernel_source_files))

x86_64_c_source_files := $(shell find src/impl/x86_64 -name *.c)
x86_64_c_object_files := $(patsubst src/impl/x86_64/%.c, build/x86_64/%.o, $(x86_64_c_source_files))

x86_64_asm_source_files := $(shell find src/impl/x86_64 -name *.asm)
x86_64_asm_object_files := $(patsubst src/impl/x86_64/%.asm, build/x86_64/%.o, $(x86_64_asm_source_files))

x86_64_object_files := $(x86_64_c_object_files) $(x86_64_asm_object_files)



# --- Переменные для Debug (отдельная папка build-debug/) ---
kernel_debug_object_files := $(patsubst src/impl/kernel/%.c, build-debug/kernel/%.o, $(kernel_source_files))
x86_64_c_debug_object_files := $(patsubst src/impl/x86_64/%.c, build-debug/x86_64/%.o, $(x86_64_c_source_files))
x86_64_asm_debug_object_files := $(patsubst src/impl/x86_64/%.asm, build-debug/x86_64/%.o, $(x86_64_asm_source_files))
x86_64_debug_object_files := $(x86_64_c_debug_object_files) $(x86_64_asm_debug_object_files)


CFLAGS_DEBUG   := -I src/Includes -ffreestanding -g -O0 -DDEBUG
ASMFLAGS      := -f elf64

build/kernel/%.o: src/impl/kernel/%.c
	mkdir -p $(dir $@)
	$(CC) -c -I src/Includes -ffreestanding $(patsubst build/kernel/%.o, src/impl/kernel/%.c, $@) -o $@

build/x86_64/OSFunc/%.o: src/impl/x86_64/OSFunc/%.c
	mkdir -p $(dir $@)
	$(CC) -c -I src/Includes -ffreestanding $(patsubst build/x86_64/%.o, src/impl/x86_64/%.c, $@) -o $@

build/x86_64/Interrupt/%.o: src/impl/x86_64/Interrupt/%.c
	mkdir -p $(dir $@)
	$(CC) -c -I src/Includes -ffreestanding -mno-mmx -mno-sse -mno-80387 $(patsubst build/x86_64/%.o, src/impl/x86_64/%.c, $@) -o $@

build/x86_64/%.o: src/impl/x86_64/%.asm
	mkdir -p $(dir $@)
	nasm -f elf64 $(patsubst build/x86_64/%.o, src/impl/x86_64/%.asm, $@) -o $@


#debug

build-debug/kernel/%.o: src/impl/kernel/%.c
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS_DEBUG) $(patsubst build-debug/kernel/%.o, src/impl/kernel/%.c, $@) -o $@

build-debug/x86_64/OSFunc/%.o: src/impl/x86_64/OSFunc/%.c
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS_DEBUG) $(patsubst build-debug/x86_64/%.o, src/impl/x86_64/%.c, $@) -o $@

build-debug/x86_64/Interrupt/%.o: src/impl/x86_64/Interrupt/%.c
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS_DEBUG) -mno-mmx -mno-sse -mno-80387 $(patsubst build-debug/x86_64/%.o, src/impl/x86_64/%.c, $@) -o $@

build-debug/x86_64/%.o: src/impl/x86_64/%.asm
	mkdir -p $(dir $@)
	nasm $(ASMFLAGS) -g $(patsubst build-debug/x86_64/%.o, src/impl/x86_64/%.asm, $@) -o $@

.PHONY: build-x86_64
build-x86_64: $(kernel_object_files) $(x86_64_object_files) $(kernel_debug_object_files) $(x86_64_debug_object_files)
	mkdir -p dist/x86_64
	$(LD) -n -o dist/x86_64/kernel.bin -T targets/x86_64/linker.ld $(kernel_object_files) $(x86_64_object_files)
	cp dist/x86_64/kernel.bin targets/x86_64/iso/boot/kernel.bin
	grub-mkrescue /usr/lib/grub/i386-pc -o dist/x86_64/kernel.iso targets/x86_64/iso
	
	#debug file 
	$(LD) -o dist/x86_64/kernel.elf -T targets/x86_64/linker.ld $(kernel_debug_object_files) $(x86_64_debug_object_files)

.PHONY: clean
clean:
	rm -rf build dist

# -mno-mmx -mno-sse -mno-80387 флаги для компиляции с прерываниями 
