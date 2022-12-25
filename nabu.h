#ifndef _nabu_h
#define _nabu_h

#define HCCA_REGISTER    0x80
#define CONTROL_REGISTER 0x00

#define PSG_DATA         0x40
#define PSG_ADDRESS      0x41

#define PSG_REG_IO_A     0x0e
#define PSG_REG_IO_B     0x0f

#define INT_MASK_HCCARINT 0x80
#define INT_MASK_HCCATINT 0x40
#define INT_MASK_KEYBOARD 0x20
#define INT_MASK_VDP      0x10

#define VDP_DATA         0xa0
#define VDP_STATUS       0xa1

#define KEYBOARD_DATA    0x90
#define KEYBOARD_STATUS  0x91

#define KEYBOARD_RXRDY   0x02

#endif
