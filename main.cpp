#include <stdio.h>
#include <string.h>
#include "hardware/spi.h"
#include "driver/st7789.h"
#include "driver/read_ad_data.h"
#include "pico/rand.h"
#include "pico/stdlib.h"
#include "image_data.h"

const struct st7789_config lcd_config = {
    .spi      = spi0,
    .baud     = 65 * 1000 * 1000,
    .gpio_din = 19,
    .gpio_clk = 18,
    .gpio_cs  = -1,
    .gpio_dc  = 20,
    .gpio_rst = 21,
    .gpio_bl  = 26,
};

uint16_t screen_buffer[ST7789_WIDTH];

void test_fill_pio_dma(){
    uint32_t start_time = time_us_32();

    for (uint8_t i = 0; i < 50; i++){

        // st7789_fill(get_rand_32() & 0xffff);
        // st7789_fill_dma_blocking(get_rand_32() & 0xffff);
        read_ad_pio_dma(1000, ad_data.data);
        st7789_fill_pio_dma_blocking(ad_data.data[100]);
    } 
    uint32_t end_time = time_us_32();
    printf("Elapsed time:%.2f\n", (end_time - start_time) / 1000000.0f);
}

void test_ramdom_line(){
    // st7789_set_cursor(0, 80);
    while(1){
        for (int i = 0; i < ST7789_WIDTH; ++i){
            screen_buffer[i] = get_rand_32() & 0xffff;
            // screen_buffer[i] = RGB565(255, 0, 255);
        }
        st7789_set_windows(0, 0, ST7789_WIDTH, ST7789_HEIGHT);
        for (int i = 0; i < ST7789_HEIGHT; ++i){
            st7789_draw_dma_blocking(screen_buffer, ST7789_WIDTH);
        }   
    }
}

void test_draw_photo(int x, int y){
    // st7789_set_cursor(0, 0);
    st7789_set_windows(x, y, x + image_width, y + image_height);

    for (int i = 0; i < image_height; i++){
        memcpy(screen_buffer, &image_data[i * image_width], image_width * sizeof(uint16_t));
        st7789_draw_dma_blocking(screen_buffer, image_width);
    }
}

void test_draw(float div){
    st7789_fill_dma_blocking(0x0000);


    read_ad_pio_set_div_clk(div);
    read_ad_pio_dma(300, ad_data.data);
    // ad_data.len += 300;

    st7789_set_windows(10, 10, 160, 310);
    for (int i = 0; i * 2 < 300; i++){
        ad_data.data[i] = 4095 - ad_data.data[i];
        ad_data.data[i] = ad_data.data[i] * 150 / 4096;

        if (i){
            memset(screen_buffer, 0, sizeof(screen_buffer));
            screen_buffer[ad_data.data[i]] = RGB565(255, 255, 0);
            st7789_draw_dma_blocking(screen_buffer, 150);

            memset(screen_buffer, 0, sizeof(screen_buffer));
            for (int k = ad_data.data[i - 1]; k <= ad_data.data[i]; k++){
                screen_buffer[k] = RGB565(255, 255, 0);
            }
            for (int k = ad_data.data[i]; k <= ad_data.data[i - 1]; k++){
                screen_buffer[k] = RGB565(255, 255, 0);
            }
            st7789_draw_dma_blocking(screen_buffer, 150);
        }
    }
}

int main()
{
    stdio_init_all();

    sleep_ms(1000);

    st7789_init(&lcd_config);

    st7789_set_bl_brightness(40);

    read_ad_init();
    
    // test_draw_photo(0, 0);
    // test_ramdom_line();

    while(1){
        test_draw(3.0f);
        sleep_ms(100);
    }
    // read_ad_pio_dma(1000)
}