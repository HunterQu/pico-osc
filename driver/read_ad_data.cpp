#include <stdio.h>
#include "pico/stdlib.h"
#include "time.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "read_ad_data.h"
#include "string.h"
#include "build/read_ad_data.pio.h"
#include <stdlib.h>


ad_data_cache_t ad_data;
read_ad_device_t   ad_device;
    
void read_ad_pio_init(uint pin_data_base, uint pin_clk, float clk_div){
    //get offset of the pio program
    uint offset = pio_add_program(AD_PIO_HANDLER, &read_ad_data_program);

    // printf("program offset:%u\n", offset);
    // get a available sm
    uint sm = pio_claim_unused_sm(AD_PIO_HANDLER, true);

    // printf("sm_no: %u\n", sm);
    // init pins
    // for (int i = AD_PIN_ST; i < AD_PIN_ST + AD_PIN_COUNT; ++i){
    //     pio_gpio_init(AD_PIO_HANDLER, i);
    // }
    pio_gpio_init(AD_PIO_HANDLER, pin_clk);

    // set dir
    pio_sm_set_consecutive_pindirs(AD_PIO_HANDLER, sm, pin_data_base, AD_PIN_COUNT, false);
    pio_sm_set_consecutive_pindirs(AD_PIO_HANDLER, sm, pin_clk, 1, true);

    // get default config
    pio_sm_config c = read_ad_data_program_get_default_config(offset);

    // set side-set pins
    sm_config_set_sideset_pins(&c, pin_clk);
    // set input pins  
    sm_config_set_in_pins(&c, pin_data_base);

    // set fifo join to rx
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_RX);

    // set clk_div 
    sm_config_set_clkdiv(&c, clk_div);

    sm_config_set_in_shift(&c, false, true, AD_PIN_COUNT);

    // enable sm
    pio_sm_init(AD_PIO_HANDLER, sm, offset, &c);
    // pio_sm_set_enabled(AD_PIO_HANDLER, sm, true);


    ad_device.c = c;
    ad_device.sm = sm;
    ad_device.offset = offset;
}

static void init_dma(){
    int dma_clear_channel = dma_claim_unused_channel(true);

    dma_channel_config c = dma_channel_get_default_config(dma_clear_channel);

    channel_config_set_dreq(&c, pio_get_dreq(AD_PIO_HANDLER, ad_device.sm, false));
    channel_config_set_read_increment(&c, false);
    channel_config_set_write_increment(&c, true);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_16);

    dma_channel_configure(dma_clear_channel, &c, nullptr, &AD_PIO_HANDLER->rxf[ad_device.sm], 0, false);

    ad_device.dma_channel = dma_clear_channel;

}

void read_ad_init(){
    gpio_init(AD_PIN_EN);
    gpio_init(AD_PIN_OTR);

    gpio_set_dir(AD_PIN_EN, GPIO_OUT);
    gpio_set_dir(AD_PIN_OTR, GPIO_IN);

    gpio_put(AD_PIN_EN, false);

    read_ad_pio_init(AD_PIN_ST, AD_PIN_CLK, 2.0f);

    init_dma();
}

uint32_t read_ad_pio_dma(int n, uint16_t * capture_buf){
    pio_sm_set_enabled(AD_PIO_HANDLER, ad_device.sm, false);

    pio_sm_clear_fifos(AD_PIO_HANDLER, ad_device.sm);
    pio_sm_restart(AD_PIO_HANDLER, ad_device.sm);

    dma_channel_transfer_to_buffer_now(ad_device.dma_channel, capture_buf, n);

    uint32_t st = time_us_32(), ed;
    pio_sm_set_enabled(AD_PIO_HANDLER, ad_device.sm, true);

    dma_channel_wait_for_finish_blocking(ad_device.dma_channel);
    ed = time_us_32();
    return ed - st;
}

void read_ad_pio_set_div_clk(float div_clk){
    pio_sm_set_enabled(AD_PIO_HANDLER, ad_device.sm, false);

    pio_sm_set_clkdiv(AD_PIO_HANDLER, ad_device.sm, div_clk);

    pio_sm_restart(AD_PIO_HANDLER, ad_device.sm);
}

// int main()
// {
//     stdio_init_all();

//     sleep_ms(1000);

//     read_ad_init();

//     sleep_ms(400);

//     // uint16_t *capture_buf = (uint16_t * )malloc(1000 * sizeof(uint16_t));
//     // uint16_t *capture_buf = (uint16_t * )malloc(1000 * sizeof(uint16_t));

//     uint16_t st = time_us_32(), ed;
//     read_ad_pio_dma(1000, ad_data.data);
//     ed = time_us_32();

//     printf("%dus\n", ed - st);
//     for (int i = 0; i < 1000; i++){
//         printf("%d, ", ad_data.data[i]);
//     }
//     return 0;
// }
