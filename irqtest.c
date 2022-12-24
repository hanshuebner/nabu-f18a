#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <z80.h>

#include "nabu.h"
#include "console.h"

#include "preamble.inc"

void
print_count(uint16_t count)
{
  char buf[6];
  buf[5] = 0;
  for (int8_t i = 4; i >= 0; i--) {
    if (count || (i == 4)) {
      buf[i] = '0' + count % 10;
      count /= 10;
    } else {
      buf[i] = ' ';
    }
  }
  put_string(buf);
}

void
print_hex(uint16_t value)
{
  char buf[5];
  buf[4] = 0;
  for (int8_t i = 3; i >= 0; i--) {
    uint8_t nibble = value & 0x000f;
    buf[i] = nibble + ((nibble <= 9) ? '0' : ('A' - 10));
    value >>= 4;
  }
  put_string(buf);
}

extern uint16_t irq_table;
extern uint16_t hccar_count;
extern uint16_t hccat_count;
extern uint16_t keyb_count;
extern uint16_t vdp_count;
extern uint16_t option3_count;
extern uint16_t option2_count;
extern uint16_t option1_count;
extern uint16_t option0_count;

extern void init_interrupts();

main()
{
  static uint8_t server_col;
  console_init();
  clear_screen();
  cursor_off();

  z80_outp(PSG_ADDRESS, 0x0E);
  z80_outp(PSG_DATA, INT_MASK_VDP | INT_MASK_KEYBOARD);
  init_interrupts();

  set_cursor(0, 0);
  put_string("Interrupt test");

  set_cursor(2, 0);
  put_string("HCCA Receive: ");
  set_cursor(3, 0);
  put_string("HCCA Transmit: ");
  set_cursor(4, 0);
  put_string("Keyboard: ");
  set_cursor(5, 0);
  put_string("VDP: ");

  set_cursor(13, 0);
  put_string("Vectors:\n");
  uint16_t* p = (uint16_t*) &irq_table;
  print_hex(p[0x00]); put_char(' ');
  print_hex(p[0x01]); put_char(' ');
  print_hex(p[0x02]); put_char(' ');
  print_hex(p[0x03]); put_char(' ');
  put_char('\n');
  print_hex(p[0x04]); put_char(' ');
  print_hex(p[0x05]); put_char(' ');
  print_hex(p[0x06]); put_char(' ');
  print_hex(p[0x07]); put_char(' ');
  put_char('\n');
  put_string("Counter base: "); print_hex((uint16_t) &hccar_count); put_char('\n');
  put_string("Vector base: "); print_hex((uint16_t) &irq_table); put_char('\n');

  p = (uint16_t*) &hccar_count;
  uint16_t alive_count = 0;
  while (true) {
    set_cursor(2, 15);
    print_count(p[0]);
    print_count(hccar_count);

    set_cursor(3, 15);
    print_count(p[1]);
    print_count(hccat_count);

    set_cursor(4, 15);
    print_count(p[2]);
    print_count(keyb_count);

    set_cursor(5, 15);
    print_count(p[3]);
    print_count(vdp_count);

    set_cursor(6, 15);
    print_count(p[4]);

    set_cursor(7, 15);
    print_count(p[5]);

    set_cursor(8, 15);
    print_count(p[6]);

    set_cursor(9, 15);
    print_count(p[7]);

    set_cursor(23, 0);
    print_count(alive_count++);
  }  
}
