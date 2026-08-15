#pragma once
#include <System/VFS/VfsFile.h>
#include <types.h>

#define MAX_PROC_FILES 32

typedef struct FdTable {
  File *Entries[MAX_PROC_FILES];
} FdTable;

void FdTableInit(FdTable *table);
int FdTableAllocFd(FdTable *table, File *file);
File *FdTableGet(FdTable *table, int fd);
int FdTableClose(FdTable *table, int fd);
int FdTableDup(FdTable *table, int old_fd);
void FdTableDestroy(FdTable *table);
