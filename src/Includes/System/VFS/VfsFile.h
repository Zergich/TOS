#pragma once

#include <System/VFS/VfsNode.h>
#include <stddef.h>
#include <types.h>

typedef enum FileFlags {
  FILE_READ = (1 << 0),  // Файл открыт для чтения
  FILE_WRITE = (1 << 1), // Файл открыт для записи
  FILE_EXEC = (1 << 2)   // Файл открыт для исполнения
} FileFlags;

typedef struct File {
  VNode *Node;  // Указатель на связанный VNode
  u64 Offset;   // Текущая позиция чтения/записи (каретка)
  u32 Flags;    // Флаги доступа (FileFlags)
  u32 RefCount; // Счетчик ссылок на САМ объект File
} File;

File *VfsAllocFile(VNode *node, u32 flags);
void VfsRefFile(File *file);
void VfsUnrefFile(File *file);
