#ifndef PIC_H
#define PIC_H

// Интерфейс программиремого контролера прерываний

/* порты для мастера и раба */
#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1

void pic_init(void);
void pic_ack(unsigned int irq);

#endif
