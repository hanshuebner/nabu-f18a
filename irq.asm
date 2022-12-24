SECTION code_compiler
PUBLIC _init_interrupts
._init_interrupts
        ld      a, _irq_table/256
        ld      i, a
        im      2
        ei
        ret

.hccar_irq
        push    af
        push    hl
        ld      hl, _hccar_count
        jp      increment_counter
.hccat_irq
        push    af
        push    hl
        ld      hl, _hccat_count
        jp      increment_counter
.keyb_irq
        push    af
        push    hl
        ld      hl, _keyb_count
        in      a, (0x90)
        jp      increment_counter
.vdp_irq
        push    af
        push    hl
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
        jr      nc, skip
        inc     l
        jr      nc, skip_h
        inc     h
.skip_h
        inc     (hl)
.skip
        pop     hl
        pop     af
        ei
        reti

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
        db      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        db      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        db      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        db      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        db      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        db      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        db      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        db      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
SECTION counters
PUBLIC _hccar_count
._hccar_count dw 0
PUBLIC _hccat_count
._hccat_count dw 1
PUBLIC _keyb_count
._keyb_count dw 2
PUBLIC _vdp_count
._vdp_count dw 3
PUBLIC _option0_count
._option0_count dw 4
PUBLIC _option1_count
._option1_count dw 5
PUBLIC _option2_count
._option2_count dw 6
PUBLIC _option3_count
._option3_count dw 7
