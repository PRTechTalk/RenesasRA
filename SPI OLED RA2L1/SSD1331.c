/*
 * SSD1331.c
 *
 *  Created on: 4 mars 2021
 *      Author: ranemyr
 */
#include "hal_data.h"
#include "stdio.h"
#include "SSD1331.h"
#include "common_utils.h"
#include "img.h"

fsp_err_t err = FSP_SUCCESS;
uint8_t sendBuff[1] = {0x00};
uint8_t i;
char buffer[40];

static volatile spi_event_t g_master_event_flag;    // Master Transfer Event completion flag
static unsigned char CHR_X, CHR_Y;



void _sendCmd(uint8_t c)
{
    // DC Low
    R_IOPORT_PinWrite(&g_ioport_ctrl, PMOD2_DC, BSP_IO_LEVEL_LOW);

    // CS Low
    R_IOPORT_PinWrite(&g_ioport_ctrl, PMOD2_CS, BSP_IO_LEVEL_LOW);

    _sendData(c);

    // CS High
    R_IOPORT_PinWrite(&g_ioport_ctrl, PMOD2_CS, BSP_IO_LEVEL_HIGH);
}


void _sendData(uint8_t c) {

    sendBuff[0] = c;
    g_master_event_flag = RESET_VALUE;

    err = R_SCI_SPI_Write(&g_spi0_ctrl, &sendBuff[0], sizeof(sendBuff), SPI_BIT_WIDTH_8_BITS);
    if (FSP_SUCCESS != err)
            {
                APP_ERR_PRINT("\r\n SCI_SPI Write failed");
                APP_ERR_TRAP(err);
            }
    while(SPI_EVENT_TRANSFER_COMPLETE != g_master_event_flag){
    }
}


/* Callback function */
void sci_spi_callback(spi_callback_args_t *p_args)
{
    /* TODO: add your own code here */
   if(SPI_EVENT_TRANSFER_COMPLETE == p_args->event){
       g_master_event_flag = SPI_EVENT_TRANSFER_COMPLETE;
   } else {
       g_master_event_flag = SPI_EVENT_TRANSFER_ABORTED;
   }
}



void wait_and_clear() {
    //delay_ms(1000);
    R_BSP_SoftwareDelay(1000, BSP_DELAY_UNITS_MILLISECONDS);

    // clear display
    SSD1331_drawFrame(0,0,96,64, COLOR_BLACK, COLOR_BLACK);

    //delay_us(500);
    R_BSP_SoftwareDelay(500, BSP_DELAY_UNITS_MICROSECONDS);
}




