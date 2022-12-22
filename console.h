#ifndef _console_h
#define _console_h

extern bool has_f18a();
extern void console_init();
extern void put_char(char c);
extern void put_string(const char* s);
extern void clear_screen();
extern char keyboard_get();

#endif
