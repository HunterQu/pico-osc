/*
 * Copyright (c) 2021 Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 * 
 */
#include "hardware/spi.h"
#include "pico/types.h"
#include "font_type.h"

#ifndef _PICO_ST7789_H_
#define _PICO_ST7789_H_

//Screen Size
#define ST7789_WIDTH    172
#define ST7789_HEIGHT   322
#define ST7789_OFFSET_X 34
#define ST7789_OFFSET_Y 0
#define ST7789_SIZE     (ST7789_HEIGHT * ST7789_WIDTH)

#define RGB565(r, g, b) ((((r)&0xf8) << 8) | (((g)&0xfc) << 3) | (b) >> 3) 

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

typedef struct _st7789_device
{
    uint16_t width;
    uint16_t height;
    uint16_t offset_x;
    uint16_t offset_y;
    bool     data_mode;
    volatile bool draw_in_process;
    int dma_channel_clear_tx;
    int dma_channel_draw_tx;
    uint     pio_sm;
} st7789_device;

extern st7789_device tftDevice;

void st7789_init(const struct st7789_config* config);
void st7789_set_dir(bool dir);
void st7789_write(const void* data, size_t len);
void st7789_put(uint16_t pixel);
void st7789_fill(uint16_t pixel);
void st7789_fill_dma_blocking(uint16_t pixel);
void st7789_set_cursor(uint16_t x, uint16_t y);
void st7789_vertical_scroll(uint16_t row);
void st7789_set_windows(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd);
void st7789_set_bl_brightness(uint8_t b);
void st7789_init_dma();
void st7789_fill_pio_dma_blocking(uint16_t pixel);
void st7789_draw_dma_blocking(uint16_t *data, uint32_t len);
void st7789_draw_dma_irq(uint16_t *data, uint32_t len);
void st7789_draw_char(char ch, const GFXfont *font, uint16_t x, uint16_t y, uint16_t ft_color, uint16_t bg_color = 0x0000);
void st7789_draw_text(const char * text, uint8_t len, const GFXfont *font, uint16_t x, uint16_t y, uint16_t ft_color, uint16_t bg_color = 0x0000);
#endif
