/*
 * Copyright (c) 2021 Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 * 
 */
#include "st7789.h"
#include <string.h>
#include "pico/stdlib.h"    
#include "stdio.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
// #include <time.h>


#define offset_x 34
#define offset_y 0

static struct st7789_config st7789_cfg;
static uint16_t st7789_width;
static uint16_t st7789_height;
static bool st7789_data_mode = false;

static void st7789_cmd(uint8_t cmd, const uint8_t *data, size_t len)
{
    
    {
        spi_set_format(st7789_cfg.spi, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);
    }
    st7789_data_mode = false;

    // sleep_us(1);
    
    gpio_put(st7789_cfg.gpio_dc, 0);
    // sleep_us(1);

    spi_write_blocking(st7789_cfg.spi, &cmd, sizeof(cmd));

    if (len)
    {
        // sleep_us(1);
        gpio_put(st7789_cfg.gpio_dc, 1);
        // sleep_us(1);

        spi_write_blocking(st7789_cfg.spi, data, len);
    }

    // sleep_us(1);
    
    gpio_put(st7789_cfg.gpio_dc, 1);
    // sleep_us(1);
}

void st7789_caset(uint16_t xs, uint16_t xe)
{
    uint8_t data[] = {
        xs >> 8,
        xs & 0xff,
        xe >> 8,
        xe & 0xff,
    };

    // CASET (2Ah): Column Address Set
    st7789_cmd(0x2a, data, sizeof(data));
}

void st7789_raset(uint16_t ys, uint16_t ye)
{
    uint8_t data[] = {
        ys >> 8,
        ys & 0xff,
        ye >> 8,
        ye & 0xff,
    };

    // RASET (2Bh): Row Address Set
    st7789_cmd(0x2b, data, sizeof(data));
}

