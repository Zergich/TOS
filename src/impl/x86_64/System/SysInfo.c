#include <ConsoleIO/print.h>
#include <System/sysinfo.h>
#include <limine.h>
#include <types.h>

struct MemoryType ReturnMemoryMapStruct = {.ReservedMemory = 0,
                                           .UsableMemory = 0};

struct MemoryType ReturnMemoryMap() {

  if (MemMapStructPtr->response == NULL) {
    return ReturnMemoryMapStruct;
  }

  uint64_t total_bytes = 0;

  uint64_t total_bytesRES = 0;
  u64 ConvertToMB = 1024 * 1024;

  struct limine_memmap_response *response = MemMapStructPtr->response;

  // Перебираем все записи в карте памяти
  for (uint64_t i = 0; i < response->entry_count; i++) {
    struct limine_memmap_entry *entry = response->entries[i];

    if (entry->type == LIMINE_MEMMAP_USABLE) {
      total_bytes += entry->length;
    }
    if (entry->type == LIMINE_MEMMAP_RESERVED) {
      total_bytesRES += entry->length;
    }
  }
  ReturnMemoryMapStruct.UsableMemory = total_bytes / ConvertToMB;
  ReturnMemoryMapStruct.ReservedMemory = total_bytesRES / ConvertToMB;
  return ReturnMemoryMapStruct;
}
