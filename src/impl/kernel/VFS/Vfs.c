#include <System/VFS/Vfs.h>
#include <stddef.h>
#include <types.h>

struct VNode *VfsRoot = NULL;
void VfsRootInit() {
  // тут должна вызываться функция самой файловой системы
  // VfsRoot = RamfsCreateRoot();
}
