#pragma once
#include <System/VFS/Vfs.h>
#include <types.h>

// Открывает файл по пути path относительно root_node. Возвращает fd (>= 0) или
// отрицательный код ошибки.
int sys_open(VNode *root_node, const char *path, int flags);

// Читает count байт из файла fd в буфер buf. Автоматически инкрементирует
// File->Offset.
int64_t sys_read(int fd, void *buf, uint64_t count);

// Записывает count байт из буфера buf в файл fd. Автоматически инкрементирует
// File->Offset.
int64_t sys_write(int fd, const void *buf, uint64_t count);

// Закрывает файловый дескриптор, уменьшает RefCount и высвобождает память при
// RefCount == 0.
int sys_close(int fd);
