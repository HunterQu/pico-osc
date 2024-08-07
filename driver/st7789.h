/*
 * Copyright (c) 2021 Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 * 
 */
#include "hardware/spi.h"
#include "pico/types.h"

#ifndef _PICO_ST7789_H_
#define _PICO_ST7789_H_

//PIO
#define PIO_HANDLER pio0

struct st7789_config {
    spi_inst_t* spi;
    uint baud;
    uint gpio_din;
    uint gpio_clk;
    int gpio_cs;
    uint gpio_dc;
    uint gpio_rst;
    uint gpio_bl;
};

typedef struct st7789_device {
    
};

void st7789_init(const struct st7789_config* config, uint16_t width, uint16_t height);
void st7789_write(const void* data, size_t len);
void st7789_put(uint16_t pixel);
void st7789_fill(uint16_t pixel);
void st7789_fill_dma_blocking(uint16_t pixel);
void st7789_fill_dma_irq(uint16_t pixel);
void st7789_set_cursor(uint16_t x, uint16_t y);
void st7789_vertical_scroll(uint16_t row);
void st7789_set_windows(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd);
void st7789_set_bl_brightness(uint8_t b);
void st7789_init_dma();

#endif
