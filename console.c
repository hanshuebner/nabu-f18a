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

#define ROWS 24
#define COLS 80

// Writes a byte to databus for register access
inline void write_port(uint8_t value)
{
  z80_outp(0xA1, value);
}

// Reads a byte from databus for register access
inline uint8_t read_status_reg()
{
  return z80_inp(0xa1);
}

inline void register_set(uint8_t reg, uint8_t value)
{
  write_port(value);
  write_port(0x80 | reg);
}

inline void vram_set_write_address(uint16_t address)
{
  z80_outp(0xA1, address & 0xff);
  z80_outp(0xa1, 0x40 | (address >> 8) & 0x3f);
}

// Writes a byte to databus for vram access
inline void vram_write(uint8_t value)
{
  z80_outp(0xA0, value);
}

inline void vram_set_read_address(uint16_t address)
{
  z80_outp(0xA1, address & 0xff);
  z80_outp(0xA1, (address >> 8) & 0x3f);
}

// Reads a byte from databus for vram access
inline uint8_t vram_read()
{
  return z80_inp(0xa0);
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

void
console_init()
{
  clear_vram();
  // 80 chars per line
  register_set(0, R0_M4);
  register_set(1, R1_16K | R1_BLANK | R1_M1 | R1_SIZE);
  register_set(2, 0x04); // name table at 0x1000
  register_set(4, 0x00); // pattern table at 0x0000
  load_font(font_1);
}

static uint8_t row = 0;
static uint8_t col = 0;

void
clear_screen()
{
  vram_set_write_address(PAGE_BASE);
  for (uint16_t i = 0; i < ROWS * COLS; i++) {
    vram_write(' ');
  }
  row = col = 0;
}

void
put_string(const char* s)
{
  vram_set_write_address(PAGE_BASE + row * 80 + col);
  while (*s && row < ROWS) {
    uint16_t c = *s++;
    switch (c) {
    case '\r':
      col = 0;
      break;
    case '\n':
      col = 0;
      row++;
      break;
    default:
      vram_write(c);
      col++;
      break;
    }
    if ((col % COLS) == 0) {
      row++;
      col = 0;
    }
  }
}