void SSD1331_init(void)
{

    err = R_SCI_SPI_Open(&g_spi0_ctrl, &g_spi0_cfg);
    if (FSP_SUCCESS != err)
        {
            APP_ERR_PRINT("\r\n SCI_SPI open failed");
            APP_ERR_TRAP(err);
        }



    // Power up and reset sequence
    R_IOPORT_PinWrite(&g_ioport_ctrl, PMOD2_CS, BSP_IO_LEVEL_LOW);

    // 1. Bring DC Pin 7 Low
    R_IOPORT_PinWrite(&g_ioport_ctrl, PMOD2_DC, BSP_IO_LEVEL_LOW);

    // 2.Bring Reset Pin 8 High
    R_IOPORT_PinWrite(&g_ioport_ctrl, PMOD2_RST, BSP_IO_LEVEL_HIGH);

    // 3. Bring VCCEN pin 9 Low
    R_IOPORT_PinWrite(&g_ioport_ctrl, PMOD2_VCCEN, BSP_IO_LEVEL_LOW);

    // 4. PMODEN High Pin 10 and delay 20ms
    R_IOPORT_PinWrite(&g_ioport_ctrl, PMOD2_PMODEN, BSP_IO_LEVEL_HIGH);
    R_BSP_SoftwareDelay(20, BSP_DELAY_UNITS_MILLISECONDS);

    // 5.Bring Reset Pin 8 Low and delay 5us then back to High
    R_IOPORT_PinWrite(&g_ioport_ctrl, PMOD2_RST, BSP_IO_LEVEL_LOW);
    R_BSP_SoftwareDelay(5, BSP_DELAY_UNITS_MICROSECONDS);
    R_IOPORT_PinWrite(&g_ioport_ctrl, PMOD2_RST, BSP_IO_LEVEL_HIGH);

    //6. Wait 5us for reset to complete
    R_BSP_SoftwareDelay(5, BSP_DELAY_UNITS_MICROSECONDS);

    // Power up Done Continue Init Sequence


    _sendData(0xFD);                     //Enable
    _sendData(0x12);

    _sendData(CMD_DISPLAY_OFF);          //Display Of

    _sendData(CMD_SET_REMAP);            //set remap & data format
    _sendData(0x72);                     //0x72

    _sendData(CMD_SET_DISPLAY_START_LINE);//Set display Start Line
    _sendData(0x0);

    _sendData(CMD_SET_DISPLAY_OFFSET);   //Set display offset
    _sendData(0x0);

    _sendData(CMD_NORMAL_DISPLAY);       //Set display mode

    _sendData(CMD_SET_MULTIPLEX_RATIO);  //Set multiplex ratio
    _sendData(0x3F);

    _sendData(CMD_SET_MASTER_CONFIGURE); //Set master configuration
    _sendData(0x8E);

    _sendData(CMD_POWER_SAVE_MODE);      //Set Power Save Mode
    _sendData(0x0B);                     //0x0B

    _sendData(CMD_PHASE_PERIOD_ADJUSTMENT);       //phase 1 and 2 period adjustment
    _sendData(0x31);                              //0x31

    _sendData(CMD_DISPLAY_CLOCK_DIV);    //display clock divider/oscillator frequency
    _sendData(0xF0);

    _sendData(CMD_SET_PRECHARGE_SPEED_A);//Set Second Pre-change Speed For ColorA
    _sendData(0x64);                     //100

    _sendData(CMD_SET_PRECHARGE_SPEED_B);//Set Second Pre-change Speed For ColorB
    _sendData(0x78);                     //120

    _sendData(CMD_SET_PRECHARGE_SPEED_C);//Set Second Pre-change Speed For ColorC
    _sendData(0x64);                     //100

    _sendData(CMD_SET_PRECHARGE_VOLTAGE);//Set Pre-Change Level
    _sendData(0x3A);

    _sendData(CMD_SET_V_VOLTAGE);        //Set vcomH
    _sendData(0x3E);

    _sendData(CMD_MASTER_CURRENT_CONTROL);//master current control
    _sendData(0x06);                     //6

    _sendData(CMD_SET_CONTRAST_A);       //Set contrast for color A
    _sendData(0x91);                     //145 (0x91)

    _sendData(CMD_SET_CONTRAST_B);       //Set contrast for color B
    _sendData(0x50);                     //80 (0x50)

    _sendData(CMD_SET_CONTRAST_C);       //Set contrast for color C
    _sendData(0x7D);                     //125 (0x7D)

    _sendData(CMD_DEACTIVE_SCROLLING);   //disable scrolling

    _sendData(CMD_CLEAR_WINDOW);
    _sendData(0x00);                     // Set the starting column coordinates
    _sendData(0x00);                     // Set the starting row coordinates
    _sendData(0x5F);                     // Set the finishing column coordinates
    _sendData(0x3F);                     // Set the finishing row coordinates

    //29. VCCEN pin 9 High wait 25ms
    R_IOPORT_PinWrite(&g_ioport_ctrl, PMOD2_VCCEN, BSP_IO_LEVEL_HIGH);
    R_BSP_SoftwareDelay(25, BSP_DELAY_UNITS_MILLISECONDS);

    //30. Turn on Display and wait 100ms
    _sendData(0xAF);
    R_BSP_SoftwareDelay(100, BSP_DELAY_UNITS_MILLISECONDS);

}


void SSD1331_drawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    if ((x < 0) || (x >= RGB_OLED_WIDTH) || (y < 0) || (y >= RGB_OLED_HEIGHT))
        return;
    //set column point
    _sendCmd(CMD_SET_COLUMN_ADDRESS);
    _sendCmd(x);
    _sendCmd(RGB_OLED_WIDTH-1);
    //set row point
    _sendCmd(CMD_SET_ROW_ADDRESS);
    _sendCmd(y);
    _sendCmd(RGB_OLED_HEIGHT-1);

    //GPIO_SetBits(GPIOB, GPIO_Pin_0); //dc
    R_IOPORT_PinWrite(&g_ioport_ctrl, PMOD2_DC, BSP_IO_LEVEL_HIGH);


    //GPIO_ResetBits(GPIOB, GPIO_Pin_1); //cs
    R_IOPORT_PinWrite(&g_ioport_ctrl, PMOD2_CS, BSP_IO_LEVEL_LOW);


    _sendData(color >> 8);
    _sendData(color);

    //GPIO_SetBits(GPIOB, GPIO_Pin_1); //cs
    R_IOPORT_PinWrite(&g_ioport_ctrl, PMOD2_CS, BSP_IO_LEVEL_HIGH);
}

