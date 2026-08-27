#pragma ocne

#include <System/FS/FatFS/ff.h>
#include <System/VFS/Vfs.h>
#include <types.h>

extern VNodeOps FatFsOps;

typedef struct {
  char
      Name[64]; // пиздец на ножке     // Имя или относительный путь файла/папки
  FIL FatFile;  // Структура открытого файла FatFs (для VNODE_FILE)
  DIR FatDir;   // Структура открытой папки FatFs (для VNODE_DIR)
} Fat32NodeContext;

int64_t FatFs_Read(VNode *node, u64 offset, void *buf, u64 count);
int64_t FatFs_Write(VNode *node, u64 offset, const void *buf, u64 count);
int FatFs_Lookup(VNode *parent, char *name, VNode **out_node);
