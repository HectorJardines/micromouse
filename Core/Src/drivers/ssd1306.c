#include "ssd1306.h"
#include "i2c.h"
#include "log.h"
#include "defines.h"
#include <string.h>

#define SSD1306_OK  (0U)
#define SSD1306_ERR (1U)

#define SINGLE_BYTE     (2U) // need to send CTL byte and DATA/CMD byte as a pair
#define DISPLAY_HEIGHT  (64U)
#define DISPLAY_WIDTH    (128U)
#define SSD1306_DEV_ADDR        (0x3CU) // addr can be 0b0111100 or 0b0111101
/**
 *  
 * CTL BYTE FORMAT = Co Bit | D/C# Bit | b'0' * 6
 * Co - Continuation Bit; when cleared, next byte conatins data only
 * D/C# - when cleared, next byte is interpreted as command. when set, next byte is data stored in GDDRAM
 * 
 * GDDRAM STRUCTURE - 8 pages each with 8 rows and 128 columns each data write
 * stores a byte into column i of page n, each bit is stored MSB first.
 * i.e. D0 in row 0 and D7 in row 7
 */  
#define SSD1306_CTL_BYTE        (0x00U)
#define SSD1306_CTL_BYTE_DATA   (SSD1306_CTL_BYTE | 0x40U)
#define SSD1306_CTL_BYTE_CMD    (SSD1306_CTL_BYTE)
#define LEN_DATA_CTL_AND_DATA   (1025U)

/**************************
 *  STATIC DECLARATIONS
 **************************/
struct ssd1306_oled_disp_t {
    void (*i2c_init)(void);
    i2c_status_e (*i2c_tx)(uint8_t *data, uint32_t len);
    i2c_status_e (*i2c_rx)(uint8_t *data, uint32_t len);
    uint8_t *gddr_buf;
    uint16_t buf_len;
    uint16_t width;
    uint16_t height;
    ssd1306_orientation_e orientation;
};

// 1024 BYTES FOR OUR USE CASE, FIGURE OUT WHY THIS WORKS...
static uint8_t oled_display_buf[DISPLAY_HEIGHT * ((DISPLAY_WIDTH + 7) >> DIV_8)];
static uint8_t data_buf[LEN_DATA_CTL_AND_DATA]; // copy data buf here to include data ctl byte
static struct ssd1306_oled_disp_t display = {.i2c_init = i2c_init, .i2c_rx = i2c_receive, .i2c_tx = i2c_transmit_poll, 
    .gddr_buf = oled_display_buf, .width = DISPLAY_WIDTH, .height = DISPLAY_HEIGHT, 
    .orientation = ORIENT_HORIZONTAL_NORMAL};

static void set_addressing_scheme(void);
static int8_t ssd1306_pwr_on(void);
static int8_t send_single_byte_cmd(uint8_t cmd);
static int8_t send_multi_byte_cmd(uint8_t *cmds, uint8_t len);
static int8_t send_single_byte_data(uint8_t *data);
static int8_t send_multi_byte_data(uint8_t *data, uint32_t len);
static void clear_display(void);


/****************************
 *      SSD1306 APIs
 ****************************/

static bool initialized = false; 
 /**
 * @brief 
 */
void ssd1306_init(void) {
    i2c_set_dev_addr(SSD1306_DEV_ADDR);
    display.buf_len = (display.width * ((display.height + 7) >> DIV_8));
    display.i2c_init();
    set_addressing_scheme();
    ssd1306_pwr_on();
    clear_display();
    initialized = true;
}

/**
 * @brief updates the pixel position in GDDRAM
 * 
 * @note must call ssd1306_display to actually render the drawn pixels
 */
int8_t ssd1306_draw_pixel(int16_t dis_x_coord, int16_t dis_y_coord, ssd1306_color_e color) {
    if ((dis_x_coord < 0 || dis_x_coord >= DISPLAY_WIDTH) || (dis_y_coord < 0 || dis_y_coord >= DISPLAY_HEIGHT))
        return SSD1306_ERR; // pixel coordinate out of bounds
    switch(display.orientation) {
        case ORIENT_HORIZONTAL_NORMAL:
            // DO NOTHING X/Y DONT NEED TO BE MODIFIED
            break;
        // NOT IMPLEMENTED CURRENTLY
        case ORIENT_HORIZONTAL_FLIPPED:
        case ORIENT_VERT_ROT_RIGHT:
        case ORIENT_VERT_ROT_LEFT:
            break;
    }

    switch(color) {
        case COLOR_INVERSE:
            // 8 pages, 128 bytes per, add x coordinate
            display.gddr_buf[dis_x_coord + (dis_y_coord >> DIV_8) * display.width] ^= (1 << (dis_y_coord & 7));
            break;
        case COLOR_BLACK:
            display.gddr_buf[dis_x_coord + (dis_y_coord >> DIV_8) * display.width] &= ~(1 << (dis_y_coord & 7));
            break;
        case COLOR_WHITE:
            display.gddr_buf[dis_x_coord + (dis_y_coord >> DIV_8) * display.width] |= (1 << (dis_y_coord & 7));
            break;
    }
    return SSD1306_OK;
}

/**
 * @brief writes all bytes in display.gddr_buf into ssd1306 RAM
 */
