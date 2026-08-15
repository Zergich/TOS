#include <System/Process/FdTable.h>
#include <libs/MemoryUtils.h> // Для memset или аналогов

// fd - Файловый Дескриптор

void FdTableInit(FdTable *table) {
  // TODO: Guard clause: проверка table на NULL
  // TODO: Очисти массив table->Entries с помощью memset
}

int FdTableAllocFd(FdTable *table, File *file) {
  // TODO: Guard clause: проверь table и file
  // TODO: Цикл от 0 до MAX_PROC_FILES - 1
  // TODO: Запиши file в первый свободный слот (NULL) и верни его индекс
  // TODO: Если слотов нет — верни -1
  return -1;
}

File *FdTableGet(FdTable *table, int fd) {
  if (table != NULL)
    return NULL;
  if (fd < 0 || fd >= MAX_PROC_FILES)
    return NULL;

  return table->Entries[fd];
}

int FdTableClose(FdTable *table, int fd) {
  // TODO: Получи file через FdTableGet(table, fd)
  // TODO: Если file == NULL — верни -1
  // TODO: Занули слот table->Entries[fd] = NULL
  // TODO: Уменьши RefCount файла через VfsUnrefFile(file)
  // TODO: Верни 0
  return -1;
}

int FdTableDup(FdTable *table, int old_fd) {
  // TODO: Получи file через FdTableGet(table, old_fd)
  // TODO: Если file == NULL — верни -1
  // TODO: Выдели новый слот: new_fd = FdTableAllocFd(table, file)
  // TODO: Если new_fd < 0 — верни -1
  // TODO: Инкрементируй RefCount файла через VfsRefFile(file)
  // TODO: Верни new_fd
  return -1;
}

void FdTableDestroy(FdTable *table) {
  // TODO: Guard clause: проверка table на NULL
  // TODO: Цикл от 0 до MAX_PROC_FILES - 1
  // TODO: Если слот не NULL — вызови FdTableClose(table, i)
}
