#pragma once

#include <stddef.h>
#include <stdint.h>
#include <types.h>

// Тип узла: пока берем только 2 базовых типа для тестов
typedef enum VNodeType {
  VNodeRegular,  // Обычный файл
  VNodeDirectory // Директория
} VNodeType;

struct VNode;

typedef struct VNodeOps {
  // Вызывается, когда VNode окончательно уничтожается (RefCount == 0),
  // чтобы конкретная ФС могла очистить свое InternalData
  int64_t (*Read)(struct VNode *node, u64 offset, void *buf, u64 count);

  // Запись данных в узел
  int64_t (*Write)(struct VNode *node, u64 offset, const void *buf, u64 count);
  void (*Destroy)(struct VNode *node);
} VNodeOps;

typedef struct VNode {
  VNodeType Type; // Тип узла
  u64 Size;       // Размер в байтах (для файлов)
  u32 RefCount;   // Счетчик ссылок на объект

  VNodeOps *Ops;      // Указатель на методы (может быть NULL)
  void *InternalData; // Данные конкретной ФС (может быть NULL)
} VNode;

// Создает новый VNode в куче ядра (через TLSF)
VNode *VfsAllocNode(VNodeType type);

// Увеличивает счетчик ссылок (защита от случайного удаления)
void VfsRefNode(VNode *node);

// Уменьшает счетчик ссылок. Если RefCount == 0 — освобождает память
void VfsUnrefNode(VNode *node);
