#ifndef _console_h
#define _console_h

extern bool has_f18a();
extern void console_init(bool text80);
extern uint8_t console_width();
extern void put_char(char c);
extern void put_string(const char* s);
extern void clear_screen();
extern char keyboard_get();
extern void set_cursor(uint8_t r, uint8_t c);
extern uint8_t get_row();
extern uint8_t get_col();
extern void set_scroll_region(uint8_t first_line, uint8_t last_line);
extern void cursor_on();
extern void cursor_off();
extern void beep();

#endif
