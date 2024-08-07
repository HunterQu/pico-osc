#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "driver/st7789.h"
#include "pico/rand.h"

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
const int lcd_width = 172;
const int lcd_height = 320 ;

int main()
{
    stdio_init_all();

    sleep_ms(1000);

    st7789_init(&lcd_config, lcd_width, lcd_height);

    st7789_set_bl_brightness(40);
    
    uint32_t start_time = time_us_32();

    for (uint8_t i = 0; i < 50; i++){
        // st7789_fill(get_rand_32() & 0xffff);
        st7789_fill_dma_irq(get_rand_32() & 0xffff);
    } 
    uint32_t end_time = time_us_32();
    printf("Elapsed time:%.2f\n", (end_time - start_time) / 1000000.0f);

    extern uint16_t st7789_width;

    printf("%d", st7789_width);
}