void st7789_init(const struct st7789_config *config, uint16_t width, uint16_t height)
{
    memcpy(&st7789_cfg, config, sizeof(st7789_cfg));
    st7789_width = width;
    st7789_height = height;

    // spi_init(st7789_cfg.spi, 20 * 1000 * 1000);
    uint baudrate = spi_init(st7789_cfg.spi, st7789_cfg.baud);
    printf("SPI baudrate: %d\n", baudrate);
    
    gpio_init(17);
    gpio_pull_down(17);
    gpio_put(17, false);
    
    {
        spi_set_format(st7789_cfg.spi, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);
    }

    gpio_set_function(st7789_cfg.gpio_din, GPIO_FUNC_SPI);
    gpio_set_function(st7789_cfg.gpio_clk, GPIO_FUNC_SPI);

    gpio_init(st7789_cfg.gpio_dc);
    gpio_init(st7789_cfg.gpio_rst);

    gpio_set_dir(st7789_cfg.gpio_dc, GPIO_OUT);
    gpio_set_dir(st7789_cfg.gpio_rst, GPIO_OUT);

    gpio_put(st7789_cfg.gpio_dc, 1);
    gpio_put(st7789_cfg.gpio_rst, 1);
    sleep_ms(100);

    // SWRESET (01h): Software Reset
    st7789_cmd(0x01, NULL, 0);
    sleep_ms(150);

    // SLPOUT (11h): Sleep Out
    st7789_cmd(0x11, NULL, 0);
    sleep_ms(50);

    // st7789_cmd(0xB1, (uint8_t[]){0x05, 0x3A, 0x3A}, 3);
    // st7789_cmd(0xB2, (uint8_t[]){0x05, 0x3A, 0x3A}, 3);
    // st7789_cmd(0xB3, (uint8_t[]){0x05, 0x3A, 0x3A, 0x05, 0x3A, 0x3A}, 6);
    // st7789_cmd(0xB4, (uint8_t[]){0x03}, 1);
    // st7789_cmd(0xC1, (uint8_t[]){0xC0}, 1);
    // st7789_cmd(0xC2, (uint8_t[]){0x0D, 0x00}, 2);
    // st7789_cmd(0xC3, (uint8_t[]){0x8D, 0x6A}, 2);
    // st7789_cmd(0xC4, (uint8_t[]){0x8D, 0xEE}, 2);
    // st7789_cmd(0xC5, (uint8_t[]){0x0E}, 1);
    // st7789_cmd(0xE0, (uint8_t[]){0x10, 0x0E, 0x02, 0x03, 0x0E, 0x07, 0x02, 0x07, 0x0A, 0x12, 0x27, 0x37, 0x00, 0x0D, 0x0E, 0x10}, 16);
    // st7789_cmd(0xE1, (uint8_t[]){0x10, 0x0E, 0x02, 0x03, 0x0E, 0x07, 0x02, 0x07, 0x0A, 0x12, 0x27, 0x37, 0x00, 0x0D, 0x0E, 0x10}, 16);

    // Declare the command data arrays
    uint8_t data_B1[] = {0x05, 0x3A, 0x3A};
    uint8_t data_B2[] = {0x05, 0x3A, 0x3A};
    uint8_t data_B3[] = {0x05, 0x3A, 0x3A, 0x05, 0x3A, 0x3A};
    uint8_t data_B4[] = {0x03};
    uint8_t data_C1[] = {0xC0};
    uint8_t data_C2[] = {0x0D, 0x00};
    uint8_t data_C3[] = {0x8D, 0x6A};
    uint8_t data_C4[] = {0x8D, 0xEE};
    uint8_t data_C5[] = {0x0E};
    uint8_t data_E0[] = {0x10, 0x0E, 0x02, 0x03, 0x0E, 0x07, 0x02, 0x07, 0x0A, 0x12, 0x27, 0x37, 0x00, 0x0D, 0x0E, 0x10};
    uint8_t data_E1[] = {0x10, 0x0E, 0x02, 0x03, 0x0E, 0x07, 0x02, 0x07, 0x0A, 0x12, 0x27, 0x37, 0x00, 0x0D, 0x0E, 0x10};

    // Use the arrays in the st7789_cmd function calls
    st7789_cmd(0xB1, data_B1, sizeof(data_B1));
    st7789_cmd(0xB2, data_B2, sizeof(data_B2));
    st7789_cmd(0xB3, data_B3, sizeof(data_B3));
    // st7789_cmd(0xB4, data_B4, sizeof(data_B4));
    st7789_cmd(0xC1, data_C1, sizeof(data_C1));
    st7789_cmd(0xC2, data_C2, sizeof(data_C2));
    st7789_cmd(0xC3, data_C3, sizeof(data_C3));
    st7789_cmd(0xC4, data_C4, sizeof(data_C4));
    st7789_cmd(0xC5, data_C5, sizeof(data_C5));
    st7789_cmd(0xE0, data_E0, sizeof(data_E0));
    st7789_cmd(0xE1, data_E1, sizeof(data_E1));

    // COLMOD (3Ah): Interface Pixel Format
    // - RGB interface color format     = 65K of RGB interface
    // - Control interface color format = 16bit/pixel
    uint8_t data_3a[] = {0x55};
    st7789_cmd(0x3a, data_3a, 1);
    sleep_ms(10);

    // MADCTL (36h): Memory Data Access Control
    // - Page Address Order            = Top to Bottom
    // - Column Address Order          = Left to Right
    // - Page/Column Order             = Normal Mode
    // - Line Address Order            = LCD Refresh Top to Bottom
    // - RGB/BGR Order                 = RGB
    // - Display Data Latch Data Order = LCD Refresh Left to Right
    uint8_t data_36[] = {0b01100000};
    // st7789_cmd(0x36, data_36, 1);

    st7789_caset(0 + offset_x, width + offset_x);
    st7789_raset(0 + offset_y, height + offset_y);

    // INVON (21h): Display Inversion On
    st7789_cmd(0x21, NULL, 0);
    sleep_ms(10);

    // NORON (13h): Normal Display Mode On
    st7789_cmd(0x13, NULL, 0);
    sleep_ms(10);

    // DISPON (29h): Display On
    st7789_cmd(0x29, NULL, 0);
    sleep_ms(10);

    // gpio_init(st7789_cfg.gpio_bl);
    // gpio_set_dir(st7789_cfg.gpio_bl, GPIO_OUT);
    // gpio_put(st7789_cfg.gpio_bl, true);

    gpio_set_function(st7789_cfg.gpio_bl, GPIO_FUNC_PWM);

    uint slice_num = pwm_gpio_to_slice_num(st7789_cfg.gpio_bl);

    pwm_config cfg = pwm_get_default_config();

    pwm_config_set_clkdiv(&cfg, 50.0f);

    pwm_init(slice_num, &cfg, true);

    pwm_set_wrap(slice_num, 99);
    // Set channel A output high for one cycle before dropping
    pwm_set_chan_level(slice_num, PWM_CHAN_A, 100);
    // Set initial B output high for three cycles before dropping
    pwm_set_chan_level(slice_num, PWM_CHAN_B, 100);
    // Set the PWM running
    pwm_set_enabled(slice_num, true);

    //clear
    st7789_fill(0x0000);

    //init DMA
    st7789_init_dma();

}