void SSD1331_drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
    if((x0 < 0) || (y0 < 0) || (x1 < 0) || (y1 < 0))
        return;

    if (x0 >= RGB_OLED_WIDTH)  x0 = RGB_OLED_WIDTH - 1;
    if (y0 >= RGB_OLED_HEIGHT) y0 = RGB_OLED_HEIGHT - 1;
    if (x1 >= RGB_OLED_WIDTH)  x1 = RGB_OLED_WIDTH - 1;
    if (y1 >= RGB_OLED_HEIGHT) y1 = RGB_OLED_HEIGHT - 1;

    _sendCmd(CMD_DRAW_LINE);//draw line
    _sendCmd(x0);//start column
    _sendCmd(y0);//start row
    _sendCmd(x1);//end column
    _sendCmd(y1);//end row
    _sendCmd((uint8_t)((color>>11)&0x1F));//R
    _sendCmd((uint8_t)((color>>5)&0x3F));//G
    _sendCmd((uint8_t)(color&0x1F));//B
}

void SSD1331_drawFrame(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t outColor, uint16_t fillColor)
{
    if((x0 < 0) || (y0 < 0) || (x1 < 0) || (y1 < 0))
        return;

    if (x0 >= RGB_OLED_WIDTH)  x0 = RGB_OLED_WIDTH - 1;
    if (y0 >= RGB_OLED_HEIGHT) y0 = RGB_OLED_HEIGHT - 1;
    if (x1 >= RGB_OLED_WIDTH)  x1 = RGB_OLED_WIDTH - 1;
    if (y1 >= RGB_OLED_HEIGHT) y1 = RGB_OLED_HEIGHT - 1;

    _sendCmd(CMD_FILL_WINDOW);//fill window
    _sendCmd(ENABLE_FILL);
    _sendCmd(CMD_DRAW_RECTANGLE);//draw rectangle
    _sendCmd(x0);//start column
    _sendCmd(y0);//start row
    _sendCmd(x1);//end column
    _sendCmd(y1);//end row
    _sendCmd((uint8_t)((outColor>>11)&0x1F));//R
    _sendCmd((uint8_t)((outColor>>5)&0x3F));//G
    _sendCmd((uint8_t)(outColor&0x1F));//B
    _sendCmd((uint8_t)((fillColor>>11)&0x1F));//R
    _sendCmd((uint8_t)((fillColor>>5)&0x3F));//G
    _sendCmd((uint8_t)(fillColor&0x1F));//B
}

void SSD1331_drawCircle(uint16_t x, uint16_t y, uint16_t radius, uint16_t color) {
    signed char xc = 0;
    signed char yc = 0;
    signed char p = 0;

    // Out of range
    if (x >= RGB_OLED_WIDTH || y >= RGB_OLED_HEIGHT)
        return;

    yc = radius;
    p = 3 - (radius<<1);
    while (xc <= yc)
    {
        SSD1331_drawPixel(x + xc, y + yc, color);
        SSD1331_drawPixel(x + xc, y - yc, color);
        SSD1331_drawPixel(x - xc, y + yc, color);
        SSD1331_drawPixel(x - xc, y - yc, color);
        SSD1331_drawPixel(x + yc, y + xc, color);
        SSD1331_drawPixel(x + yc, y - xc, color);
        SSD1331_drawPixel(x - yc, y + xc, color);
        SSD1331_drawPixel(x - yc, y - xc, color);
        if (p < 0) p += (xc++ << 2) + 6;
            else p += ((xc++ - yc--)<<2) + 10;
    }

}

// Set current position in cache
void SSD1331_SetXY(unsigned char x, unsigned char y) {
    CHR_X = x;
    CHR_Y = y;
}

void SSD1331_XY_INK(LcdFontSize size) {
    CHR_X += 6*size;
    if (CHR_X + 6*size > RGB_OLED_WIDTH) {
        CHR_X = 0;
        CHR_Y += 8*size;
        if (CHR_Y + 8*size > RGB_OLED_HEIGHT) {
            CHR_Y = 0;
        }
    }
}

