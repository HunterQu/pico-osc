// -------------------------------------------------- //
// This file is autogenerated by pioasm; do not edit! //
// -------------------------------------------------- //

#pragma once

#if !PICO_NO_HARDWARE
#include "hardware/pio.h"
#endif

// -------------- //
// st7789_tft_spi //
// -------------- //

#define st7789_tft_spi_wrap_target 0
#define st7789_tft_spi_wrap 1

static const uint16_t st7789_tft_spi_program_instructions[] = {
            //     .wrap_target
    0x6001, //  0: out    pins, 1         side 0     
    0x1000, //  1: jmp    0               side 1     
            //     .wrap
};

#if !PICO_NO_HARDWARE
static const struct pio_program st7789_tft_spi_program = {
    .instructions = st7789_tft_spi_program_instructions,
    .length = 2,
    .origin = -1,
};

static inline pio_sm_config st7789_tft_spi_program_get_default_config(uint offset) {
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset + st7789_tft_spi_wrap_target, offset + st7789_tft_spi_wrap);
    sm_config_set_sideset(&c, 1, false, false);
    return c;
}
#endif
