static void orgit() __naked {
  __asm
    org     0x140D
    nop
    nop
    di
  __endasm;
}

static void entry_point()
{
  extern void main();
  main();
}