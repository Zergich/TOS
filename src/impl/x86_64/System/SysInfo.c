#include <ConsoleIO/print.h>
#include <System/sysinfo.h>
#include <limine.h>
#include <types.h>
#if LOADER == 0
struct multiboot_tag {
  u32 type;
  u32 size;
  u32 entry_size;
  u32 entry_version;
} __attribute__((packed));

struct MemoryInfo {
  u64 total_mb;
  u64 available_mb;
};
struct multiboot_mmap_entry {
  u64 addr;
  u64 len;
  u32 type; // 1 = ДоступнаяaRAM и 3х
  u32 reserved;
} __attribute__((packed));

u64 Mb2InfoAddr = 0;
u32 Mb2Magic = 0;

void GetPhysicsMemory(u64 mb2_magic, u64 mb2_info_addr,
                      struct MemoryInfo *mem) {
  mem->total_mb = 0;
  mem->available_mb = 0;

  if (mb2_magic != 0x36D76289)
    return;

  u64 total_bytes = 0;
  u64 available_bytes = 0;

  u8 *ptr = (u8 *)mb2_info_addr + 8;

  while (ptr < (u8 *)(mb2_info_addr + *(u32 *)mb2_info_addr)) {
    struct multiboot_tag *tag = (struct multiboot_tag *)ptr;

    if (tag->type == 6) {
      struct multiboot_mmap_entry *entry =
          (struct multiboot_mmap_entry *)(ptr + 16);
      u32 entry_count = (tag->size - 16) / 24;

      for (u32 i = 0; i < entry_count; i++) {

        // считать всю память
        if (entry->type != 0) {
          total_bytes += entry->len;
        }

        // Доступная память = Type 1 (Usable) + Type 3 (ACPI Reclaimable)
        // Type 3 можно использовать после чтения таблиц ACPI!
        if (entry->type == 1 || entry->type == 15) {
          available_bytes += entry->len;
        }

        entry = (struct multiboot_mmap_entry *)((u8 *)entry + 24);
      }
      break;
    }

    u32 tag_size = tag->size;
    if (tag_size % 8 != 0) {
      tag_size += 8 - (tag_size % 8);
    }
    ptr += tag_size;
  }

  mem->total_mb = total_bytes / (1024ULL * 1024ULL);
  mem->available_mb = available_bytes / (1024ULL * 1024ULL);
}

void PrintMem() {
  struct MemoryInfo mem;

  GetPhysicsMemory(Mb2Magic, Mb2InfoAddr, &mem);

  // Выводим (используем функцию print_int из предыдущего ответа)
  print("Total RAM:     ");
  printf("%i", mem.total_mb);
  print(" MB\n");

  print("Available RAM: ");
  printf("%i", mem.available_mb);
  print(" MB\n");

  // Если хочешь вывести сколько занято:
  u64 reserved = mem.total_mb - mem.available_mb;
  print("Reserved:      ");
  printf("%i", reserved);
  print(" MB\n");
}

// #elif LOADER == 1
//
//
#endif

void PrintMem() {}
