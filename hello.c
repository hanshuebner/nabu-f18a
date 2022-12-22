#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <z80.h>

#include "console.h"

#include "preamble.inc"

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

  while (true) {
    uint8_t c = keyboard_get();
    if (c) {
      if (c == '\r') {
        c = '\n';
      }
      put_char(c);
    }
  }
}
