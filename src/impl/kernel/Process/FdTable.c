#include <System/Process/FdTable.h>
#include <System/rsod.h>
#include <libs/MemoryUtils.h>
// fd - Файловый Дескриптор

void FdTableInit(FdTable *table) {
  if (table == NULL)
    Panic(U"Указатель на страницу файловых дескрипторов не действителен");
  memset(table->Entries, 0, sizeof(table->Entries));
}

int FdTableAllocFd(FdTable *table, File *file) {
  if (table == NULL || file == NULL)
    return -2; // условная егорка что херню передал в качестве параметров
  for (int i = 0; i < MAX_PROC_FILES; i++) {
    if (table->Entries[i] == NULL) {
      table->Entries[i] = file;
      return i;
    }
  }

  return -1; // значит нет мест
}

File *FdTableGet(FdTable *table, int fd) {
  if (table == NULL)
    return NULL;
  if (fd < 0 || fd >= MAX_PROC_FILES)
    return NULL;

  return table->Entries[fd];
}

int FdTableClose(FdTable *table, int fd) {
  File *file = FdTableGet(table, fd);
  if (file == NULL)
    return -1;
  table->Entries[fd] = NULL;

  VfsUnrefFile(file);

  return 0;
}

int FdTableDup(FdTable *table, int old_fd) {
  File *file = FdTableGet(table, old_fd);
  if (file == NULL)
    return -1;
  int new_fd = FdTableAllocFd(table, file);
  if (new_fd < 0)
    return -1;
  VfsRefFile(file);
  return new_fd;
}

void FdTableDestroy(FdTable *table) {
  if (table == NULL)
    Panic(U"Указатель на страницу файловых дескрипторов не действителен");
  for (int i = 0; i < MAX_PROC_FILES; i++) {
    if (table->Entries[i] != NULL) {
      FdTableClose(table, i);
    }
  }
}
