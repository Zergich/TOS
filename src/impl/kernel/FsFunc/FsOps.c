#include <System/MemoryManager/kmalloc/kmalloc.h>
#include <types.h>

#include <System/VFS/Vfs.h>
#include <System/VFS/VfsFile.h>

#define MAX_GLOBAL_FILES 64

// Глобальная таблица дескрипторов
static File *GlobalFileTable[MAX_GLOBAL_FILES] = {NULL};

// Вспомогательная функция подсчета длины строки (на случай отсутствия
// <string.h>)
static int _kstrlen(const char *str) {
  int len = 0;
  while (str[len])
    len++;
  return len;
}

// ---------------------------------------------------------------------------
// sys_open
// ---------------------------------------------------------------------------
int sys_open(VNode *root_node, const char *path, int flags) {
  // Этап 1: Базовые проверки указателей
  if (!root_node)
    return -1; // BAD_POINTER_ROOT
  if (!path)
    return -2; // BAD_POINTER_PATH

  // Этап 2: Защита от Page Fault 14 (Вызов по нулевому адресу)
  // Проверяем, что драйвер ФС вообще реализовал структуру Ops и метод Lookup
  if (!root_node->Ops)
    return -3; // NO_OPS_STRUCTURE
  if (!root_node->Ops->Lookup)
    return -4; // NO_LOOKUP_METHOD

  // Этап 3: Защита от Page Fault 14 (Запись в Read-Only память)
  // Если драйвер внутри Lookup попытается изменить строку (например, разделить
  // путь по '/'), а мы передали "TEST.TXT" из секции .rodata, ядро упадет.
  // Решение: делаем безопасную копию строки в куче.
  int path_len = _kstrlen(path);
  char *mutable_path = (char *)kmalloc(path_len + 1);
  if (!mutable_path) {
    return -5; // NO_MEMORY_FOR_PATH
  }
  for (int i = 0; i <= path_len; i++) {
    mutable_path[i] = path[i];
  }

  // Этап 4: Поиск VNode через VFS
  VNode *target_vnode = NULL;
  int err = root_node->Ops->Lookup(root_node, mutable_path, &target_vnode);

  // Освобождаем временный буфер сразу после использования
  kfree(mutable_path);

  if (err != 0)
    return -6; // LOOKUP_ERROR (вернул сам драйвер)
  if (!target_vnode)
    return -7; // FILE_NOT_FOUND (не найден, но драйвер не вернул ошибку)

  // Этап 5: Поиск свободного слота в FD-таблице
  int fd = -1;
  for (int i = 0; i < MAX_GLOBAL_FILES; i++) {
    if (GlobalFileTable[i] == NULL) {
      fd = i;
      break;
    }
  }

  if (fd < 0) {
    return -8; // TOO_MANY_OPEN_FILES
  }

  // Этап 6: Выделение памяти под сессию файла
  File *file_obj = (File *)kmalloc(sizeof(File));
  if (!file_obj) {
    return -9; // NO_MEMORY_FOR_FILE
  }

  // Этап 7: Инициализация структуры
  file_obj->Node = target_vnode;
  file_obj->Offset = 0;
  file_obj->Flags = flags;
  file_obj->RefCount = 1;

  // Увеличиваем счетчик удержания VNode в памяти
  target_vnode->RefCount++;

  // Этап 8: Вызов функции открытия конкретной ФС (если она определена
  // драйвером)
  if (target_vnode->Ops && target_vnode->Ops->Open) {
    if (target_vnode->Ops->Open(target_vnode, file_obj) != 0) {
      // Откат изменений при ошибке открытия
      target_vnode->RefCount--;
      kfree(file_obj);
      return -10; // OPEN_FAILED (отказал драйвер ФС)
    }
  }

  // Этап 9: Регистрация дескриптора
  GlobalFileTable[fd] = file_obj;

  return fd;
}

// ---------------------------------------------------------------------------
// sys_read
// ---------------------------------------------------------------------------
int64_t sys_read(int fd, void *buf, uint64_t count) {
  // Расширенная диагностика ошибок
  if (fd < 0 || fd >= MAX_GLOBAL_FILES)
    return -11; // INVALID_FD_RANGE
  if (!buf)
    return -12; // BAD_BUFFER_POINTER

  File *file_obj = GlobalFileTable[fd];
  if (!file_obj)
    return -13; // FD_NOT_ALLOCATED
  if (!file_obj->Node)
    return -14; // BAD_VNODE_IN_FILE

  if (!file_obj->Node->Ops || !file_obj->Node->Ops->Read) {
    return -15; // READ_NOT_SUPPORTED
  }

  int64_t bytes_read =
      file_obj->Node->Ops->Read(file_obj->Node, file_obj->Offset, buf, count);

  if (bytes_read > 0) {
    file_obj->Offset += (u64)bytes_read;
  }

  return bytes_read;
}

// ---------------------------------------------------------------------------
// sys_write
// ---------------------------------------------------------------------------
int64_t sys_write(int fd, const void *buf, uint64_t count) {
  if (fd < 0 || fd >= MAX_GLOBAL_FILES)
    return -16;
  if (!buf)
    return -17;

  File *file_obj = GlobalFileTable[fd];
  if (!file_obj || !file_obj->Node)
    return -18;

  if (!file_obj->Node->Ops || !file_obj->Node->Ops->Write) {
    return -19;
  }

  int64_t bytes_written =
      file_obj->Node->Ops->Write(file_obj->Node, file_obj->Offset, buf, count);

  if (bytes_written > 0) {
    file_obj->Offset += (u64)bytes_written;
  }

  return bytes_written;
}

// ---------------------------------------------------------------------------
// sys_close
// ---------------------------------------------------------------------------
int sys_close(int fd) {
  if (fd < 0 || fd >= MAX_GLOBAL_FILES)
    return -20; // INVALID_FD

  File *file_obj = GlobalFileTable[fd];
  if (!file_obj)
    return -21; // ALREADY_CLOSED

  GlobalFileTable[fd] = NULL;
  file_obj->RefCount--;

  if (file_obj->RefCount == 0) {
    VNode *node = file_obj->Node;

    if (node) {
      if (node->Ops && node->Ops->Close) {
        node->Ops->Close(node, file_obj);
      }

      node->RefCount--;

      if (node->RefCount == 0) {
        if (node->Ops && node->Ops->Destroy) {
          node->Ops->Destroy(node);
        }
      }
    }

    kfree(file_obj);
  }

  return 0;
}
