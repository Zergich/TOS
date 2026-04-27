# --- Настройки компилятора ---
CC := x86_64-elf-gcc
LD := x86_64-elf-ld

# --- Флаги компиляции ---
# Общие флаги для всех файлов ядра
CFLAGS_COMMON := -I src/Includes -I src/Includes/System -I src/Includes/VGA  -I src/Includes/ConsoleIO \
                 -ffreestanding -mcmodel=kernel -mno-red-zone -m64

# Флаги для релизной сборки
GCC_RELEASE := $(CFLAGS_COMMON) -O2 -c

# Флаги для дебаг сборки
CFLAGS_DEBUG := $(CFLAGS_COMMON) -g -O0 -DDEBUG -c

# Флаги линковщика
LDFLAGS := -m elf_x86_64 -nostdlib -static -z max-page-size=0x1000

# --- Поиск исходников ---
# Находим все .c файлы в ядре и платформозависимой части
kernel_source_files := $(shell find src/impl/kernel -name "*.c")
x86_64_c_source_files := $(shell find src/impl/x86_64 -name "*.c")

# Генерируем пути для объектных файлов (release)
kernel_object_files := $(patsubst src/impl/kernel/%.c, build/kernel/%.o, $(kernel_source_files))
x86_64_object_files := $(patsubst src/impl/x86_64/%.c, build/x86_64/%.o, $(x86_64_c_source_files))

# Генерируем пути для объектных файлов (debug)
kernel_debug_object_files := $(patsubst src/impl/kernel/%.c, build-debug/kernel/%.o, $(kernel_source_files))
x86_64_debug_object_files := $(patsubst src/impl/x86_64/%.c, build-debug/x86_64/%.o, $(x86_64_c_source_files))

# --- Правила сборки ---

.PHONY: all clean build-x86_64

all: build-x86_64

# Универсальное правило для x86_64 (Release) с защитой от спецсимволов
build/x86_64/%.o: src/impl/x86_64/%.c
	@mkdir -p "$(dir $@)"
	$(CC) $(GCC_RELEASE) "$<" -o "$@"

# Универсальное правило для ядра (Release)
build/kernel/%.o: src/impl/kernel/%.c
	@mkdir -p "$(dir $@)"
	$(CC) $(GCC_RELEASE) "$<" -o "$@"

# Специфичные флаги для папки прерываний (Release)
build/x86_64/Interrupt/%.o: GCC_RELEASE += -mno-mmx -mno-sse -mno-80387

# Правило для ядра (Debug)
build-debug/kernel/%.o: src/impl/kernel/%.c
	@mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS_DEBUG) "$<" -o "$@"

# Правило для x86_64 (Debug)
build-debug/x86_64/%.o: src/impl/x86_64/%.c
	@mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS_DEBUG) "$<" -o "$@"


# Специфичные флаги для папки прерываний (Debug)
build-debug/x86_64/Interrupt/%.o: CFLAGS_DEBUG += -mno-mmx -mno-sse -mno-80387

# Основная цель сборки
build-x86_64: $(kernel_object_files) $(x86_64_object_files) $(kernel_debug_object_files) $(x86_64_debug_object_files)
	@mkdir -p dist/x86_64
	
	# Линковка релизного ядра
	$(LD) $(LDFLAGS) -o dist/x86_64/kernel.elf -T targets/x86_64/linker.ld $(kernel_object_files) $(x86_64_object_files)
	
	# Линковка дебаг ядра
	$(LD) $(LDFLAGS) -o dist/x86_64/kernelD.elf -T targets/x86_64/linker.ld $(kernel_debug_object_files) $(x86_64_debug_object_files)
	
	# Подготовка ISO
	@mkdir -p targets/x86_64/iso/boot
	cp dist/x86_64/kernel.elf targets/x86_64/iso/boot/kernel.elf
	
	# Создание ISO образа (используем xorriso как для Limine)
	xorriso -as mkisofs -b limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		targets/x86_64/iso -o dist/x86_64/kernel.iso
	

clean:
	rm -rf build build-debug dist
