#include <System/MemoryManager/kmalloc/kmalloc.h>
#include <System/VFS/Vfs.h>
#include <System/VFS/VfsFile.h>
#include <System/VFS/VfsNode.h>
#include <stddef.h>
#include <types.h>

struct VNode *VfsRoot = NULL;
void VfsRootInit() {
  // тут должна вызываться функция самой файловой системы
  // VfsRoot = RamfsCreateRoot();
}

int VfsOpen(struct VNode *cwd, char *path, u32 flags, struct File **out_file) {
  if (cwd == NULL || path == NULL)
    return NULL_POINTER;
  struct VNode *OutNode;
  int Status = LookUpPath(cwd, path, &OutNode);
  if (Status != OK)
    return Status;

  struct File *file = kmalloc(sizeof(File));
  if (file == NULL) {
    VfsUnrefNode(OutNode);
    return NULL_POINTER;
  }

  file->Flags = flags;
  file->Node = OutNode;
  file->Offset = 0;
  file->RefCount = 1;

  if (OutNode->Ops && OutNode->Ops->Open) {
    int Status = OutNode->Ops->Open(OutNode, file);
    if (Status != OK) {
      kfree(file);
      VfsUnrefNode(OutNode);
      return Status;
    }
  }

  *out_file = file;

  return OK;
}
