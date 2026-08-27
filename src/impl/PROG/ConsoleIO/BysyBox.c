#include <ConsoleIO/print.h>
#include <System/FS/FatFS/ff.h>
#include <System/VFS/Vfs.h>
#include <types.h>

// void LS(const char *path) {
//   VNode *dir_node = VfsOpenPath(path);
//   if (!dir_node || dir_node->Type != VNODE_DIR)
//     return;
//
//   // Считываем список файлов в директории через FatFs f_readdir внутри
//   // абстракции
//   FatPrivate *priv = (FatPrivate *)dir_node->PrivateData;
//   DIR dir;
//   FILINFO fno;
//
//   if (f_opendir(&dir, priv->path) == FR_OK) {
//     while (f_readdir(&dir, &fno) == FR_OK && fno.fname[0] != 0) {
//       VNode *child = NULL;
//       // Ищем/получаем ноду каждого ребёнка через абстрактную таблицу
//       Operations if (dir_node->Ops->Lookup(dir_node, fno.fname, &child) == 0)
//       {
//         printf("%s %s (%lu bytes)\n",
//                (child->Type == VNODE_DIR) ? "[DIR]" : "[FILE]", fno.fname,
//                (unsigned long)child->Size);
//       }
//     }
//     f_closedir(&dir);
//   }
// }
