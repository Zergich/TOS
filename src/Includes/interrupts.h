void set_idt_gate(int n, void (*handler)(void));
void divide_by_zero_handler(struct interrupt_frame *frame);
void idt_init();
void load_idt();
