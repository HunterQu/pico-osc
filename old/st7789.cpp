#include "st7789.h"
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include <stdio.h>

// 全局变量
sTftDevice tftDevice = {
    .width = ST7789_WIDTH,
    .height = ST7789_HEIGHT
};

static inline void tftSetDCCS(bool dc, bool cs){ //dc:1-data 0-cmd
    gpio_put_masked((1 << PIN_DC) | (1 << PIN_CS), (dc << PIN_DC) | (cs << PIN_CS));
}

static inline void tftWriteCmd(uint8_t cmd, const uint8_t *data, size_t len){
    //write cmd
    tftSetDCCS(0, 0);sleep_us(1);
    spi_write_blocking(SPI_PORT, &cmd, sizeof(cmd));sleep_us(1);
    tftSetDCCS(0, 1);sleep_us(1);

    //if have parameters
    if (len)
    {
        tftSetDCCS(1, 0);sleep_us(1);
        spi_write_blocking(SPI_PORT, data, len);sleep_us(1);
        tftSetDCCS(1, 1);sleep_us(1);
    }
    
}

static inline void tftWriteData(const uint16_t data){
    tftSetDCCS(1, 0);

    uint8_t dt[2];

    dt[0] = (data >> 8) & 0xff;
    dt[1] = data & 0xff;

    spi_write_blocking(SPI_PORT, dt, 2);

    tftSetDCCS(1, 1);
}
void tftCASet(uint16_t xs, uint16_t xe)
{
    uint8_t data[] = {
        (uint8_t)xs >> 8,
        (uint8_t)xs & 0xff,
        (uint8_t)xe >> 8,
        (uint8_t)xe & 0xff,
    };

    // CASET (2Ah): Column Address Set
    tftWriteCmd(ST7789_CASET, data, sizeof(data));
}

