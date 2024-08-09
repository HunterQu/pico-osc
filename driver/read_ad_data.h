#ifndef _READ_AD_DATA_H_
#define _READ_AD_DATA_H_

#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "pico/types.h"

//AD_pin_out 11~0 <-> gpio 1~12
//ad_pin_otr      <-> gpio 0
#define AD_PIN_OTR      0
#define AD_PIN_EN       16
#define AD_PIN_CLK      13
#define AD_PIN_ST       1
#define AD_PIN_COUNT    12

//pio
#define AD_PIO_HANDLER  pio1

#define ad_data_mask    (((1 << AD_PIN_COUNT) - 1) << (AD_PIN_ST))

#define AD_DATA_CACHE_SIZE 2500

typedef struct ad_data_cache_s
{
    uint16_t len;
    uint16_t data[AD_DATA_CACHE_SIZE];
} ad_data_cache_t;

typedef struct _read_ad_pio_t
{
    uint sm;
    uint offset;
    pio_sm_config c;
    int dma_channel;
} read_ad_device_t;

extern ad_data_cache_t ad_data;
extern read_ad_device_t   ad_device;

void read_ad_init();
void read_ad_pio_init(uint pin_data_base, uint pin_clk, float clk_div);
uint32_t read_ad_pio_dma(int n, uint16_t * capture_buf);
void read_ad_pio_set_div_clk(float div_clk);

#endif