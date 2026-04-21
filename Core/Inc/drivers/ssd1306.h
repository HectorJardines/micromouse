#ifndef _SSD1306_H
#define _SSD1306_H

#include <stdint.h>

/**
 * SSD1306 FUNDAMENTAL COMMAND BYTES
 */
#define CMD_SET_CONTRAST        (0x81U) // b1000 0001
#define CMD_ENT_DIS_ON_RAM      (0xA4) // turns on entire display and resumes RAM content
#define CMD_ENT_DIS_ON_NO_RAM   (0xA5)
#define CMD_DIS_ORIENT_NORMAL   (0xA6)
#define CMD_DIS_ORIENT_INVERT   (0xA7)
#define CMD_SET_DIS_OFF         (0xAE) // display off (sleep mode)
#define CMD_SET_DIS_ON         (0xAF)

/**
 * SSD1306 ADDRESSING SCHEME COMMAND BYTES
 */
#define CMD_SET_LOW_COL_STRT_ADDR_PG    (0x00) // set lower nibble of col start addr resg for Page addressing mode (using lower 4 bits)
#define CMD_SET_HI_COL_STRT_ADDR_PG     (0x10)

#define CMD_SET_MEM_ADDRESSING_MODE     (0x20) // SEND ADDITIONAL BYTE WITH LOWER 2 BITS AS ADDR MODE
#define OPT_ADDR_MODE_HORI              (0x00)
#define OPT_ADDR_MODE_VERT              (0x01)
#define OPT_ADDR_MODE_PAGE              (0x02)

#define CMD_SET_COL_STRT_END_ADDR       (0x21) // TRIPLE BYTE COMMAND REQUIRES 7 BIT COL START AND END ADDRS (VERT/HORI MODE ONLY)
#define CMD_SET_PAGE_STRT_END_ADDR      (0x22) // TRIPLE BYTE COMMAND SIMILAR AS COL (VERT/HORI MODE ONLY)
#define CMD_SET_PAGE_STRT_ADDR          (0xB0) // MSK LOWER TWO 3 BITS FOR START AT PAGE0-7 (PG ADDR MODE ONLY)


/**
 * SSD1306 HW CONFIG COMMAND BYTES (PANEL RES / LAYOUT)
 */
#define CMD_SET_DISPLAY_STRT_LINE   (0x40) // MSK LOWER 6 BITS FOR LINES 0-63
#define CMD_SET_SEGREMAP_NORMAL     (0xA0U)
#define CMD_SET_SEGREMAP_ALT        (0xA1U)
#define CMD_SET_MULTIPLEX_RATIO     (0xA8) // 2 BYTE COMMAND SUBSEQUENT BYTE CONATINS 6 BIT MUX RATIO VALUE (0-14 INVALID)
#define CMD_SET_COM_SCAN_NORMAL     (0xC0) // SCANS FROM COM0 - COM[N-1] WHERE N IS MUX RATIO
#define CMD_SET_COM_SCAN_REMAP      (0xC8)
#define CMD_SET_DIS_OFFSET          (0xD3) // 2 BYTE CMD; SUBSEQUENT 6 BIT DATA SETS VERTICAL SHIFT FROM COM0-63
#define CMD_SET_COM_HW_CFG          (0xDA) // 2 BYTE CMD; SUBSEQUENT A[5:4] SPECIFIED COM PIN CFG


/**
 * SSD1306 TIMING / DRIVING SCHEME CMD BYTES
 */
#define CMD_SET_CLK_DIV_OSC_FREQ    (0xD5) // 2 BYTE CMD; A[3:0] + 1 defines the display clk div ratio; A[7:4] INCREASE OSC FREQ
#define CMD_SET_PRE_CHARGE_T        (0xD9) // 2 BYTE CMD; A[7:4] defines phase 2 period; A[3:0] defines phase 1 period (BOTH UP TO 15 DCLK)

#define CMD_DEACTIVATE_SCROLL   (0x28U)
#define CMD_SET_CHARGEPUMP      (0x8D)
#define OPT_CHARGEPUMP_ON       (0x14)


/**
 * ENUMS
 */

typedef enum {
    COLOR_INVERSE,
    COLOR_BLACK,
    COLOR_WHITE
} ssd1306_color_e;

typedef enum {
    ORIENT_HORIZONTAL_NORMAL,
    ORIENT_HORIZONTAL_FLIPPED,
    ORIENT_VERT_ROT_RIGHT,
    ORIENT_VERT_ROT_LEFT
} ssd1306_orientation_e;


/**
 * USER APIS
 */

/**
 * @brief Initializes the SSD1306 driver configuration via I2C transactions
 * 
 * This API configures the SSD1306 display clock, addressing scheme, display layout, etc.
 * It also intializes the display hanlder with the appropriate i2c APIs.
 * 
 * @return void
 */
void ssd1306_init(void);

/**
 * @brief Stores a single pixel in ssd1306 GDDRAM
 * 
 * This API stores a single pixel, represented as an (x,y) coordinate pair in the
 * SSD1306's interal 128x64 bit GDDRAM. This API should be called to populate the 
 * buffer and ssd1306_display should be called to update the display contents.
 * 
 * @param dis_x_coord
 * @param dis_y_coord
 * @param color
 * 
 * @return negative if err; positive else
 */
int8_t ssd1306_draw_pixel(int16_t dis_x_coord, int16_t dis_y_coord, ssd1306_color_e color);

/**
 * @brief Updates the display contents with the contens stored in GDDRAM
 * 
 * Initiates an I2C transaction with the SSD1306 driver that loads all
 * of the pixels drawn (via ssd1306_draw_pixel) into the physical memory
 * of the ssd1306.
 * 
 * @return void
 */
void ssd1306_display(void);


#endif