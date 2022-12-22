#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <z80.h>

#include "console.h"

#include "preamble.inc"

#define HCCA_REGISTER    0x80
#define CONTROL_REGISTER 0x00

#define PSG_DATA         0x40
#define PSG_ADDRESS      0x41

#define INT_MASK_HCCARINT 0x80
#define INT_MASK_HCCATINT 0x40
#define INT_MASK_KEYBOARD 0x20

#define CMD_CHAT         0xa0
#define LINE_BUFFER_SIZE 120

inline void
hcca_write(uint8_t byte)
{
  z80_outp(HCCA_REGISTER, byte);
}

inline uint8_t
hcca_read()
{
  return z80_inp(HCCA_REGISTER);
}

inline bool
has_interrupt(uint8_t mask)
{
  // Enable HCCA and keyboard interrupts
  z80_outp(PSG_ADDRESS, 0x0E);
  z80_outp(PSG_DATA, mask);
  z80_outp(PSG_ADDRESS, 0x0F);
  return z80_inp(PSG_DATA) & 1;
}

const char* divider = "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=";

void
main()
{
  static uint8_t server_col = 0;
  console_init();
  clear_screen();
  
  set_scroll_region(0, 21);
  set_cursor(22, 0);
  put_string(divider);
  if (has_f18a()) {
    put_string(divider);
  }
  set_cursor(23, 0);

  hcca_write(CMD_CHAT);
  while (true) {
    if (has_interrupt(INT_MASK_KEYBOARD)) {
      static void handle_keyboard_input();
      handle_keyboard_input();
    } else if (has_interrupt(INT_MASK_HCCARINT)) {
      uint8_t save_col = get_col();
      set_cursor(21, server_col);
      put_char(hcca_read());
      server_col = get_col();
      set_cursor(23, save_col);
    }
  }
}

static void
clear_input_line()
{
  uint8_t width = has_f18a() ? 80 : 40;
  cursor_off();
  set_cursor(23, 0);
  for (int i = 0; i < width; i++) {
    put_char(' ');
  }
  cursor_on();
}

char line_buffer[LINE_BUFFER_SIZE + 1];
char input_cursor = 0;

static void
handle_keyboard_input()
{
  uint8_t c = keyboard_get();
  switch (c) {
  case '\010':
  case '\177':
    if (input_cursor) {
      input_cursor--;
      put_string("\177");
    }
    break;
  case '\r':
  case '\n':
    if (input_cursor) {
      line_buffer[input_cursor++] = '\r';
      for (uint8_t i = 0; i < input_cursor; i++) {
        while (!has_interrupt(INT_MASK_HCCATINT))
          ;
        hcca_write(line_buffer[i]);
      }
      input_cursor = 0;
      clear_input_line();
    }
    break;
  default:
    if (c >= ' ' && c < '\177') {
      if (input_cursor < LINE_BUFFER_SIZE) {
        line_buffer[input_cursor++] = c;
        put_char(c);
      }
    }
  }
}