void SSD1331_Chr(LcdFontSize size, unsigned char ch, uint16_t chr_color, uint16_t bg_color) {
    unsigned char y, x, sx, sy;
    uint16_t color;
    /////uint16_t cx=CHR_X*6*size;
    uint16_t cx=CHR_X;
    /////uint16_t cy=CHR_Y*8*size;
    uint16_t cy=CHR_Y;

    if ( (cx + 6*size > RGB_OLED_WIDTH) || (cy + 8*size > RGB_OLED_HEIGHT) ) {
        return;
    }

    // CHR
    if ( (ch >= 0x20) && (ch <= 0x7F) )
    {
        // offset in symbols table ASCII[0x20-0x7F]
        ch -= 32;
    }
    else if (ch >= 0xC0)
    {
        // offset in symbols table CP1251[0xC0-0xFF] (Cyrillic)
        ch -= 96;
    }
    else
    {
        // Ignore unknown symbols
        ch = 95;
    }

    if ((size > FONT_1X) & (ch > 15) & (ch < 26)) {
        ch -= 16;
        for (sy = 0; sy<size; sy++) {
        for (y = 0; y<8; y++ ) {
            //set column point
            _sendCmd(CMD_SET_COLUMN_ADDRESS);
            _sendCmd(cx);
            _sendCmd(RGB_OLED_WIDTH-1);
            //set row point
            _sendCmd(CMD_SET_ROW_ADDRESS);
            _sendCmd(y + cy + sy*8);
            _sendCmd(RGB_OLED_HEIGHT-1);
            //GPIO_SetBits(GPIOB, GPIO_Pin_0); //dc
            R_IOPORT_PinWrite(&g_ioport_ctrl, PMOD2_DC, BSP_IO_LEVEL_HIGH);

            //GPIO_ResetBits(GPIOB, GPIO_Pin_1); //cs
            R_IOPORT_PinWrite(&g_ioport_ctrl, PMOD2_CS, BSP_IO_LEVEL_LOW);

            for (x = 0; x < 5*size; x++ ) {
                if ( (((BigNumbers[ch][x+sy*10] >> y) & 0x01 ) & (size == FONT_2X)) |
                     (((LargeNumbers[ch][x+sy*20] >> y) & 0x01 ) & (size == FONT_4X))

                    ) {
                    color = chr_color;
                }
                else {
                    color = bg_color;
                }
                _sendData(color >> 8);
                _sendData(color);
            }
        }
        }
    }
    else {
        for (y = 0; y<8; y++ ) {
            for (sy = 0; sy<size; sy++ ) {
                //set column point
                _sendCmd(CMD_SET_COLUMN_ADDRESS);
                _sendCmd(cx);
                _sendCmd(RGB_OLED_WIDTH-1);
                //set row point
                _sendCmd(CMD_SET_ROW_ADDRESS);
                _sendCmd(y*size + sy + cy);
                _sendCmd(RGB_OLED_HEIGHT-1);

                //GPIO_SetBits(GPIOB, GPIO_Pin_0); //dc
                R_IOPORT_PinWrite(&g_ioport_ctrl, PMOD2_DC, BSP_IO_LEVEL_HIGH);

                //GPIO_ResetBits(GPIOB, GPIO_Pin_1); //cs
                R_IOPORT_PinWrite(&g_ioport_ctrl, PMOD2_CS, BSP_IO_LEVEL_LOW);

                for (x = 0; x<5; x++ ) {
                    if ((FontLookup[ch][x] >> y) & 0x01) {
                        color = chr_color;
                    }
                    else {
                        color = bg_color;
                    }
                    //SSD1331_drawPixel(x+cx, y+cy, color);
                    for (sx = 0; sx<size; sx++ ) {
                        _sendData(color >> 8);
                        _sendData(color);
                    }
                }
                _sendData(bg_color >> 8);
                _sendData(bg_color);

                //GPIO_SetBits(GPIOB, GPIO_Pin_1); //cs
                R_IOPORT_PinWrite(&g_ioport_ctrl, PMOD2_CS, BSP_IO_LEVEL_HIGH);


            }
        }
    }

    /////CHR_X++;
    //CHR_X += 6*size;
}

