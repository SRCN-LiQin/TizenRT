/****************************************************************************
 *
 * Copyright 2019 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <tinyara/config.h>

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <debug.h>
#include <assert.h>

#include <tinyara/board.h>
#include <tinyara/lcd/lcd.h>
#include <tinyara/lcd/ili9341.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Configuration ************************************************************/
/* Check contrast selection */

#if !defined(CONFIG_LCD_MAXCONTRAST)
#  define CONFIG_LCD_MAXCONTRAST 100
#endif

/* Check power setting */

#if !defined(CONFIG_LCD_MAXPOWER)
#  define CONFIG_LCD_MAXPOWER 100
#endif

/* Simulated LCD geometry and color format */

#ifndef CONFIG_ESP32_FBWIDTH
#  define CONFIG_ESP32_FBWIDTH  320 /* Framebuffer width in pixels */
#endif

#ifndef CONFIG_ESP32_FBHEIGHT
#  define CONFIG_ESP32_FBHEIGHT 240 /* Framebuffer height in pixels */
#endif

#ifndef CONFIG_ESP32_FBBPP
#  define CONFIG_ESP32_FBBPP    16  /* Framebuffer bytes per pixel (RGB) */
#endif

#define FB_STRIDE ((CONFIG_ESP32_FBBPP * CONFIG_ESP32_FBWIDTH + 7) >> 3)

#undef FB_FMT
#if CONFIG_ESP32_FBBPP == 1
#  define FB_FMT FB_FMT_RGB1
#elif CONFIG_ESP32_FBBPP == 4
#  define FB_FMT FB_FMT_RGB4
#elif CONFIG_ESP32_FBBPP == 8
#  define FB_FMT FB_FMT_RGB8
#elif CONFIG_ESP32_FBBPP == 16
#  define FB_FMT FB_FMT_RGB16_565
#elif CONFIG_ESP32_FBBPP == 24
#  define FB_FMT FB_FMT_RGB24
#elif CONFIG_ESP32_FBBPP == 32
#  define FB_FMT FB_FMT_RGB32
#else
#  error "Unsupported BPP"
#endif

/****************************************************************************
 * Private Type Definition
 ****************************************************************************/

static void esp32_ili9341_select(FAR struct ili9341_lcd_s *lcd);
static void esp32_ili9341_deselect(FAR struct ili9341_lcd_s *lcd);
static int esp32_ili9341_sendcmd(FAR struct ili9341_lcd_s *lcd, const uint8_t cmd);
static int esp32_ili9341_sendparam(FAR struct ili9341_lcd_s *lcd, const uint8_t param);
static int esp32_ili9341_recvparam(FAR struct ili9341_lcd_s *lcd, uint8_t *param);
static int esp32_ili9341_recvgram(FAR struct ili9341_lcd_s *lcd, uint16_t *wd, uint32_t nwords);
static int esp32_ili9341_sendgram(FAR struct ili9341_lcd_s *lcd, const uint16_t *wd, uint32_t nwords);
static int esp32_ili9341_backlight(FAR struct ili9341_lcd_s *lcd, int level);

FAR struct ili9341_lcd_s g_lcd = {
	.select = esp32_ili9341_select,
	.deselect = esp32_ili9341_deselect,
	.sendcmd = esp32_ili9341_sendcmd,
	.sendparam = esp32_ili9341_sendparam,
	.recvparam = esp32_ili9341_recvparam,
	.recvgram = esp32_ili9341_recvgram,
	.sendgram = esp32_ili9341_sendgram,
	.backlight = esp32_ili9341_backlight
};

FAR struct lcd_dev_s *g_lcddev = NULL;

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static void esp32_ili9341_select(FAR struct ili9341_lcd_s *lcd)
{
}

static void esp32_ili9341_deselect(FAR struct ili9341_lcd_s *lcd)
{
}

static int esp32_ili9341_sendcmd(FAR struct ili9341_lcd_s *lcd, const uint8_t cmd)
{
	return 0;
}

static int esp32_ili9341_sendparam(FAR struct ili9341_lcd_s *lcd, const uint8_t param)
{
	return 0;
}

static int esp32_ili9341_recvparam(FAR struct ili9341_lcd_s *lcd, uint8_t *param)
{
	return 0;
}

static int esp32_ili9341_recvgram(FAR struct ili9341_lcd_s *lcd, uint16_t *wd, uint32_t nwords)
{
	return nwords;
}

static int esp32_ili9341_sendgram(FAR struct ili9341_lcd_s *lcd, const uint16_t *wd, uint32_t nwords)
{
	return nwords;
}

static int esp32_ili9341_backlight(FAR struct ili9341_lcd_s *lcd, int level)
{
	return 0;
}


/****************************************************************************
 * Public Functions
 ****************************************************************************/

FAR struct ili9341_lcd_s *esp32_ili9341_initialize(void)
{
	return &g_lcd;
}

/****************************************************************************
 * Name:  board_lcd_initialize
 *
 * Description:
 *   Initialize the LCD video hardware.  The initial state of the LCD is
 *   fully initialized, display memory cleared, and the LCD ready to use,
 *   but with the power setting at 0 (full off).
 *
 ****************************************************************************/

int board_lcd_initialize(void)
{
	g_lcddev = ili9341_initialize(esp32_ili9341_initialize(), 0);

	return OK;
}

/****************************************************************************
 * Name:  board_lcd_getdev
 *
 * Description:
 *   Return a a reference to the LCD object for the specified LCD.  This
 *   allows support for multiple LCD devices.
 *
 ****************************************************************************/

FAR struct lcd_dev_s *board_lcd_getdev(int lcddev)
{
	DEBUGASSERT(lcddev == 0);

	return g_lcddev;
}

/****************************************************************************
 * Name:  board_lcd_uninitialize
 *
 * Description:
 *   Unitialize the LCD support
 *
 ****************************************************************************/

void board_lcd_uninitialize(void)
{

}
