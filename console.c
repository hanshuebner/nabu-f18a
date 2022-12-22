#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <z80.h>

#include "console.h"

#include "fonts.inc"

#define R0_EXTVID 0x01
#define R0_M3     0x02
#define R0_M4     0x04
#define R0_IE1    0x10

#define R1_MAG    0x01
#define R1_SIZE   0x02
#define R1_M2     0x08
#define R1_M1     0x10
#define R1_IE0    0x20
#define R1_BLANK  0x40
#define R1_16K    0x80

#define FONT_BASE 0x0100
#define PAGE_BASE 0x1000

#define VDP_RAM_SIZE 0x4000

#define ROWS    24
#define MAXCOLS 80

#define VDP_DATA         0xa0
#define VDP_STATUS       0xa1

#define KEYBOARD_DATA    0x90
#define KEYBOARD_STATUS  0x91

#define KEYBOARD_RXRDY   0x02

// Writes a byte to databus for register access
inline void
write_status_reg(uint8_t value)
{
  z80_outp(VDP_STATUS, value);
}

// Reads a byte from databus for register access
inline uint8_t
read_status_reg()
{
  return z80_inp(VDP_STATUS);
}

inline void
register_set(uint8_t reg, uint8_t value)
{
  write_status_reg(value);
  write_status_reg(0x80 | reg);
}

inline void
vram_set_write_address(uint16_t address)
{
  z80_outp(VDP_STATUS, address & 0xff);
  z80_outp(VDP_STATUS, 0x40 | (address >> 8) & 0x3f);
}

// Writes a byte to databus for vram access
inline void
vram_write(uint8_t value)
{
  z80_outp(VDP_DATA, value);
}

inline void
vram_set_read_address(uint16_t address)
{
  z80_outp(VDP_STATUS, address & 0xff);
  z80_outp(VDP_STATUS, (address >> 8) & 0x3f);
}

// Reads a byte from databus for vram access
inline uint8_t
vram_read()
{
  return z80_inp(VDP_DATA);
}

static void
load_font(uint8_t* font)
{
  vram_set_write_address(FONT_BASE);
  for (int i = 0; i < FONT_SIZE; i++) {
    vram_write(font[i]);
  }
}

static void
clear_vram()
{
  vram_set_write_address(0);
  for (int i = 0; i < VDP_RAM_SIZE; i++) {
    vram_write(0);
  }
}

bool
has_f18a()
{
  write_status_reg(0x01);
  return (read_status_reg() & 0xe0) == 0xe0;
}

static uint8_t cols = 40;

static void
vdp_init()
{
  clear_vram();
  // 80 chars per line
  register_set(0, R0_M4);
  register_set(1, R1_16K | R1_BLANK | R1_M1 | R1_SIZE);
  register_set(2, 0x04); // name table at 0x1000
  register_set(4, 0x00); // pattern table at 0x0000
  load_font(font_1);

  if (has_f18a()) {
    cols = 80;
  } else {
    cols = 40;
  }
}

static void
keyboard_init()
{
  // ROM has initialized keyboard UART, nothing to be done here
}

void
console_init()
{
  vdp_init();
  keyboard_init();
}

static uint8_t row = 0;
static uint8_t col = 0;

static void
set_vram_write_address_to_cursor()
{
  vram_set_write_address(PAGE_BASE + row * cols + col);
}

static void
set_vram_read_address_to_cursor()
{
  vram_set_read_address(PAGE_BASE + row * cols + col);
}

void
clear_screen()
{
  vram_set_write_address(PAGE_BASE);
  for (uint16_t i = 0; i < ROWS * cols; i++) {
    vram_write(' ');
  }
  row = col = 0;
  set_vram_write_address_to_cursor();
}

void
scroll_up()
{
  static uint8_t line[MAXCOLS];
  for (uint8_t r = 1; r < ROWS; r++) {
    vram_set_read_address(PAGE_BASE + r * cols);
    for (uint8_t c = 0; c < cols; c++) {
      line[c] = vram_read();
    }
    vram_set_write_address(PAGE_BASE + (r-1) * cols);
    for (uint8_t r = 0; r < cols; r++) {
      vram_write(line[r]);
    }
  }
  vram_set_write_address(PAGE_BASE + cols * (ROWS - 1));
  for (uint8_t c = 0; c < cols; c++) {
    vram_write(' ');
  }    
}

static void
newline()
{
  if (row == ROWS - 1) {
    scroll_up();
  } else {
    row++;
  }
  col = 0;
  set_vram_write_address_to_cursor();
}

static char char_under_cursor = ' ';

void
hide_cursor()
{
  set_vram_write_address_to_cursor();
  vram_write(char_under_cursor);
  set_vram_write_address_to_cursor();
}

void
show_cursor()
{
  set_vram_read_address_to_cursor();
  char_under_cursor = vram_read();
  set_vram_write_address_to_cursor();
  vram_write('\177');
}

void
put_char_internal(char c)
{
  switch (c) {
  case '\r':
    col = 0;
    set_vram_write_address_to_cursor();
    break;
  case '\n':
    newline();
    break;
  case '\f':
    clear_screen();
    break;
  case '\177':
    if (col > 0) {
      col--;
      set_vram_write_address_to_cursor();
      vram_write(' ');
      set_vram_write_address_to_cursor();
    }
    break;
  case '\b':
    if (col > 0) {
      col--;
      set_vram_write_address_to_cursor();
    }
    break;
  default:
    if (c >= ' ') {
      vram_write(c);
      col++;
      if (col == cols) {
        col = 0;
        newline();
      }
    }
    break;
  }
}

void
put_char(char c)
{
  hide_cursor();
  put_char_internal(c);
  show_cursor();
}

void
put_string(const char* s)
{
  hide_cursor();
  while (*s) {
    put_char_internal(*s++);
  }
  show_cursor();
}

char
keyboard_get()
{
  if (z80_inp(KEYBOARD_STATUS) & KEYBOARD_RXRDY) {
    uint8_t c = z80_inp(KEYBOARD_DATA);
    if (!(c & 0x80)) {
      return c;
    }
  }
  return 0;
}