void ssd1306_display(void) {
    set_addressing_scheme();
    uint16_t count = display.width * ((display.height + 7) >> DIV_8);
    uint8_t *display_data = display.gddr_buf;
    send_multi_byte_data(display_data, count);
}


/*************************
 *  STATIC DEFINITIONS
 *************************/

static void clear_display(void) {
    for (uint16_t i = 0; i < display.buf_len; i++)
        display.gddr_buf[i] = 0;
}

static int8_t ssd1306_pwr_on(void) {
    uint8_t init_cmds[] = {SSD1306_CTL_BYTE_CMD,
                        CMD_SET_DIS_OFF, 
                        CMD_SET_CLK_DIV_OSC_FREQ, 
                        0x80, 
                        CMD_SET_MULTIPLEX_RATIO,
                        DISPLAY_HEIGHT - 1};
    int8_t res = send_multi_byte_cmd(init_cmds, sizeof(init_cmds));

    uint8_t layout_cmds[] = {SSD1306_CTL_BYTE_CMD,
                            CMD_SET_DIS_OFFSET,
                            0x00,
                            CMD_SET_DISPLAY_STRT_LINE | 0x00,
                            CMD_SET_COM_SCAN_NORMAL,
                            CMD_SET_SEGREMAP_NORMAL};
    res = send_multi_byte_cmd(layout_cmds, sizeof(layout_cmds));

    uint8_t hw_config_cmds[] = {SSD1306_CTL_BYTE_CMD,
                                CMD_SET_COM_HW_CFG,
                                0x12,
                                CMD_SET_CONTRAST,
                                0x9F,
                                CMD_SET_PRE_CHARGE_T,
                                0xF1, // phase1 = 15, phase2 = 1
                                CMD_SET_CHARGEPUMP,
                                0x14}; // chargepump on
    res = send_multi_byte_cmd(hw_config_cmds, sizeof(hw_config_cmds));

    uint8_t display_on_cmds[] = {SSD1306_CTL_BYTE_CMD,
                                CMD_ENT_DIS_ON_RAM,
                                CMD_DIS_ORIENT_NORMAL,
                                CMD_DEACTIVATE_SCROLL,
                                CMD_SET_DIS_ON};
    res = send_multi_byte_cmd(display_on_cmds, sizeof(display_on_cmds));

    return res;
}

static void set_addressing_scheme(void) {
    uint8_t hori_addressing[] = {SSD1306_CTL_BYTE_CMD, 
                                CMD_SET_MEM_ADDRESSING_MODE, 
                                OPT_ADDR_MODE_HORI};
    int8_t res = send_multi_byte_cmd(hori_addressing, sizeof(hori_addressing));
    if (res)
        LOG(LOG_LEVEL_ERROR, "FAILED TO SET HORIZONTAIL ADDR SCHEME\n");
    uint8_t hori_strt_end_col[] = {SSD1306_CTL_BYTE_CMD, 
                                    CMD_SET_COL_STRT_END_ADDR, 
                                    0x00, 
                                    0x7F};
    res = send_multi_byte_cmd(hori_strt_end_col, sizeof(hori_strt_end_col));
    if (res)
        LOG(LOG_LEVEL_ERROR, "FAILED TO SET COL STRT/END ADDRS\n");
    uint8_t hori_strt_end_pg[] = {SSD1306_CTL_BYTE_CMD, 
                                    CMD_SET_PAGE_STRT_END_ADDR, 
                                    0x00, 
                                    0x07};
    res = send_multi_byte_cmd(hori_strt_end_pg, sizeof(hori_strt_end_pg));
    if (res)
        LOG(LOG_LEVEL_ERROR, "FAILED TO SET PAGE STRT/END ADDRS\n");
}

static int8_t send_single_byte_cmd(uint8_t cmd) {
    int8_t res = I2C_BUSY_IN_TX;
    uint16_t retry = UINT16_MAX;
    uint8_t cmd_pair[] = {SSD1306_CTL_BYTE_CMD, cmd};
    while (retry-- && res == I2C_BUSY_IN_TX)
        res = display.i2c_tx(cmd_pair, SINGLE_BYTE);
    return res;
}

static int8_t send_multi_byte_cmd(uint8_t *cmds, uint8_t len) {
    uint16_t retry = UINT16_MAX;
    int8_t res = I2C_BUSY_IN_TX;
    while (retry-- && res == I2C_BUSY_IN_TX)
        res = display.i2c_tx(cmds, len);
    return res;
}

static int8_t send_single_byte_data(uint8_t *data) {
    int8_t res = I2C_BUSY_IN_TX;
    uint16_t retry = UINT16_MAX;
    uint8_t data_pair[] = {SSD1306_CTL_BYTE_DATA, *data};
    while (retry-- && res == I2C_BUSY_IN_TX)
        res = display.i2c_tx(data_pair, SINGLE_BYTE);
    return res;
}

static int8_t send_multi_byte_data(uint8_t *data, uint32_t len) {
    int8_t res = I2C_BUSY_IN_TX;
    uint16_t retry = UINT16_MAX;
    data_buf[0] = SSD1306_CTL_BYTE_DATA;
    memcpy(&data_buf[1], data, len);
    while (retry-- && res == I2C_BUSY_IN_TX)
        res = display.i2c_tx(data_buf, LEN_DATA_CTL_AND_DATA);
    return res;
}
