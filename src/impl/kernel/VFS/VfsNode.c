#include <System/MemoryManager/kmalloc/kmalloc.h>
#include <System/VFS/VfsNode.h>
#include <System/rsod.h>
#include <libs/MemoryUtils.h>
#include <stddef.h>
#include <stdint.h>
#include <types.h>

VNode *VfsAllocNode(VNodeType type) {
  VNode *node = kmalloc(sizeof(VNode));

  if (node == NULL)
    return NULL;
  memset(node, 0, sizeof(VNode)); // чтоб не читал битые данные зануляем
  node->Type = type;
  node->RefCount = 1; // раз файл был создан то что то его использует

  return node;
}

void VfsRefNode(VNode *node) {
  // когда что то сохраняет у себя указатель на ноду
  if (node != NULL) {
    node->RefCount++;
  }
}

void VfsUnrefNode(VNode *node) {
  // для закрытия файла (освобождение ресурсов)
  if (node != NULL) {
    // если функция была случайно взвана когда кол во ссыллок уже блыло 0

    if (node->RefCount == 0)
      Panic(U"Функция освобожния была вызвана случайно или же Use-After-Free!");

    node->RefCount--;
    if (node->RefCount == 0) {
      // проверка встали ли дрова файловой системы в
      // интерфейс виртуальной (фабрики)
      if (node->Ops != NULL && node->Ops->Destroy != NULL) {
        node->Ops->Destroy(node); // Даем конкретной ФС очистить ее InternalData
      }
      kfree(node);
    }
  }
}

int64_t VfsWrite(struct VNode *node, u64 offset, const void *buf, u64 count) {
  return -1;
}

int64_t VfsRead(struct VNode *node, u64 offset, void *buf, u64 count) {
  return -1;
}