void tftRASet(uint16_t ys, uint16_t ye)
{
    uint8_t data[] = {
        (uint8_t) ys >> 8,
        (uint8_t) ys & 0xff,
        (uint8_t) ye >> 8,
        (uint8_t) ye & 0xff,
    };

    // RASET (2Bh): Row Address Set
    tftWriteCmd(ST7789_RASET, data, sizeof(data));
}
void tftInit(){
    // init SPI
    uint baudrate = spi_init(SPI_PORT, SPI_BAUDRATE);
    printf("SPI baudrate: %d\n", baudrate);
    
    //init gpio
    gpio_init(PIN_CS);
    gpio_init(PIN_DC);
    gpio_init(PIN_RST);
    gpio_init(PIN_BL);

    //set pins to OUT(true)
    gpio_set_dir(PIN_CS, true);
    gpio_set_dir(PIN_DC, true);
    gpio_set_dir(PIN_RST, true);
    gpio_set_dir(PIN_BL, true);

    //pull up CS
    gpio_put(PIN_CS, true);
    
    
    gpio_set_function(PIN_SCK , GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN_DC  , GPIO_FUNC_SPI);

    //reset screen
    gpio_put(PIN_RST, false);
    sleep_ms(100);
    gpio_put(PIN_RST, true);
    sleep_ms(100);

    // SWRESET (01h): Software Reset
    tftWriteCmd(ST7789_SWRESET, NULL, 0);
    sleep_ms(150);
    // SLPOUT (11h): Sleep Out
    tftWriteCmd(ST7789_SLPOUT, NULL, 0);
    sleep_ms(50);

    uint8_t rgbctrlParams[] = {0x05, 0x3A, 0x3A};
    uint8_t porctrlParams[] = {0x05, 0x3A, 0x3A};
    uint8_t frctrl1Params[] = {0x05, 0x3A, 0x3A, 0x05, 0x3A, 0x3A};
    uint8_t idsetParams[] = {0xC0};
    uint8_t vdvvrhenParams[] = {0x0D, 0x00};
    uint8_t vrhsParams[] = {0x8D, 0x6A};
    uint8_t vdvsetParams[] = {0x8D, 0xEE};
    uint8_t vcmofsetParams[] = {0x0E};
    uint8_t pvgamctrlParams[] = {0x10, 0x0E, 0x02, 0x03, 0x0E, 0x07, 0x02, 0x07, 0x0A, 0x12, 0x27, 0x37, 0x00, 0x0D, 0x0E, 0x10};
    uint8_t nvgamctrlParams[] = {0x10, 0x0E, 0x02, 0x03, 0x0E, 0x07, 0x02, 0x07, 0x0A, 0x12, 0x27, 0x37, 0x00, 0x0D, 0x0E, 0x10};
    uint8_t b4param[] = {0x03};

    tftWriteCmd(ST7789_RGBCTRL, rgbctrlParams, 3);
    tftWriteCmd(ST7789_PORCTRL, porctrlParams, 3);
    tftWriteCmd(ST7789_FRCTRL1, frctrl1Params, 6);
    tftWriteCmd(0xB4, b4param, 1);
    tftWriteCmd(ST7789_IDSET, idsetParams, 1);
    tftWriteCmd(ST7789_VDVVRHEN, vdvvrhenParams, 2);
    tftWriteCmd(ST7789_VRHS, vrhsParams, 2);
    tftWriteCmd(ST7789_VDVSET, vdvsetParams, 2);
    tftWriteCmd(ST7789_VCMOFSET, vcmofsetParams, 1);
    tftWriteCmd(ST7789_PVGAMCTRL, pvgamctrlParams, 16);
    tftWriteCmd(ST7789_NVGAMCTRL, nvgamctrlParams, 16);

    // COLMOD (3Ah): Interface Pixel Format
    // - RGB interface color format     = 65K of RGB interface
    // - Control interface color format = 16bit/pixel
    uint8_t colmodParams[] = {0x55};
    tftWriteCmd(ST7789_COLMOD, colmodParams, 1);
    sleep_ms(10);

    tftSetDirection(ST7789_DIRECTION_0);
    
    tftCASet(0, tftDevice.width);
    tftRASet(0, tftDevice.height);

    // INVON (21h): Display Inversion On
    tftWriteCmd(0x21, NULL, 0);
    sleep_ms(10);

    // NORON (13h): Normal Display Mode On
    tftWriteCmd(0x13, NULL, 0);
    sleep_ms(10);

    // DISPON (29h): Display On
    tftWriteCmd(0x29, NULL, 0);
    sleep_ms(10);


    gpio_put(PIN_BL, true);


    tftClear(ST7789_BLUE);
}



void tftSetWindow(uint16_t x, uint16_t y, uint16_t width, uint16_t height){
    uint16_t temp;

    //vertival
    temp = x + width;
    if (temp >= tftDevice.width)
    {
        temp = tftDevice.width - 1;
    }
    
    tftCASet(x, temp);

    temp = y + height;
    if (temp >= tftDevice.height)
    {
        temp =tftDevice.height -1;
    }
    
    tftRASet(y, temp);
}

void tftSetDirection(eTftDirection dir){
    // MADCTL (36h): Memory Data Access Control
    // - Page Address Order            = Top to Bottom
    // - Column Address Order          = Left to Right
    // - Page/Column Order             = Normal Mode
    // - Line Address Order            = LCD Refresh Top to Bottom
    // - RGB/BGR Order                 = RGB
    // - Display Data Latch Data Order = LCD Refresh Left to Right
    uint8_t cmd_data = 0x00;
     
    switch (dir)
    {
    case ST7789_DIRECTION_0:
        cmd_data = 0x60;
        break;
    
    default:
        break;
    }

    tftWriteCmd(ST7789_MADCTL, &cmd_data, 1);
}

void tftClear(uint16_t color){
    tftSetWindow(0, 0, tftDevice.width, tftDevice.height);

    size_t fillsize = ST7789_SIZE;
    for (size_t index = 0; index < fillsize; ++index){
        tftWriteData(color);
    }
}

void tftPlot(uint16_t x, uint16_t y, uint16_t color){
    tftSetWindow(x, y, 1, 1);
    tftWriteData(color);
}

void tftDrawArray(uint16_t *src, size_t len){
    for (size_t index = 0; index < len; ++index){
        tftWriteData(src[index]);
    }
}
