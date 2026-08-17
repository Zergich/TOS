#pragma once

#include <System/VFS/Vfs.h>
#include <stddef.h>
#include <stdint.h>
#include <types.h>

// Создает новый VNode в куче ядра (через TLSF)
VNode *VfsAllocNode(VNodeType type);

// Увеличивает счетчик ссылок (защита от случайного удаления)
void VfsRefNode(VNode *node);

// Уменьшает счетчик ссылок. Если RefCount == 0 — освобождает память
void VfsUnrefNode(VNode *node);

int64_t VfsReadNode(struct VNode *node, u64 offset, void *buf, u64 count);
int64_t VfsWriteNode(struct VNode *node, u64 offset, const void *buf,
                     u64 count);
