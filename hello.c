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

#define CMD_CHAT 0xa0

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

void
main()
{
  console_init();
  clear_screen();
  
  if (has_f18a()) {
    put_string("F18A found, 80 chars/line\n");
  } else {
    put_string("F18A not found, 40 chars/line\n");
  }

  set_scroll_region(0, 21);
  set_cursor(22, 0);
  put_string("----------------------------------------");

  hcca_write(CMD_CHAT);
  while (true) {
    if (has_interrupt(INT_MASK_KEYBOARD)) {
      uint8_t c = keyboard_get();
      if (c) {
        put_char(c);
        hcca_write(c);
      }
    } else if (has_interrupt(INT_MASK_HCCARINT)) {
      put_char(hcca_read());
    }
  }
}
