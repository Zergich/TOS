# ==============================================================================
#                 ПОЛНОСТЬЮ ПЕРЕПИСАННЫЙ MAKEFILE ДЛЯ ЯДРА ОС
# ==============================================================================

# --- Настройки компилятора и инструментария ---
CC := x86_64-elf-gcc
LD := x86_64-elf-ld

# --- Флаги компиляции ---
# -I подключает папки заголовков, чтобы не писать длинные пути в #include
CFLAGS_COMMON := -I src/Includes \
                 -I src/Includes/System \
                 -I src/Includes/VGA \
                 -I src/Includes/ConsoleIO \
                 -I src/Includes/Drivers \
                 -I src/Includes/libs \
                 -I src/Includes/arch \
                 -ffreestanding -mcmodel=kernel -mno-red-zone -m64  -mcmodel=kernel -mno-sse -mno-sse2 -march=x86-64
# Флаги для релизной сборки (с оптимизацией)
GCC_RELEASE := $(CFLAGS_COMMON) -O2 -c

# Флаги для дебаг сборки (с отладочной информацией)
CFLAGS_DEBUG := $(CFLAGS_COMMON) -g -O0 -DDEBUG -c

# Флаги линковщика
LDFLAGS := -m elf_x86_64 -nostdlib -static -z max-page-size=0x1000

# --- Автоматический поиск исходников ---
# Находим все .c файлы в корне ядра и подпапках, ИСКЛЮЧАЯ платформозависимую arch
kernel_source_files := $(shell find src/impl -type f -name "*.c" ! -path "src/impl/arch/*")

# Находим все .c файлы СТРОГО внутри платформозависимой папки arch
arch_source_files := $(shell find src/impl/arch -type f -name "*.c")

# Находим все ассемблерные .s файлы (если они есть в проекте)
asm_source_files := $(shell find src/impl -type f -name "*.s")

# --- Генерация путей для объектных файлов (.o) ---
# Релизные объекты
kernel_object_files := $(patsubst src/impl/%.c, build/%.o, $(kernel_source_files))
arch_object_files   := $(patsubst src/impl/arch/%.c, build/arch/%.o, $(arch_source_files))
asm_object_files    := $(patsubst src/impl/%.s, build/%.o, $(asm_source_files))
ALL_RELEASE_OBJECTS := $(kernel_object_files) $(arch_object_files) $(asm_object_files)

# Дебаг объекты
kernel_debug_object_files := $(patsubst src/impl/%.c, build-debug/%.o, $(kernel_source_files))
arch_debug_object_files   := $(patsubst src/impl/arch/%.c, build-debug/arch/%.o, $(arch_source_files))
asm_debug_object_files    := $(patsubst src/impl/%.s, build-debug/%.o, $(asm_source_files))
ALL_DEBUG_OBJECTS   := $(kernel_debug_object_files) $(arch_debug_object_files) $(asm_debug_object_files)

# --- Главные цели (Rules) ---

.PHONY: all clean build-x86_64

all: build-x86_64

# --- Специфичные флаги для прерываний ---
# Отключаем MMX/SSE инструкции для обработчиков прерываний, чтобы не разрушать FPU-стек процессора
build/arch/x86_64/Interrupt/%.o: GCC_RELEASE += -mno-mmx -mno-sse -mno-80387
build-debug/arch/x86_64/Interrupt/%.o: CFLAGS_DEBUG += -mno-mmx -mno-sse -mno-80387

# --- Правила компиляции для RELEASE ---

# Компиляция общего кода ядра (kernel, libs, Drivers, VGA, ConsoleIO)
build/%.o: src/impl/%.c
	@mkdir -p "$(dir $@)"
	$(CC) $(GCC_RELEASE) "$<" -o "$@"

# Компиляция платформозависимого кода (arch)
build/arch/%.o: src/impl/arch/%.c
	@mkdir -p "$(dir $@)"
	$(CC) $(GCC_RELEASE) "$<" -o "$@"

# Компиляция ассемблерных файлов (.s)
build/%.o: src/impl/%.s
	@mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS_COMMON) -c "$<" -o "$@"

# --- Правила компиляции для DEBUG ---

build-debug/%.o: src/impl/%.c
	@mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS_DEBUG) "$<" -o "$@"

build-debug/arch/%.o: src/impl/arch/%.c
	@mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS_DEBUG) "$<" -o "$@"

build-debug/%.o: src/impl/%.s
	@mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS_COMMON) -c "$<" -o "$@"

# --- Основная цель: Линковка и сборка образов ---

build-x86_64: $(ALL_RELEASE_OBJECTS) $(ALL_DEBUG_OBJECTS)
	@mkdir -p dist/x86_64
	@echo "🔗 Линковка релизного ядра..."
	$(LD) $(LDFLAGS) -o dist/x86_64/kernel.elf -T targets/x86_64/linker.ld $(ALL_RELEASE_OBJECTS)
	
	@echo "🔗 Линковка дебаг ядра..."
	$(LD) $(LDFLAGS) -o dist/x86_64/kernelD.elf -T targets/x86_64/linker.ld $(ALL_DEBUG_OBJECTS)
	
	@echo "📁 Подготовка папки ISO..."
	@mkdir -p targets/x86_64/iso/boot
	cp dist/x86_64/kernel.elf targets/x86_64/iso/boot/kernel.elf
	
	@echo "💿 Создание загрузочного ISO-образа через xorriso..."
	# Создание ISO образа (Облегченный и чистый вариант без ломающих MBR-флагов)
	xorriso -as mkisofs -b limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot limine-uefi-cd.bin \
		targets/x86_64/iso -o dist/x86_64/kernel.iso

	@echo "✨ Сборка успешно завершена! Образ: dist/x86_64/kernel.iso"

# --- Очистка проекта ---
clean:
	rm -rf build build-debug dist
	@echo "🧹 Проект полностью очищен от объектных файлов и образов."

