#include <stdio.h>
#include <string.h>
#include "hardware/spi.h"
#include "hardware/vreg.h"
#include "hardware/clocks.h"
#include "driver/st7789.h"
#include "driver/read_ad_data.h"
#include "pico/rand.h"
#include "pico/stdlib.h"
#include "fft.h"

const struct st7789_config lcd_config = {
    .spi      = spi0,
    .baud     = 125 * 1000 * 1000,
    .gpio_din = 19,
    .gpio_clk = 18,
    .gpio_cs  = -1,
    .gpio_dc  = 20,
    .gpio_rst = 21,
    .gpio_bl  = 26,
};

uint16_t screen_buffer[ST7789_WIDTH * 2];
#define screen_buffer(i) (&screen_buffer[(i) * ST7789_WIDTH])

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

// void test_draw_photo(int x, int y){
//     // st7789_set_cursor(0, 0);
//     st7789_set_windows(x, y, x + image_width, y + image_height);

//     for (int i = 0; i < image_height; i++){
//         memcpy(screen_buffer, &image_data[i * image_width], image_width * sizeof(uint16_t));
//         st7789_draw_dma_blocking(screen_buffer, image_width);
//     }
// }

void test_draw_at_a_fix_div(float div){
    st7789_fill_dma_blocking(0x0000);


    read_ad_pio_set_div_clk(div);
    read_ad_pio_dma(300, ad_data.data);
    // ad_data.len += 300;

    uint8_t f = 0;
    st7789_set_windows(10, 10, 160, 310);
    for (int i = 0; i < 300; i++){
        ad_data.data[i] = 4095 - ad_data.data[i];
        ad_data.data[i] = ad_data.data[i] * 150 / 4096;

        if (i){
            memset(screen_buffer(f), 0, sizeof(uint16_t) * ST7789_WIDTH);
            if (!(i % 50)){
                memset(screen_buffer(f), 0xff, sizeof(uint16_t) * ST7789_WIDTH);
            }
            for (int k = ad_data.data[i - 1]; k <= ad_data.data[i]; k++){
                screen_buffer(f)[k] = RGB565(255, 255, 0);
            }
            for (int k = ad_data.data[i]; k <= ad_data.data[i - 1]; k++){
                screen_buffer(f)[k] = RGB565(255, 255, 0);
            }
            st7789_draw_dma_blocking(screen_buffer(f), 150);
            f = 1 - f;
        }
    }
}

void draw_fft(){
    int N = 512;
#define SAMPLING_RATE (100 * 1000 * 1000)
    
    read_ad_pio_set_div_clk(clock_get_hz(clk_sys) / SAMPLING_RATE / 2); //25mhz
    printf("time:%dus\n", read_ad_pio_dma(N + 30, ad_data.data));

    Complex fft_res[N];
    double magnitudeResult[N / 2];
    double frequencies[N / 2];

    for (int i = 0; i < N; i++){
        fft_res[i].real = ad_data.data[i];
        fft_res[i].imag = 0.0;
    }

    removeDCOffset(fft_res, N);

    fft(fft_res, N);

    printf("finish fft\n");

    for (int i = 0; i < N / 2; ++i) {
        magnitudeResult[i] = magnitude(fft_res[i]);
        frequencies[i] = i * (SAMPLING_RATE / N);
    }

    for (int i = 0; i < N / 2; ++i) {
        printf("%.2f, ", magnitudeResult[i]);
    }
    printf("\n");

    int peakIndex = 0;
    double peakmag = 0;
    for (int i = 1; i < N / 2; ++i) {
        if (magnitudeResult[i] > magnitudeResult[peakIndex]) {
            peakIndex = i;
            peakmag = magnitudeResult[i];
        }
    }

    double peakFrequency = frequencies[peakIndex];
    printf("\n");
    printf("%.2fHz\n", peakFrequency);


    uint8_t f = 0;
    st7789_set_windows(10, 10, 160, 310);
    for (int i = 0; i < 300; i++){
        magnitudeResult[i] = magnitudeResult[i] * 150 / peakmag;

        if (i){
            memset(screen_buffer(f), 0, sizeof(uint16_t) * ST7789_WIDTH);
            for (int k = 0; k <= (int)magnitudeResult[i]; ++k)
                screen_buffer(f)[k] = RGB565(255, 255, 0);
            // screen_buffer(f)[(int)magnitudeResult[i]] = RGB565(255, 255, 0);
            st7789_draw_dma_blocking(screen_buffer(f), 150);
            f = 1 - f;
        }
    }
}

int main()
{
    vreg_set_voltage(VREG_VOLTAGE_1_20); // 300MHz需要加到1.35v
    sleep_ms(10);
    bool sys_clk_ = set_sys_clock_khz(250 * 1000, false);

    uint32_t sys_freq = clock_get_hz(clk_sys);

    // clk_peri does not have a divider, so input and output frequencies will be the same
    clock_configure(clk_peri,
                        0,
                        CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS,
                        sys_freq,
                        sys_freq);

    stdio_init_all();

    sleep_ms(1000);

    if (sys_clk_) printf("system clock:%dmhz\n", clock_get_hz(clk_sys) / 1000 / 1000);

    st7789_init(&lcd_config);

    st7789_set_bl_brightness(40);

    read_ad_init();
    
    // test_draw_photo(0, 0);
    // test_ramdom_line();

    // while(1){
    //     test_draw_at_a_fix_div(1.0f);
    //     sleep_ms(50);
    // }    
    
    while(1){
        draw_fft();
        sleep_ms(100);
    }
}