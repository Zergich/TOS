#include <Drivers/ATA.h>
#include <arch/x86_64/io.h>
#include <stdint.h>

// Порты Primary ATA Bus
#define ATA_PRIMARY_DATA 0x1F0
#define ATA_PRIMARY_SECTOR_COUNT 0x1F2
#define ATA_PRIMARY_LBA_LOW 0x1F3
#define ATA_PRIMARY_LBA_MID 0x1F4
#define ATA_PRIMARY_LBA_HIGH 0x1F5
#define ATA_PRIMARY_DRIVE_SELECT 0x1F6
#define ATA_PRIMARY_COMMAND 0x1F7
#define ATA_PRIMARY_STATUS 0x1F7

// Битовые маски регистра статуса
#define ATA_STATUS_BSY 0x80 // Busy
#define ATA_STATUS_DRQ 0x08 // Data Request
#define ATA_STATUS_ERR 0x01 // Error

/**
 * Чтение ровно 1 сектора (512 байт) через LBA28 Polling
 */
int ata_read_sector(uint32_t lba, uint8_t *buf) {
  while ((inb(ATA_PRIMARY_STATUS) & ATA_STATUS_BSY) != 0)
    ;

  outb(ATA_PRIMARY_DRIVE_SELECT, 0xE0 | ((lba >> 24) & 0x0F));
  outb(ATA_PRIMARY_SECTOR_COUNT, 1);
  outb(ATA_PRIMARY_LBA_LOW, (uint8_t)(lba & 0xFF));
  outb(ATA_PRIMARY_LBA_MID, (uint8_t)((lba >> 8) & 0xFF));
  outb(ATA_PRIMARY_LBA_HIGH, (uint8_t)((lba >> 16) & 0xFF));
  outb(ATA_PRIMARY_COMMAND, 0x20);

  // Задержка
  for (int i = 0; i < 4; i++)
    inb(ATA_PRIMARY_STATUS);

  // Безопасное ожидание с проверкой флага ошибки
  while (1) {
    uint8_t status = inb(ATA_PRIMARY_STATUS);
    if ((status & ATA_STATUS_BSY) == 0) {
      if (status & ATA_STATUS_ERR) {
        return 1; // Устройство сообщило об ошибке!
      }
      if (status & ATA_STATUS_DRQ) {
        break; // Готово к передаче данных
      }
    }
  }

  uint16_t *ptr = (uint16_t *)buf;
  for (int i = 0; i < 256; i++) {
    ptr[i] = inw(ATA_PRIMARY_DATA);
  }
  return 0; // Успех
}
/**
 * Обертка для FatFs (помещается в файл diskio.c библиотеки FatFs)
 * FatFs тип DRESULT: 0 = RES_OK, 1 = RES_ERROR
 */
int ATA_disk_read(uint8_t pdrv, uint8_t *buff, uint32_t sector,
                  uint32_t count) {
  if (pdrv != 0 || buff == 0)
    return 1;

  for (uint32_t i = 0; i < count; i++) {
    // Если чтение сектора завершилось ошибкой, возвращаем RES_ERROR
    if (ata_read_sector(sector + i, buff + (i * 512)) != 0) {
      return 1;
    }
  }
  return 0;
}
