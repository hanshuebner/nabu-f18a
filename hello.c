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
  put_string("Hello WORLD!\n\n");
  put_string("This NABU has a F18A VDP replacement fitted.\n\n");
  put_string("It can thus display 80 characters per line\n");

  while (true) {}
}
