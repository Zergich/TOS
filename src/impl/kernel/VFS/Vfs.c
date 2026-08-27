#include "ConsoleIO/print.h"
#include "System/rsod.h"
#include <System/FS/FatFS/Driver/DriverFatFs.h>
#include <System/FS/FatFS/ff.h>
#include <System/MemoryManager/kmalloc/kmalloc.h>
#include <System/VFS/Vfs.h>
#include <System/VFS/VfsFile.h>
#include <System/VFS/VfsNode.h>
#include <stddef.h>
#include <types.h>

struct VNode *VfsRoot = NULL;
static FATFS FatFsObject; // Объект монтирования диска

void VfsRootInit(void) {
  // 1. Смонтировать диск с помощью FatFs
  FRESULT res = f_mount(&FatFsObject, "0:", 1);
  if (res != FR_OK) {
    printf("Ошибка монтирования: %u", res);
    Panic("");
    // Ошибка монтирования (например, нет диска или не FAT)
    return;
  }
  // 2. Создаем корневой VNode для нашего VFS
  VNode *root = kmalloc(sizeof(VNode));
  root->Type = VNODE_DIR;
  root->Size = 0;
  root->RefCount = 1;
  root->Ops = &FatFsOps;

  // В PrivateData корневого узла можно положить сам объект FATFS
  root->PrivateData = &FatFsObject;

  // 3. Присваиваем глобальному указателю
  VfsRoot = root;
}
int VfsOpen(struct VNode *cwd, char *path, u32 flags, struct File **out_file) {
  if (cwd == NULL || path == NULL)
    return VFS_NULL_POINTER;
  struct VNode *OutNode;
  int Status = LookUpPath(cwd, path, &OutNode);
  if (Status != VFS_OK)
    return Status;

  struct File *file = kmalloc(sizeof(File));
  if (file == NULL) {
    VfsUnrefNode(OutNode);
    return VFS_NULL_POINTER;
  }

  file->Flags = flags;
  file->Node = OutNode;
  file->Offset = 0;
  file->RefCount = 1;

  if (OutNode->Ops && OutNode->Ops->Open) {
    int Status = OutNode->Ops->Open(OutNode, file);
    if (Status != VFS_OK) {
      kfree(file);
      VfsUnrefNode(OutNode);
      return Status;
    }
  }

  *out_file = file;

  return VFS_OK;
}

int VfsClose(struct File *file) {
  if (file == NULL || file->Node == NULL)
    return VFS_NULL_POINTER;
  if (file->RefCount != 0) {
    file->RefCount--;
    if (file->RefCount > 0)
      return VFS_OK;
  }

  if (file->Node->Ops && file->Node->Ops->Close) {
    int Status = file->Node->Ops->Close(file->Node, file);
    if (Status != VFS_OK) {
      VfsUnrefNode(file->Node);
      kfree(file);

      return Status;
    }
  }
  VfsUnrefNode(file->Node);
  kfree(file);

  return VFS_OK;
}
