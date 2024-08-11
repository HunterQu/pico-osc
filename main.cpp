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
#include "font_Montez_Regular_30.h"
#include "font_Serif_plain_12.h"
#include "font_robot_12.h"

#define SWL 22
#define SW2 25
#define SW4 23
#define SWR 24

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

void init_switch()
{
    gpio_init(SWL);
    gpio_init(SW2);
    gpio_init(SW4);
    gpio_init(SWR);

    gpio_pull_up(SWL);
    gpio_pull_up(SW2);
    gpio_pull_up(SW4);
    gpio_pull_up(SWR);

    gpio_set_dir(SWL, GPIO_IN);
    gpio_set_dir(SW2, GPIO_IN);
    gpio_set_dir(SW4, GPIO_IN);
    gpio_set_dir(SWR, GPIO_IN);
}

void init_all(){
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

    init_switch();
}

enum STATUS_TYPE{
    MENU = 0,
    WAVE
}sys_status;
#define MENU_SIZE 3
const char *menu_list[MENU_SIZE] = {
    "OK",
    "y-Axie:",
    ":)"
};
uint8_t now_selected            =   0;
bool    need_draw_menu          =   1;
bool    need_draw_wave_fram     =   1;
bool    need_draw_wave          =   0;
uint64_t bt_time;

#define BT_MIN_TIME     (40 * 1000)
#define BT_LONG_TIME    (400 * 1000)

void gpio_callback(uint gpio, uint32_t events) {
    if (events == 4){ //press
        bt_time = time_us_64();
    } else if (events == 8) {//release
        uint64_t now_time = time_us_64();
        if (now_time - bt_time < BT_MIN_TIME){ // to short
            return;
        }
        if (now_time - bt_time > BT_LONG_TIME){
            switch (sys_status)
            {
            case WAVE:
                switch (gpio)
                {
                case SWL:
                    
                    break;
                case SWR:
                    
                    break;
                case SW2:
                    
                    sys_status = MENU;
                    now_selected = 0;
                    need_draw_menu = 1;
                    break;
                
                default:
                    break;
                }
                break;
            
            default:
                break;
            }
        } else {
            switch (sys_status)
            {
            case MENU:
                switch (gpio)
                {
                case SWL:
                    now_selected = (now_selected + 1) % MENU_SIZE;
                    need_draw_menu = 1;
                    printf("now_selected:%d\n", now_selected);
                    break;
                case SWR:
                    now_selected = (now_selected - 1 + MENU_SIZE) % MENU_SIZE;
                    need_draw_menu = 1;
                    break;
                case SW2:
                    
                    switch (now_selected)
                    {
                    case 0:
                        sys_status = WAVE;
                        need_draw_wave_fram = 1;
                        need_draw_wave = 1;
                        break;
                    
                    default:
                        break;
                    }

                    break;
                default:
                    break;
                }
                break;
            
            case WAVE:

                break;
            }
        }
    }
}

char text_buff[256];

#define MENU_X_OFFSET   50
#define MENU_Y_OFFSET   50
#define MENU_FONT       Serif_plain_12
#define MENU_COLOR      RGB565(255, 255, 0)
void draw_menu(){
    printf("draw_menu\n");

    st7789_set_dir(0);
    st7789_fill_dma_blocking(0x0000);

    for (uint8_t i = 0; i < MENU_SIZE; ++i) {
        if (i == now_selected) {
            // 在当前选中的菜单项前添加前缀"> "
            snprintf(text_buff, sizeof(text_buff), "> %s", menu_list[i]);
        } else {
            // 正常显示未选中的菜单项
            snprintf(text_buff, sizeof(text_buff), "  %s", menu_list[i]);
        }
        // 绘制菜单项
        st7789_draw_text(text_buff, 
                        strlen(text_buff), 
                        &MENU_FONT, 
                        MENU_X_OFFSET, 
                        MENU_Y_OFFSET + i * MENU_FONT.yAdvance, 
                        MENU_COLOR);
    }
}

#define WAVE_FONT Serif_plain_12
#define WAVE_NUM_FONT Roboto_Light_12
#define WAVE_INDI_COLOR     RGB565(233, 150, 0)

void draw_wave_fram(){

    st7789_set_dir(0);

    st7789_fill_dma_blocking(0x0000);

    snprintf(text_buff, sizeof(text_buff), "1234mv");
    st7789_draw_text(text_buff, strlen(text_buff), &WAVE_NUM_FONT,
                    0, 50, WAVE_INDI_COLOR);

    snprintf(text_buff, sizeof(text_buff), "1294707756137560756107");
    st7789_draw_text(text_buff, strlen(text_buff), &WAVE_NUM_FONT,
                    50, 170, WAVE_INDI_COLOR);
}

#define WAVE_X_OFFSET   12
#define WAVE_Y_OFFSET   50
#define WAVE_X_LEN      160
#define WAVE_Y_LEN      280

void wave_draw_at_a_fix_div(float div){
    st7789_set_dir(1);

    read_ad_pio_set_div_clk(div);
    read_ad_pio_dma(300, ad_data.data);
    // ad_data.len += 300;

    uint8_t f = 0;
    st7789_set_windows(WAVE_X_OFFSET, WAVE_Y_OFFSET, WAVE_X_OFFSET + WAVE_X_LEN, WAVE_Y_OFFSET + WAVE_Y_LEN);
    for (int i = 0; i < WAVE_Y_LEN; i++){
        ad_data.data[i] = 4095 - ad_data.data[i];
        ad_data.data[i] = ad_data.data[i] * WAVE_X_LEN / 4096;

        if (i){
            memset(screen_buffer(f), 0x00, sizeof(uint16_t) * ST7789_WIDTH);
            // if (!(i % 50)){
            //     memset(screen_buffer(f), 0xff, sizeof(uint16_t) * ST7789_WIDTH);
            // }
            for (int k = ad_data.data[i - 1]; k <= ad_data.data[i]; k++){
                screen_buffer(f)[k] = RGB565(255, 255, 0);
            }
            for (int k = ad_data.data[i]; k <= ad_data.data[i - 1]; k++){
                screen_buffer(f)[k] = RGB565(255, 255, 0);
            }
            st7789_draw_dma_blocking(screen_buffer(f), WAVE_X_LEN);
            f = 1 - f;
        }
    }
}

void show_handler(){
    switch (sys_status)
    {
    case MENU:
        if (need_draw_menu){
            draw_menu();
            need_draw_menu = 0;
        }
        break;
    case WAVE:
        if (need_draw_wave_fram){
            draw_wave_fram();
            need_draw_wave_fram = 0;
        }
        if (need_draw_wave){
            wave_draw_at_a_fix_div(2.0f);
            need_draw_wave = 0;
        }
        break;
    } 
}

int main()
{
    
    // test_draw_photo(0, 0);
    // test_ramdom_line();

    // while(1){
    //     test_draw_at_a_fix_div(1.0f);
    //     sleep_ms(50);
    // }    
    
    // while(1){
    //     draw_fft();
        
    //     sleep_ms(100);
    // }

    init_all();

    // while (1){
    //     show_handler();
    //     sleep_ms(1);
    // }

    printf("Hello GPIO IRQ\n");
    gpio_set_irq_enabled_with_callback(SWL, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(SW2, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(SW4, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(SWR, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    static uint64_t r_time = time_us_64();

    while(1){
        show_handler();
        sleep_ms(1);

        if (1){
            if (time_us_64() - r_time > 100 * 1000){
                need_draw_wave = 1;
                r_time = time_us_64();
            }
        }
    }
}