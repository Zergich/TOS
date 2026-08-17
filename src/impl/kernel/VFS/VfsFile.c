#include <System/MemoryManager/kmalloc/kmalloc.h>
#include <System/VFS/VfsFile.h>
#include <System/VFS/VfsNode.h>
#include <System/rsod.h>
#include <libs/MemoryUtils.h>

File *VfsAllocFile(VNode *node, u32 flags) {

  if (node == NULL)
    return NULL;
  File *file = kmalloc(sizeof(File));
  if (file == NULL)
    return NULL;
  memset(file, 0, sizeof(File));

  file->Node = node;
  file->Flags = flags;
  file->Offset = 0;
  file->RefCount = 1;

  VfsRefNode(node);
  return file;
}

void VfsRefFile(File *file) {
  if (file == NULL)
    Panic(U"Пустой указатель на File");
  file->RefCount++;
}

void VfsUnrefFile(File *file) {

  if (file == NULL)
    return;

  if (file->RefCount == 0)
    Panic(U"Повторное освобождение файла");
  file->RefCount--;

  if (file->RefCount == 0) {
    VfsUnrefNode(file->Node);
    kfree(file);
  }
}

int64_t VfsWriteFile(File *file, const void *buf, u64 count) {
  if (file == NULL)
    return NULL_POINTER;
  if ((file->Flags & FILE_WRITE) == 0)
    return ACESS_DENIED;

  int64_t bytes = VfsWriteNode(file->Node, file->Offset, buf, count);

  if (bytes > 0)
    file->Offset += bytes;
  return bytes;
}

int64_t VfsReadFile(File *file, void *buf, u64 count) {
  if (file == NULL)
    return NULL_POINTER;
  if ((file->Flags & FILE_READ) == 0)
    return ACESS_DENIED;

  int64_t bytes = VfsReadNode(file->Node, file->Offset, buf, count);
  if (bytes > 0)
    file->Offset += bytes;
  return bytes;
}
