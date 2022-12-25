#include "nabu.h"

;;; NABU interrupt handling example.

;;; The NABU PC uses interrupt mode 2.  It has eight interrupt
;;; sources, which are numbered from 0 to 7.  Four sources are used by
;;; the on-board peripherals, and four are available to extension
;;; cards.
SECTION code_compiler
PUBLIC _init_interrupts
._init_interrupts
        ld      a, _irq_table/256
        ld      i, a
        im      2
        ei
        ret

;;; HCCA receiver - Receives characters until a newline character is
;;; read.  The newline character is not included in the receive
;;; buffer.  To start receiving, _hcca_receive_pointer needs to be
;;; pointed to the start of the receive buffer, _hcca_receive_max
;;; needs point at the last character of the buffer and the HCCA
;;; receive interrupt needs to be enabled.  When a message has been
;;; read, the HCCA receive interrupt is disabled and needs to be
;;; re-enabled to read the next message.
.hccar_irq
        push    af
        push    hl
        ld      hl, (_hcca_receive_pointer)
        in      a, (HCCA_REGISTER)
        ld      (hl), a
        cp      '\n'
        ;; save flags during increment + terminate buffer op
        push    af
        inc     l
        jr      nz, skip_receive_h
        inc     h
.skip_receive_h
        ld      (_hcca_receive_pointer), hl
        ;; zero-terminate receive buffer
        push    af
        xor     a
        ld      (hl), a
        pop     af
        ;; restore saved flags from newline comparison
        pop     af
        jr      z, end_of_received_message
        ld      bc, (_hcca_receive_max)
        and     a                                           ; clear carry
        sbc     hl, bc
        jr      nz, count_receive
.end_of_received_message
        ;; end of message, switch off the HCCA RX interrupt
        ld      a, PSG_REG_IO_A
        out     (PSG_ADDRESS), a
        in      a, (PSG_DATA)
        and     ~INT_MASK_HCCARINT
        push    af
        ld      a, PSG_REG_IO_A
        out     (PSG_ADDRESS), a
        pop     af
        out     (PSG_DATA), a
.count_receive
        ld      hl, _hccar_count
        jp      increment_counter

;;; HCCA transmitter.  To transmit, the _hcca_send_pointer needs to
;;; point to the start of the message.  The message ends with a zero
;;; character, which is not transmitted.
.hccat_irq
        push    af
        push    hl
        ld      hl, (_hcca_send_pointer)
        ld      a, (hl)
        out     (HCCA_REGISTER), a
        inc     l
        jr      nz, skip_send_h
        inc     h
.skip_send_h
        ld      (_hcca_send_pointer), hl
        ld      a, (hl)
        or      a
        jr      nz, increment_hccat_counter
        ;; end of message, switch off the HCCA TX interrupt
        ld      a, PSG_REG_IO_A
        out     (PSG_ADDRESS), a
        in      a, (PSG_DATA)
        and     ~INT_MASK_HCCATINT
        push    af
        ld      a, PSG_REG_IO_A
        out     (PSG_ADDRESS), a
        pop     af
        out     (PSG_DATA), a
.increment_hccat_counter
        ld      hl, _hccat_count
        jp      increment_counter

;;; Keyboard handler.  When a key is pressed, it is put into the
;;; _last_char variable which can be read from the user program.
.keyb_irq
        push    af
        push    hl
        in      a, (KEYBOARD_DATA)
        ld      hl, _last_char
        ld      (hl), a
        ld      hl, _keyb_count
        jp      increment_counter

;;; VDP interrupt handler.  The VDP issues an interrupt at the end (?)
;;; of each scan line.
.vdp_irq
        push    af
        push    hl
        in      a, (VDP_STATUS)
        ld      hl, _vdp_count
        jp      increment_counter

.option0_irq
        push    af
        push    hl
        ld      hl, _option0_count
        jp      increment_counter

.option1_irq
        push    af
        push    hl
        ld      hl, _option1_count
        jp      increment_counter

.option2_irq
        push    af
        push    hl
        ld      hl, _option2_count
        jp      increment_counter

.option3_irq
        push    af
        push    hl
        ld      hl, _option3_count
        jp      increment_counter

.increment_counter
        inc     (hl)
        jr      nz, skip
        inc     l
        jr      nz, skip_h
        inc     h
.skip_h
        inc     (hl)
.skip
        pop     hl
        pop     af
        ei
        reti

;;; The irq_table holds the interrupt vectors for the Z80 interrupt
;;; mode 2.
SECTION irq_table
PUBLIC _irq_table
._irq_table
        align   256
        dw      hccar_irq
        dw      hccat_irq
        dw      keyb_irq
        dw      vdp_irq
        dw      option0_irq
        dw      option1_irq
        dw      option2_irq
        dw      option3_irq

;;; Each interrupt is counted and can be read from the user program
SECTION variables
PUBLIC _hccar_count
._hccar_count dw 0
PUBLIC _hccat_count
._hccat_count dw 0
PUBLIC _keyb_count
._keyb_count dw 0
PUBLIC _vdp_count
._vdp_count dw 0
PUBLIC _option0_count
._option0_count dw 0
PUBLIC _option1_count
._option1_count dw 0
PUBLIC _option2_count
._option2_count dw 0
PUBLIC _option3_count
._option3_count dw 0

;;; Keyboard and HCCA variables
PUBLIC _last_char
._last_char dw 0
PUBLIC _hcca_send_pointer
._hcca_send_pointer dw 0
PUBLIC _hcca_receive_pointer
._hcca_receive_pointer dw 0
PUBLIC _hcca_receive_max
._hcca_receive_max dw 0
