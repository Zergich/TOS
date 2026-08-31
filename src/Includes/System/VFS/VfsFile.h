#pragma once

#include <System/VFS/Vfs.h>
#include <stddef.h>
#include <types.h>

typedef enum FileFlags {
  FILE_READ = (1 << 0),  // Файл открыт для чтения
  FILE_WRITE = (1 << 1), // Файл открыт для записи
  FILE_EXEC = (1 << 2)   // Файл открыт для исполнения
} FileFlags;

File *VfsAllocFile(VNode *node, u32 flags);
void VfsRefFile(File *file);
void VfsUnrefFile(File *file);
int LookUpPath(struct VNode *parent, char *path, struct VNode **out_node);
