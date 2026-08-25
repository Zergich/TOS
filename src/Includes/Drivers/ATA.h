#pragma once
#include <types.h>

#define ATA_STATUS_BSY 0x80
#define ATA_STATUS_DRQ 0x08
int disk_read(uint8_t pdrv, uint8_t *buff, uint32_t sector, uint32_t count);
void ata_read_sector(uint32_t lba, uint8_t *buf);
