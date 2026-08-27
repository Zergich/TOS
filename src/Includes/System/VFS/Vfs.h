#pragma once
#include <stdint.h>
#include <types.h>

typedef enum { VNODE_FILE, VNODE_DIR, VNODE_CHARDEV, VNODE_BLKDEV } VNodeType;

struct VNode;
struct File;

// Таблица операций над узлом VFS (Полиморфизм)
typedef struct VNodeOps {
  int64_t (*Read)(struct VNode *node, u64 offset, void *buf, u64 count);
  int64_t (*Write)(struct VNode *node, u64 offset, const void *buf, u64 count);
  int (*Lookup)(struct VNode *parent, char *name, struct VNode **out_node);

  int (*Open)(struct VNode *node, struct File *file);
  int (*Close)(struct VNode *node, struct File *file);

  int (*Destroy)(struct VNode *node);
} VNodeOps;

// Физический узел VFS
typedef struct VNode {
  VNodeType Type;
  u64 Size;
  u32 RefCount;
  VNodeOps *Ops;     // Драйвер конкретной ФС
  void *PrivateData; // Указатель на структуру конкретной ФС (например,
                     // FAT32_Node*)
} VNode;

// Открытая сессия файла
typedef struct File {
  VNode *Node;
  u64 Offset;
  u32 Flags;
  u32 RefCount;
} File;

extern struct VNode *VfsRoot;

void VfsRootInit(void);
