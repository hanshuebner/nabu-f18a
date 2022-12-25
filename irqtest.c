#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <z80.h>

#include "nabu.h"
#include "console.h"

#include "preamble.inc"

#define CMD_ECHO         0xa1
#define LINE_BUFFER_SIZE 40

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
print_u16x(uint16_t value)
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

void
print_u8x(uint8_t value)
{
  char buf[3];
  buf[2] = 0;
  for (int8_t i = 1; i >= 0; i--) {
    uint8_t nibble = value & 0x0f;
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
extern uint16_t option0_count;
extern uint16_t option1_count;
extern uint16_t option2_count;
extern uint16_t option3_count;

extern char last_char;

extern uint16_t hcca_send_pointer;
extern uint16_t hcca_receive_pointer;
extern uint16_t hcca_receive_max;

extern void init_interrupts();

uint8_t
get_interrupt_mask()
{
  uint8_t mask;
  __asm di __endasm;
  z80_outp(PSG_ADDRESS, PSG_REG_IO_A);
  mask = z80_inp(PSG_DATA);
  __asm ei __endasm;
  return mask;
}

void
set_interrupt_mask(uint8_t mask)
{
  __asm di __endasm;
  z80_outp(PSG_ADDRESS, PSG_REG_IO_A);
  z80_outp(PSG_DATA, mask);
  __asm ei __endasm;
}

char
kbhit()
{
  __asm di __endasm;
  char retval = last_char;
  last_char = 0;
  __asm ei __endasm;
  return retval;
}

main()
{
  set_interrupt_mask(0);
  init_interrupts();

  console_init(false);
  clear_screen();
  cursor_off();

  set_cursor(0, 0);
  put_string("Interrupt test");

  set_cursor(2, 0);
  put_string("HCCA Receive:\n");
  put_string("HCCA Transmit:\n");
  put_string("Keyboard:\n");
  put_string("VDP:\n");
  put_string("Option 0:\n");
  put_string("Option 1:\n");
  put_string("Option 2:\n");
  put_string("Option 3:\n");
  set_cursor(11, 0);
  put_string("Input:");
  set_cursor(14, 0);
  put_string("Received:");
  set_cursor(17, 0);
  put_string("Scan code:");

  hcca_write(CMD_ECHO);

  uint8_t input_pointer;
  char line_buffer[LINE_BUFFER_SIZE+1];
  input_pointer = 0;
  line_buffer[0] = 0;

  char receive_buffer[LINE_BUFFER_SIZE+1];
  hcca_receive_pointer = (uint16_t) receive_buffer;
  hcca_receive_max = (uint16_t) receive_buffer + LINE_BUFFER_SIZE;

  set_interrupt_mask(INT_MASK_VDP | INT_MASK_KEYBOARD | INT_MASK_HCCARINT);

  uint8_t interrupt_mask;
  while (true) {
    set_cursor(2, 15);
    print_count(hccar_count);

    set_cursor(3, 15);
    print_count(hccat_count);

    set_cursor(4, 15);
    print_count(keyb_count);

    set_cursor(5, 15);
    print_count(vdp_count);

    set_cursor(6, 15);
    print_count(option0_count);

    set_cursor(7, 15);
    print_count(option1_count);

    set_cursor(8, 15);
    print_count(option2_count);

    set_cursor(9, 15);
    print_count(option3_count);

    interrupt_mask = get_interrupt_mask();

    if (!(interrupt_mask & INT_MASK_HCCATINT)) {
      char c = kbhit();
      if (c) {
        set_cursor(18, 0); print_u8x(c);
        if (c >= ' ' && c < '\177' && input_pointer < LINE_BUFFER_SIZE) {
          set_cursor(12, input_pointer);
          put_char(c);
          line_buffer[input_pointer++] = c;
          line_buffer[input_pointer] = 0;
        } else if (c == '\r') {
          line_buffer[input_pointer++] = '\n';
          line_buffer[input_pointer] = 0;
          hcca_send_pointer = (uint16_t) line_buffer;
          input_pointer = 0;
          interrupt_mask |= INT_MASK_HCCATINT;
          set_cursor(12, 0);
          for (int i = 0; i < LINE_BUFFER_SIZE; i++) {
            put_char(' ');
          }
        }
      }
    }
    if (!(interrupt_mask & INT_MASK_HCCARINT)) {
      set_cursor(15, 0);
      for (int i = 0; i < LINE_BUFFER_SIZE; i++) {
        put_char(' ');
      }
      set_cursor(15, 0);
      put_string(receive_buffer);
      hcca_receive_pointer = (uint16_t) receive_buffer;
      interrupt_mask |= INT_MASK_HCCARINT;
    }

    set_interrupt_mask(interrupt_mask);
  }  
}
