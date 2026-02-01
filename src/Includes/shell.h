#ifndef SHELL_H
#define SHELL_H

void print_string(const char *str, int x, int y, unsigned char color);
void init_shell(void);
void shell_handle_input(char c);
void toggle_cursor(void);

#endif