void st7789_set_bl_brightness(uint8_t b){
    uint slice_num = pwm_gpio_to_slice_num(st7789_cfg.gpio_bl);
    // Set channel A output high for one cycle before dropping
    pwm_set_chan_level(slice_num, PWM_CHAN_A, b);
    // Set initial B output high for three cycles before dropping
    pwm_set_chan_level(slice_num, PWM_CHAN_B, b);
}

void st7789_ramwr()
{
    sleep_us(1);
  
    gpio_put(st7789_cfg.gpio_dc, 0);
    sleep_us(1);

    // RAMWR (2Ch): Memory Write
    uint8_t cmd = 0x2c;
    spi_write_blocking(st7789_cfg.spi, &cmd, sizeof(cmd));

    sleep_us(1);

    gpio_put(st7789_cfg.gpio_dc, 1);
    sleep_us(1);
}

void st7789_write(const void *data, size_t len)
{
    if (!st7789_data_mode)
    {
        st7789_ramwr();

        {
            spi_set_format(st7789_cfg.spi, 16, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);
        }

        st7789_data_mode = true;
    }

    spi_write16_blocking(st7789_cfg.spi, (const uint16_t *)data, len / 2);
}

void st7789_put(uint16_t pixel)
{
    st7789_write(&pixel, sizeof(pixel));
}

void st7789_fill(uint16_t pixel)
{
    int num_pixels = st7789_width * st7789_height;

    st7789_set_cursor(0, 0);

    for (int i = 0; i < num_pixels; i++)
    {
        st7789_put(pixel);
    }
}

void st7789_set_cursor(uint16_t x, uint16_t y)
{
    st7789_caset(offset_x + x, offset_x + st7789_width);
    st7789_raset(offset_y + y, offset_y + st7789_height);
}

void st7789_set_windows(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd)
{
    st7789_caset(offset_x + xStart, offset_x + xEnd);
    st7789_raset(offset_y + yStart, offset_y + yEnd);
}

void st7789_vertical_scroll(uint16_t row)
{
    uint8_t data[] = {
        (row >> 8) & 0xff,
        row & 0x00ff};

    // VSCSAD (37h): Vertical Scroll Start Address of RAM
    st7789_cmd(0x37, data, sizeof(data));
}

static int dma_channel_clear_tx;
static volatile bool clear_in_process = false;

static void clear_complete(void){
    if (dma_channel_get_irq0_status(dma_channel_clear_tx)){
        dma_channel_acknowledge_irq0(dma_channel_clear_tx);

        clear_in_process = false;
    }
}

void st7789_init_dma(){
    //create dma channel for clear screen
    int dma_clear_channel = dma_claim_unused_channel(true);

    dma_channel_config c = dma_channel_get_default_config(dma_clear_channel);

    channel_config_set_dreq(&c, spi_get_dreq(st7789_cfg.spi, true));
    channel_config_set_read_increment(&c, false);
    channel_config_set_write_increment(&c, false);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_16);

    dma_channel_configure(dma_clear_channel, &c, &spi_get_hw(st7789_cfg.spi)->dr, nullptr, 0, false);

    //irq when finishing screen clear
    dma_set_irq0_channel_mask_enabled(dma_clear_channel, true);

    irq_set_exclusive_handler(DMA_IRQ_0, clear_complete);
    irq_set_enabled(DMA_IRQ_0, true);

    dma_channel_clear_tx = dma_clear_channel;
}

void st7789_fill_dma_blocking(uint16_t pixel){
    if (!st7789_data_mode)
    {
        st7789_ramwr();

        {
            spi_set_format(st7789_cfg.spi, 16, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);
        }

        st7789_data_mode = true;
    }
    dma_channel_transfer_from_buffer_now(dma_channel_clear_tx, &pixel, st7789_width * st7789_height);
    dma_channel_wait_for_finish_blocking(dma_channel_clear_tx);
}
void st7789_fill_dma_irq(uint16_t pixel){
    while (clear_in_process);
    dma_channel_transfer_from_buffer_now(dma_channel_clear_tx, &pixel, st7789_width * st7789_height);
    clear_in_process = true;
}