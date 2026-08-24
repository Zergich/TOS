#include <System/MemoryManager/kmalloc/kmalloc.h>
#include <System/VFS/Vfs.h>
#include <System/VFS/VfsFile.h>
#include <System/VFS/VfsNode.h>
#include <System/rsod.h>
#include <libs/MemoryUtils.h>
#include <libs/string.h>
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

int64_t VfsWriteNode(struct VNode *node, u64 offset, const void *buf,
                     u64 count) {
  if (node == NULL || node->Ops == NULL || node->Ops->Write == NULL ||
      buf == NULL)
    return NULL_POINTER;
  return node->Ops->Write(node, offset, buf, count);
}

int64_t VfsReadNode(struct VNode *node, u64 offset, void *buf, u64 count) {
  if (node == NULL || node->Ops == NULL || node->Ops->Read == NULL ||
      buf == NULL)
    return NULL_POINTER;
  return node->Ops->Read(node, offset, buf, count);
}

// да я знаю что путь должен быть константным но слит заточен под дрругое так
// что пока что анлак
int LookUpPath(struct VNode *parent, char *path, struct VNode **out_node) {
  if (parent == NULL || parent->Ops == NULL || parent->Ops->Lookup == NULL)
    return NULL_POINTER;

  // кароче залупа шерстит по папкам рекурсивно вниз в поисках файла на открытие
  // и так далее если начало строки пути из / значит корень и берем корневую
  // ноду если же нет берем ту которая была передана в качестве параметра parent
  int argc = 0;
  char **argv = string.SplitCh(path, '/', &argc);

  struct VNode *CurrentNode = VfsRoot;
  if (path[0] != '/')
    CurrentNode = parent;
  struct VNode *NextNode = NULL;

  VfsRefNode(CurrentNode);
  for (int i = 0; i < argc; i++) {
    // кароче все говорят что она не стригерритс на последний файл типа ранше
    // счетчик выйдет. че та жесткий затуп словил с этого
    if (CurrentNode->Type != VNODE_DIR) {
      VfsUnrefNode(CurrentNode);
      kfree(argv);
      return NOT_A_DIR;
    }

    int Status = CurrentNode->Ops->Lookup(CurrentNode, argv[i], &NextNode);
    if (Status != 0) {
      kfree(argv);
      VfsUnrefNode(CurrentNode);
      return Status;
    }
    VfsUnrefNode(CurrentNode);
    CurrentNode = NextNode;
  }

  kfree(argv);
  *out_node = CurrentNode;
  return OK;
}
