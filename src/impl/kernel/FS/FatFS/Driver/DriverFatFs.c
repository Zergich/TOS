#include "System/FS/FatFS/ff.h"
#include <System/FS/FatFS/Driver/DriverFatFs.h>
#include <System/MemoryManager/kmalloc/kmalloc.h>
#include <System/VFS/Vfs.h>
#include <libs/string.h>
#include <types.h>

// Адаптер Чтения
int64_t FatFs_Read(VNode *node, u64 offset, void *buf, u64 count) {
  Fat32NodeContext *ctx = (Fat32NodeContext *)node->PrivateData;
  FIL *file = &ctx->FatFile;

  UINT bytes_read = 0;

  // Смещаемся на нужную позицию в файле
  if (f_lseek(file, (FSIZE_t)offset) != FR_OK)
    return -1;

  // Читаем данные через FatFs
  FRESULT res = f_read(file, buf, (UINT)count, &bytes_read);
  if (res != FR_OK)
    return -1;

  return (int64_t)bytes_read;
}

// Адаптер Записи
int64_t FatFs_Write(VNode *node, u64 offset, const void *buf, u64 count) {
  Fat32NodeContext *ctx = (Fat32NodeContext *)node->PrivateData;
  FIL *file = &ctx->FatFile;
  UINT bytes_written = 0;

  if (f_lseek(file, offset) != FR_OK)
    return -1;

  FRESULT res = f_write(file, buf, (UINT)count, &bytes_written);
  if (res != FR_OK)
    return -1;

  return (int64_t)bytes_written;
}

int FatFs_Lookup(VNode *parent, char *name, VNode **out_node);
int FatFsOpen(VNode *node, File *file);
int FatFsClose(VNode *node, File *file);
int FatfsDestroy(VNode *node);

VNodeOps FatFsOps = {.Read = FatFs_Read,
                     .Write = FatFs_Write,
                     .Lookup = FatFs_Lookup,
                     .Open = FatFsOpen,
                     .Close = FatFsClose,
                     .Destroy = FatfsDestroy};
// Поиск внутри директории (Lookup)
int FatFs_Lookup(VNode *parent, char *name, VNode **out_node) {
  FILINFO fno;
  int status = f_stat(name, &fno);
  if (status != 0)
    return -1; // чтоб не путаться между ошибками фс и обработчиками выше этой
               // функции

  VNode *node = kmalloc(sizeof(VNode));
  if (node == NULL) {
    return NULL_POINTER;
  }

  Fat32NodeContext *context = kmalloc(sizeof(Fat32NodeContext));
  if (context == NULL) {
    kfree(node);
    return NULL_POINTER;
  }

  string.Strcpy(
      context->Name,
      name); // могут быть ошибки так как буфер для имени всего 64 байта
  context->Name[sizeof(context->Name) - 1] = '\0';

  node->Type = (fno.fattrib & AM_DIR) ? VNODE_DIR : VNODE_FILE;

  node->Size = fno.fsize;
  node->RefCount = 1;
  node->Ops = &FatFsOps;
  node->PrivateData = context; // Прячем структуру FatFs внутрь VNode

  *out_node = node;
  return 0; // Успех
}
int FatFsOpen(VNode *node, File *file) {
  if (node == NULL || file == NULL || node->PrivateData == NULL)
    return -1;

  Fat32NodeContext *ctx = (Fat32NodeContext *)node->PrivateData;

  // Переводим флаги VFS в флаги FatFs
  BYTE mode = 0;
  if (file->Flags == 0 || (file->Flags & 1))
    mode |= FA_READ;
  if (file->Flags & 2)
    mode |= FA_WRITE;

  FRESULT res = f_open(&ctx->FatFile, ctx->Name, mode); // <-- ВОТ чего не было
  return (res == FR_OK) ? 0 : -1;
}

int FatFsClose(VNode *node, File *file) {
  if (node == NULL || node->PrivateData == NULL)
    return -1;
  Fat32NodeContext *ctx = (Fat32NodeContext *)node->PrivateData;

  if (node->Type == VNODE_FILE)
    f_close(&ctx->FatFile);
  return 0;
}

int FatfsDestroy(VNode *node) {
  if (node == NULL)
    return -1;
  // ВАЖНО: сам node освобождает VfsUnrefNode (он делает kfree(node) после
  // Destroy). Здесь чистим ТОЛЬКО PrivateData, иначе double free.
  if (node->PrivateData != NULL) {
    kfree(node->PrivateData);
    node->PrivateData = NULL;
  }
  return 0;
}