// Print a string to display
void SSD1331_Str(LcdFontSize size, unsigned char dataArray[], uint16_t chr_color, uint16_t bg_color) {
    unsigned char tmpIdx=0;

    while( dataArray[ tmpIdx ] != '\0' )
    {
        /*/////
        if (CHR_X > 15) {
            CHR_X = 0;
            CHR_Y++;
            if (CHR_Y > 7) {
                CHR_Y = 0;
            }
        }
        */
        /*
        if (CHR_X + 6*size > RGB_OLED_WIDTH) {
            CHR_X = 0;
            CHR_Y += 8*size;
            if (CHR_Y + 8*size > RGB_OLED_HEIGHT) {
                CHR_Y = 0;
            }
        }*/

        SSD1331_Chr(size, dataArray[ tmpIdx ], chr_color, bg_color);
        SSD1331_XY_INK(size);
        tmpIdx++;
    }
}

void SSD1331_Str_XY(unsigned char x, unsigned char y, LcdFontSize size, unsigned char dataArray[], uint16_t chr_color, uint16_t bg_color) {
    SSD1331_SetXY(x, y);
    SSD1331_Str(size, dataArray, chr_color, bg_color);
}









// Print a string from the Flash to display
void SSD1331_FStr(LcdFontSize size, const unsigned char *dataPtr, uint16_t chr_color, uint16_t bg_color) {
    unsigned char c;
    for (c = *( dataPtr ); c; ++dataPtr, c = *( dataPtr ))
    {
        /*
        if (CHR_X > 15) {
            CHR_X = 0;
            CHR_Y++;
            if (CHR_Y > 7) {
                CHR_Y = 0;
            }
        }
        */

        SSD1331_Chr(size, c, chr_color, bg_color);
        SSD1331_XY_INK(size);
    }
}

void SSD1331_IMG(const unsigned char *img, uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
    uint16_t xx, yy;

    if ( (x + width > RGB_OLED_WIDTH) | (y+height > RGB_OLED_HEIGHT) ){
        return;
    }

    for (yy=0; yy<height; yy++) {
        //set column point
        _sendCmd(CMD_SET_COLUMN_ADDRESS);
        _sendCmd(x);
        _sendCmd(RGB_OLED_WIDTH-1);
        //set row point
        _sendCmd(CMD_SET_ROW_ADDRESS);
        _sendCmd(y + yy);
        _sendCmd(RGB_OLED_HEIGHT-1);

        //GPIO_SetBits(GPIOB, GPIO_Pin_0); //dc
        R_IOPORT_PinWrite(&g_ioport_ctrl, PMOD2_DC, BSP_IO_LEVEL_HIGH);

        //GPIO_ResetBits(GPIOB, GPIO_Pin_1); //cs
        R_IOPORT_PinWrite(&g_ioport_ctrl, PMOD2_CS, BSP_IO_LEVEL_LOW);


        for (xx=0; xx<width*2; xx++) {
            _sendData(img[yy*width*2 + xx]);
        }
    }
}

void SSD1331_copyWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,uint16_t x2, uint16_t y2)
{
    _sendCmd(CMD_COPY_WINDOW);//copy window
    _sendCmd(x0);//start column
    _sendCmd(y0);//start row
    _sendCmd(x1);//end column
    _sendCmd(y1);//end row
    _sendCmd(x2);//new column
    _sendCmd(y2);//new row
}

void SSD1331_dimWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    _sendCmd(CMD_DIM_WINDOW);//copy area
    _sendCmd(x0);//start column
    _sendCmd(y0);//start row
    _sendCmd(x1);//end column
    _sendCmd(y1);//end row
}

void SSD1331_clearWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    _sendCmd(CMD_CLEAR_WINDOW);//clear window
    _sendCmd(x0);//start column
    _sendCmd(y0);//start row
    _sendCmd(x1);//end column
    _sendCmd(y1);//end row
}

void SSD1331_setScrolling(ScollingDirection direction, uint8_t rowAddr, uint8_t rowNum, uint8_t timeInterval)
{
    uint8_t scolling_horizontal = 0x0;
    uint8_t scolling_vertical = 0x0;
    switch(direction){
        case Horizontal:
            scolling_horizontal = 0x01;
            scolling_vertical = 0x00;
            break;
        case Vertical:
            scolling_horizontal = 0x00;
            scolling_vertical = 0x01;
            break;
        case Diagonal:
            scolling_horizontal = 0x01;
            scolling_vertical = 0x01;
            break;
        default:
            break;
    }
    _sendCmd(CMD_CONTINUOUS_SCROLLING_SETUP);
    _sendCmd(scolling_horizontal);
    _sendCmd(rowAddr);
    _sendCmd(rowNum);
    _sendCmd(scolling_vertical);
    _sendCmd(timeInterval);
    _sendCmd(CMD_ACTIVE_SCROLLING);
}

