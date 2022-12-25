#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <z80.h>

#include "nabu.h"
#include "console.h"

#include "preamble.inc"


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
  z80_outp(PSG_ADDRESS, 0x0E);
  z80_outp(PSG_DATA, mask);
  z80_outp(PSG_ADDRESS, 0x0F);
  return z80_inp(PSG_DATA) & 1;
}

const char* divider = "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=";
uint8_t screen_width;

static void
clear_input_line()
{
  cursor_off();
  set_cursor(23, 0);
  for (int i = 0; i < screen_width; i++) {
    put_char(' ');
  }
  cursor_on();
}

char line_buffer[LINE_BUFFER_SIZE + 1];
uint16_t input_cursor = 0;
uint16_t input_offset = 0;

static void
redisplay_input()
{
  set_cursor(23, 0);
  for (uint16_t i = 0; i < screen_width; i++) {
    if (i < input_cursor - input_offset) {
      put_char(line_buffer[input_offset + i]);
    } else {
      put_char(' ');
    }
  }
  set_cursor(23, input_cursor - input_offset);
}

static void
handle_keyboard_input()
{
  uint8_t c = keyboard_get();

  switch (c) {

  case '\010': // ^H BS  Backspace
  case '\177': //    DEL Delete
    if (input_cursor) {
      if (input_offset && ((input_cursor - input_offset) % (screen_width / 2) == 0)) {
        input_cursor--;
        input_offset -= screen_width / 2;
        redisplay_input();
      } else {
        input_cursor--;
        put_string("\177");
      }
    }
    break;

  case '\027': // ^W ETB delete word
    while (input_cursor && line_buffer[input_cursor-1] == ' ') {
      input_cursor--;
    }
    while (input_cursor) {
      if (line_buffer[input_cursor-1] == ' ') {
        break;
      }
      input_cursor--;
    }
    while (input_cursor < input_offset) {
      input_offset -= screen_width / 2;
    }
    redisplay_input();
    break;

  case '\025': // ^U NAK delete input
    input_cursor = 0;
    input_offset = 0;
    redisplay_input();
    break;

  case '\r': // ^M CR Carriage Return
  case '\n': // ^J LF Linefeed
    if (input_cursor) {
      line_buffer[input_cursor++] = '\r';
      for (uint8_t i = 0; i < input_cursor; i++) {
        while (!has_interrupt(INT_MASK_HCCATINT))
          ;
        hcca_write(line_buffer[i]);
      }
      input_cursor = 0;
      input_offset = 0;
      clear_input_line();
    }
    break;

  default:
    if (c >= ' ' && c < '\177') {
      if (input_cursor < LINE_BUFFER_SIZE) {
        line_buffer[input_cursor++] = c;
        if ((input_offset + input_cursor) % screen_width == 0) {
          input_offset += screen_width / 2;
          redisplay_input();
        } else {
          put_char(c);
        }
      } else {
        beep();
      }
    }
  }
}

void
main()
{
  static uint8_t server_col;
  console_init(true);
  clear_screen();

  screen_width = console_width();

  set_scroll_region(0, 21);
  set_cursor(22, 0);
  put_string(divider);
  if (has_f18a()) {
    put_string(divider);
  }
  set_cursor(23, 0);

  server_col = 0;
  hcca_write(CMD_CHAT);
  while (true) {
    if (has_interrupt(INT_MASK_KEYBOARD)) {
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
