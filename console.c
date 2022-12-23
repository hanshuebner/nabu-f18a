#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <z80.h>

#include "nabu.h"
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
vdp_set_register(uint8_t reg, uint8_t value)
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

void
copy_to_vram(uint16_t destination, uint8_t* src, uint16_t count)
{
  vram_set_write_address(destination);
  for (int i = 0; i < count; i++) {
    vram_write(src[i]);
  }
}

static void
load_font(uint8_t* font)
{
  copy_to_vram(FONT_BASE, font, FONT_SIZE);
}

static void
clear_vram()
{
  vram_set_write_address(0);
  for (int i = 0; i < VDP_RAM_SIZE; i++) {
    vram_write(0);
  }
}

// unlocks the f18a
void
unlock_f18a()
{
  vdp_set_register(0x39, 0x1c);   // VR1/57, value 00011100, write once (corrupts VDPR1)
  vdp_set_register(0x39, 0x1c);   // write again (unlocks enhanced mode)
}

// lock the F18A (preserves all settings)
void
lock_f18a()
{
  vdp_set_register(0x39, 0x00);       // VR1/57, value 00000000 (corrupts VDPR1 if already locked)
}

bool f18a_detected = false;

static void
check_f18a()
{
  unlock_f18a();
  vdp_set_register(0x0f, 1);
  f18a_detected = (read_status_reg() & ~3) == 0xE0;
}

bool
has_f18a()
{
  return f18a_detected;
}

static uint8_t cols = 40;

static void
vdp_init()
{
  check_f18a();
  if (has_f18a()) {
    cols = 80;
  } else {
    cols = 40;
  }

  clear_vram();
  // 80 chars per line
  vdp_set_register(0, R0_M4); // R0_M4 selects 80 cols mode on the F18A, ignored by the TMS9918
  vdp_set_register(1, R1_16K | R1_BLANK | R1_M1 | R1_SIZE);
  vdp_set_register(2, 0x04); // name table at 0x1000
  vdp_set_register(4, 0x00); // pattern table at 0x0000
  vdp_set_register(7, 0xf0); // Colors: white on black
  load_font(font_1);
}

static void
keyboard_init()
{
  // ROM has initialized keyboard UART, nothing to be done here
}

static void
psg_init()
{
  z80_outp(PSG_ADDRESS, '\000'); // C0 Fine tune
  z80_outp(PSG_DATA, 80);
  z80_outp(PSG_ADDRESS, '\001'); // C0 Coarse tune
  z80_outp(PSG_DATA, 0);
  z80_outp(PSG_ADDRESS, '\007'); // Mixer 1
  z80_outp(PSG_DATA, 0x7e);
  z80_outp(PSG_ADDRESS, '\010'); // Mixer 2
  z80_outp(PSG_DATA, 0x0);
  z80_outp(PSG_ADDRESS, '\013'); // Envelope fine tune
  z80_outp(PSG_DATA, 0);
  z80_outp(PSG_ADDRESS, '\014'); // Envelope coarse tune
  z80_outp(PSG_DATA, 20);
  z80_outp(PSG_ADDRESS, '\015'); // Envelope shape
  z80_outp(PSG_DATA, 0x00);
}

void
console_init()
{
  vdp_init();
  keyboard_init();
  psg_init();
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

static char char_under_cursor = ' ';
static bool cursor_enabled = true;

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
  if (cursor_enabled) {
    set_vram_read_address_to_cursor();
    char_under_cursor = vram_read();
    set_vram_write_address_to_cursor();
    vram_write('\177');
  }
}

uint8_t
get_row()
{
  return row;
}

uint8_t
get_col()
{
  return col;
}

void
set_cursor(uint8_t r, uint8_t c)
{
  hide_cursor();
  row = r < ROWS ? r : (ROWS - 1);
  col = c < cols ? c : (cols - 1);
  show_cursor();
}

void
clear_screen()
{
  hide_cursor();
  vram_set_write_address(PAGE_BASE);
  for (uint16_t i = 0; i < ROWS * cols; i++) {
    vram_write(' ');
  }
  row = col = 0;
  show_cursor();
}

uint8_t scroll_first_line = 0;
uint8_t scroll_last_line = ROWS - 1;

void
set_scroll_region(uint8_t first_line, uint8_t last_line)
{
  if (first_line <= last_line && last_line < ROWS) {
    scroll_first_line = first_line;
    scroll_last_line = last_line;
  }
}

void
scroll_up()
{
  static uint8_t line[MAXCOLS];
  for (uint8_t r = scroll_first_line + 1; r <= scroll_last_line; r++) {
    vram_set_read_address(PAGE_BASE + r * cols);
    for (uint8_t c = 0; c < cols; c++) {
      line[c] = vram_read();
    }
    vram_set_write_address(PAGE_BASE + (r-1) * cols);
    for (uint8_t r = 0; r < cols; r++) {
      vram_write(line[r]);
    }
  }
  vram_set_write_address(PAGE_BASE + cols * scroll_last_line);
  for (uint8_t c = 0; c < cols; c++) {
    vram_write(' ');
  }    
}

static void
newline()
{
  if (row == scroll_last_line) {
    scroll_up();
  } else if (row < ROWS) {
    row++;
  }
  col = 0;
  set_vram_write_address_to_cursor();
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

void
cursor_on()
{
  cursor_enabled = true;
}

void
cursor_off()
{
  cursor_enabled = false;
}

void
beep()
{
  z80_outp(PSG_ADDRESS, '\015'); // Envelope shape
  z80_outp(PSG_DATA, 0x00);
  z80_outp(PSG_ADDRESS, '\010'); // Mixer 2
  z80_outp(PSG_DATA, 0x10);
}