void SSD1331_enableScrolling(bool enable)
{
    if(enable)
        _sendCmd(CMD_ACTIVE_SCROLLING);
    else
        _sendCmd(CMD_DEACTIVE_SCROLLING);
}

void SSD1331_setDisplayMode(DisplayMode mode)
{
    _sendCmd(mode);
}

void SSD1331_setDisplayPower(DisplayPower power)
{
    _sendCmd(power);
}

void demo(void){

        sprintf(buffer, "%s", "DEMO");
        SSD1331_Str_XY(5,5,FONT_2X,(unsigned char*)buffer, COLOR_WHITE, COLOR_BLUE);
        sprintf(buffer, "%s", "Lines");
        SSD1331_Str_XY(5,30,FONT_2X,(unsigned char*)buffer, COLOR_WHITE, COLOR_BLUE);

        wait_and_clear();

        //Lines
         for (i = 0; i < 24; i++) {
             SSD1331_drawLine(0, 0, i*4, 64, RGB(i*10,0,0));
             SSD1331_drawLine(96, 0, i*4, 64, RGB(i*10,0,0));
             //delay_ms(20);
             R_BSP_SoftwareDelay(20, BSP_DELAY_UNITS_MILLISECONDS);
         }

         wait_and_clear();
         sprintf(buffer, "%s", "DEMO");
         SSD1331_Str_XY(5,5,FONT_2X,(unsigned char*)buffer, COLOR_WHITE, COLOR_BLUE);
         sprintf(buffer, "%s", "Circle");
         SSD1331_Str_XY(5,30,FONT_2X,(unsigned char*)buffer, COLOR_WHITE, COLOR_BLUE);
         wait_and_clear();

         // circle
         SSD1331_drawCircle(48, 32, 5, COLOR_BLUE);
         //delay_ms(50);
         R_BSP_SoftwareDelay(50, BSP_DELAY_UNITS_MILLISECONDS);

         SSD1331_drawCircle(48, 32, 10, COLOR_GOLDEN);
         //delay_ms(50);
         R_BSP_SoftwareDelay(50, BSP_DELAY_UNITS_MILLISECONDS);
         SSD1331_drawCircle(48, 32, 15, COLOR_BLUE);
         //delay_ms(50);
         R_BSP_SoftwareDelay(50, BSP_DELAY_UNITS_MILLISECONDS);

         SSD1331_drawCircle(48, 32, 20, COLOR_GOLDEN);
         //delay_ms(50);
         R_BSP_SoftwareDelay(50, BSP_DELAY_UNITS_MILLISECONDS);
         SSD1331_drawCircle(48, 32, 25, COLOR_BLUE);

         //delay_ms(50);
         R_BSP_SoftwareDelay(50, BSP_DELAY_UNITS_MILLISECONDS);

         SSD1331_drawCircle(48, 32, 30, COLOR_GOLDEN);
         wait_and_clear();


         sprintf(buffer, "%s", "DEMO");
         SSD1331_Str_XY(5,5,FONT_2X,(unsigned char*)buffer, COLOR_WHITE, COLOR_BLUE);
         sprintf(buffer, "%s", "Rect");
         SSD1331_Str_XY(5,30,FONT_2X,(unsigned char*)buffer, COLOR_WHITE, COLOR_BLUE);
         wait_and_clear();

         // rect
         for (i = 0; i < 5; i++) {
             SSD1331_drawFrame(i*5, i*5, 96-i*5, 64-i*5, COLOR_RED, COLOR_YELLOW);
             //delay_ms(20);
             R_BSP_SoftwareDelay(20, BSP_DELAY_UNITS_MILLISECONDS);
         }
         SSD1331_drawFrame(25, 25, 71, 39, COLOR_BLUE, COLOR_GREEN);
         wait_and_clear();

         sprintf(buffer, "%s", "DEMO");
         SSD1331_Str_XY(5,5,FONT_2X,(unsigned char*)buffer, COLOR_WHITE, COLOR_BLUE);
         sprintf(buffer, "%s", "FONT 1x");
         SSD1331_Str_XY(5,30,FONT_2X,(unsigned char*)buffer, COLOR_WHITE, COLOR_BLUE);
         wait_and_clear();

         // text FONT_1X
         SSD1331_SetXY(0,0);
         for (i = 33; i < 126; i++) {
             SSD1331_Chr(FONT_1X, i, COLOR_BLUE, COLOR_BLACK);
             SSD1331_XY_INK(FONT_1X);
         }
         wait_and_clear();


         sprintf(buffer, "%s", "DEMO");
         SSD1331_Str_XY(5,5,FONT_2X,(unsigned char*)buffer, COLOR_WHITE, COLOR_BLUE);
         sprintf(buffer, "%s", "FONT 2x");
         SSD1331_Str_XY(5,30,FONT_2X,(unsigned char*)buffer, COLOR_WHITE, COLOR_BLUE);
         wait_and_clear();

         // text FONT_2X
         SSD1331_SetXY(0,0);
         for (i = 33; i < 64; i++) {
             SSD1331_Chr(FONT_2X, i, COLOR_BROWN, COLOR_BLACK);
             SSD1331_XY_INK(FONT_2X);
         }
         wait_and_clear();

         // Numeric FONT_2X
         sprintf(buffer, "%s", "3758");
         SSD1331_SetXY(0, 0);
         SSD1331_Str(FONT_2X,(unsigned char*)buffer, COLOR_WHITE, COLOR_BLACK);
         wait_and_clear();




         sprintf(buffer, "%s", "DEMO");
         SSD1331_Str_XY(5,5,FONT_2X,(unsigned char*)buffer, COLOR_WHITE, COLOR_BLUE);
         sprintf(buffer, "%s", "FONT 4x");
         SSD1331_Str_XY(5,30,FONT_2X,(unsigned char*)buffer, COLOR_WHITE, COLOR_BLUE);
         wait_and_clear();


         // Numeric FONT_4X
         SSD1331_SetXY(0, 0);
         SSD1331_Str(FONT_4X,(unsigned char*)buffer, COLOR_WHITE, COLOR_BLACK);
         wait_and_clear();



         sprintf(buffer, "%s", "DEMO");
         SSD1331_Str_XY(5,5,FONT_2X,(unsigned char*)buffer, COLOR_WHITE, COLOR_BLUE);
         sprintf(buffer, "%s", "Images");
         SSD1331_Str_XY(5,30,FONT_2X,(unsigned char*)buffer, COLOR_WHITE, COLOR_BLUE);
         wait_and_clear();

         // Images
             SSD1331_IMG(IMG0, 0,0, 96,64);
             wait_and_clear();

             SSD1331_IMG(IMG_LOGO, 0,0, 96,64);
             R_BSP_SoftwareDelay(3, BSP_DELAY_UNITS_SECONDS);
             wait_and_clear();


             SSD1331_IMG(IMG_AVATAR, 0,0, 96,64);
             R_BSP_SoftwareDelay(3, BSP_DELAY_UNITS_SECONDS);
             wait_and_clear();


             SSD1331_IMG(IMG1, 16,0, 64,64);
             wait_and_clear();

             // copy window
             SSD1331_IMG(IMG3, 32,16, 32,32);
             SSD1331_copyWindow(32,16,64,48, 0,0);
             wait_and_clear();

             // scrolling
             SSD1331_IMG(IMG2, 23,7, 50,50);
             SSD1331_setScrolling(Horizontal, 0, 64, 1);
             SSD1331_enableScrolling(true);

             wait_and_clear();
             SSD1331_enableScrolling(false);
             // text from Flash
             SSD1331_SetXY(0, 24);
             //SSD1331_FStr(FONT_1X,(unsigned char*)str_inFlash, COLOR_BLUE, COLOR_BLACK);


         while(1)
             {
             SSD1331_IMG(IMG_LOGO, 0,0, 96,64);
             R_BSP_SoftwareDelay(3, BSP_DELAY_UNITS_SECONDS);
             wait_and_clear();


             SSD1331_IMG(IMG_AVATAR, 0,0, 96,64);
             R_BSP_SoftwareDelay(3, BSP_DELAY_UNITS_SECONDS);
             wait_and_clear();

         }


}
