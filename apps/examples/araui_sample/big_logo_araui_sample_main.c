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
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <araui/ui_core.h>
#include <araui/ui_view.h>
#include <araui/ui_widget.h>
#include <araui/ui_commons.h>
#include <tinyara/config.h>

/****************************************************************************
 * araui_main
 ****************************************************************************/

//static ui_widget_t g_canvas;
static int ani_time;
#define DISPLAY_WIDTH 320
#define DISPLAY_HEIGHT 240
#define BUTTON_X ((DISPLAY_WIDTH - 175) / 2)
#define START_X_POSITION 0
#define END_X_POSITION   (DISPLAY_WIDTH - 120)
#define Y_POSITION       6

ui_asset_t g_default_font_20;
ui_asset_t g_digital_7_mono_48;
ui_asset_t g_arial_num_64;
ui_asset_t g_button_bg;
ui_asset_t g_weather_bg;
ui_asset_t g_weather_icon_cloudy;
ui_asset_t g_weather_icon_cloudy_night;
ui_asset_t g_weather_icon_hot;
ui_asset_t g_weather_icon_rainy;
ui_asset_t g_weather_icon_sunny;
ui_asset_t g_weather_icon_thunder;
ui_asset_t g_loading_icon[60];
ui_asset_t g_heartrate_icon;

ui_widget_t g_start_page;
ui_widget_t g_menu_page;
ui_widget_t g_clock_page;
ui_widget_t g_weather_page;
ui_widget_t g_health_page;
ui_widget_t g_animation_page;
ui_widget_t g_transition_page;
ui_widget_t g_transition_fade_page;

ui_view_t g_transit_view;
ui_view_t g_view;
typedef void (*button_touched_callback)(ui_widget_t widget);

/*
static void draw_line_cb(ui_widget_t widget, uint32_t dt)
{
	if (ui_canvas_widget_draw_line(widget, 1, 1, 200, 200, 0xffffff) != UI_OK) {
		printf("error: failed to draw a line.\n");
	}

	if (ui_canvas_widget_draw_circle(widget, 100, 100, 50, 0x800080) != UI_OK) {
		printf("error: failed to draw a circle.\n");
	}	
}
*/
static void on_show_cb(ui_view_t view);
static void on_hide_cb(ui_view_t view);

static void hide_all_pages(void)
{
	ui_widget_set_visible(g_start_page, false);
	ui_widget_set_visible(g_menu_page, false);
	ui_widget_set_visible(g_clock_page, false);
	ui_widget_set_visible(g_weather_page, false);
	ui_widget_set_visible(g_health_page, false);
	ui_widget_set_visible(g_animation_page, false);
	ui_widget_set_visible(g_transition_page, false);
}

void btn_menu_cb(ui_widget_t widget)
{
	hide_all_pages();
	ui_widget_set_visible(g_menu_page, true);
}

static void btn_clock_cb(ui_widget_t widget)
{
	hide_all_pages();
	ui_widget_set_visible(g_clock_page, true);
}

static void btn_weather_cb(ui_widget_t widget)
{
	hide_all_pages();
	ui_widget_set_visible(g_weather_page, true);
}

static void btn_health_cb(ui_widget_t widget)
{
	hide_all_pages();
	ui_widget_set_visible(g_health_page, true);
}

static void btn_animation_cb(ui_widget_t widget)
{
	hide_all_pages();
	ui_widget_set_visible(g_animation_page, true);
	//hide_all_animation_widget();
}

static void btn_transition_cb(ui_widget_t widget)
{
	hide_all_pages();
	ui_widget_set_visible(g_transition_page, true);
}

ui_widget_t create_center_button(const char *text, button_touched_callback cb)
{
	ui_widget_t btn = ui_button_widget_create(175, 60);
	ui_widget_add_child(btn, ui_image_widget_create(g_button_bg), 0, 0);
	ui_widget_t btn_text = ui_text_widget_create(175, 60, g_default_font_20, text);
	ui_text_widget_set_align(btn_text, UI_ALIGN_CENTER | UI_ALIGN_BOTTOM);
	ui_text_widget_set_color(btn_text, 0x808080);
	ui_widget_add_child(btn, btn_text, 0, 0);
	//ui_button_widget_set_touched_callback(btn, cb);

	return btn;
}

static ui_widget_t build_up_start_page(void)
{
	ui_widget_t page = ui_widget_create(DISPLAY_WIDTH, DISPLAY_HEIGHT);

	//text & image
	ui_widget_t title = ui_text_widget_create(DISPLAY_WIDTH, DISPLAY_HEIGHT - 60, g_default_font_20, "UI");
	ui_text_widget_set_align(title, UI_ALIGN_CENTER | UI_ALIGN_MIDDLE);
	ui_widget_add_child(page, title, 0, 0);
	//ui_widget_add_child(page, ui_image_widget_create(g_button_bg), 0, 0);

	//image/text on button
	//ui_widget_add_child(page, create_center_button("START", btn_menu_cb), BUTTON_X, 240 - 60 - 32);

	//ui_widget_add_child(page, create_center_button("IU", btn_clock_cb), BUTTON_X, 50);

	ui_widget_set_visible(page, true);
	
	return page;
}

static ui_widget_t build_up_menu_page(void)
{
	ui_widget_t page = ui_scroll_widget_create(DISPLAY_WIDTH, DISPLAY_HEIGHT);
	ui_scroll_widget_set_content_size(page, DISPLAY_WIDTH, 480);

	ui_widget_t title = ui_text_widget_create(DISPLAY_WIDTH, 20, g_default_font_20, "메인메뉴");
	ui_text_widget_set_align(title, UI_ALIGN_CENTER | UI_ALIGN_MIDDLE);
	ui_widget_add_child(page, title, 0, 16);
	ui_widget_add_child(page, create_center_button("Clock 위젯", btn_clock_cb), BUTTON_X, 50);
	ui_widget_add_child(page, create_center_button("Weather 위젯", btn_weather_cb), BUTTON_X, 130);
	ui_widget_add_child(page, create_center_button("Health 위젯", btn_health_cb), BUTTON_X, 210);
	ui_widget_add_child(page, create_center_button("Animation 테스트", btn_animation_cb), BUTTON_X, 290);
	ui_widget_add_child(page, create_center_button("화면전환 테스트", btn_transition_cb), BUTTON_X, 370);

	ui_widget_set_visible(page, false);

	return page;
}

#define DRAW_BASIC_TEXT_IMAGE 0
#define DRAW_ANIMATION 1
//extern uint8_t utc_font[844];
//extern const uint8_t utc_image[30296];
extern uint8_t samsungone35[3128];
extern uint8_t logo[135736];


#if DRAW_ANIMATION
static void linear_end_cb(ui_widget_t widget);

static void linear_again_cb(ui_widget_t widget)
{
	ui_widget_tween_moveto(widget, 0, 200, ani_time, TWEEN_LINEAR, linear_end_cb);
}

static void linear_end_cb(ui_widget_t widget)
{
	ui_widget_tween_moveto(widget, 0, 6, ani_time, TWEEN_LINEAR, linear_again_cb);
}
#endif
static void view_create_cb(ui_view_t view)
{
/*	g_canvas = ui_canvas_widget_create(CONFIG_UI_DISPLAY_WIDTH, CONFIG_UI_DISPLAY_HEIGHT, draw_line_cb);
	if (ui_view_add_widget(view, g_canvas, 0, 0) != UI_OK) {
		printf("error: failed to add widget to the view.\n");
	}*/

	int32_t i;
	char loading_fn[32];


	//g_default_font_20 = ui_font_asset_create_from_buffer(samsungone35);
	g_button_bg = ui_image_asset_create_from_buffer(logo);

#if DRAW_BASIC_TEXT_IMAGE
/*	g_default_font_20 = ui_font_asset_create_from_file("assets/samsungone20.fnt");
	g_digital_7_mono_48 = ui_font_asset_create_from_file("assets/digital-7_mono_48.fnt");
	g_arial_num_64 = ui_font_asset_create_from_file("assets/arial_num_64.fnt");
	g_button_bg = ui_image_asset_create_from_file("assets/button.png");
	g_weather_bg = ui_image_asset_create_from_file("assets/weather_bg.png");
	g_weather_icon_cloudy = ui_image_asset_create_from_file("assets/weather_icon_cloudy.png");
	g_weather_icon_cloudy_night = ui_image_asset_create_from_file("assets/weather_icon_cloudy_night.png");
	g_weather_icon_hot = ui_image_asset_create_from_file("assets/weather_icon_hot.png");
	g_weather_icon_rainy = ui_image_asset_create_from_file("assets/weather_icon_rainy.png");
	g_weather_icon_sunny = ui_image_asset_create_from_file("assets/weather_icon_sunny.png");
	g_weather_icon_thunder = ui_image_asset_create_from_file("assets/weather_icon_thunder.png");

	for (i = 0; i < 60; i++) {
		snprintf(loading_fn, 32, "assets/loading_%02d.png", i);
		g_loading_icon[i] = ui_image_asset_create_from_file(loading_fn);
	}

	g_heartrate_icon = ui_image_asset_create_from_file("assets/heartrate.png");
*/
	g_start_page = build_up_start_page();
	//g_menu_page = build_up_menu_page();
	//g_clock_page = build_up_clock_page();
	//g_weather_page = build_up_weather_page();
	//g_health_page = build_up_health_page();
	//g_animation_page = build_up_animation_page();
	//g_transition_page = build_up_transition_page();

	ui_view_add_widget(view, g_start_page, 0, 0);
	printf("add start page to view\n");
	//ui_view_add_widget(view, g_menu_page, 0, 0);
	//ui_view_add_widget(view, g_clock_page, 0, 0);
	//ui_view_add_widget(view, g_weather_page, 0, 0);
	//ui_view_add_widget(view, g_health_page, 0, 0);
	//ui_view_add_widget(view, g_animation_page, 0, 0);
	//ui_view_add_widget(view, g_transition_page, 0, 0);
#endif

#if DRAW_ANIMATION
	ui_widget_t container = ui_widget_create(DISPLAY_WIDTH, DISPLAY_HEIGHT);
	ui_widget_t linear_image = ui_image_widget_create(g_button_bg);
	ui_widget_add_child(container, linear_image, START_X_POSITION, Y_POSITION);
	ui_view_add_widget(view, container, 0, 0);
	ui_widget_set_visible(container, true);
	printf("Now start new animation!!\n");
	ui_widget_tween_moveto(linear_image, 0, 200, ani_time, TWEEN_LINEAR, linear_end_cb);
	return;

/*
	ui_widget_t container = ui_widget_create(DISPLAY_WIDTH, DISPLAY_HEIGHT);
	g_animation_page = ui_scroll_widget_create(DISPLAY_WIDTH, DISPLAY_HEIGHT);
	ui_scroll_widget_set_content_size(g_animation_page, DISPLAY_WIDTH, DISPLAY_HEIGHT);
	//ui_widget_add_child(container, g_animation_page, 0, 0);

	ui_widget_t linear_widget = ui_widget_create(DISPLAY_WIDTH, DISPLAY_HEIGHT);
//	ui_widget_t linear_title = ui_text_widget_create(160, 60, g_default_font_20, "123");
//	ui_text_widget_set_align(linear_title, UI_ALIGN_CENTER | UI_ALIGN_MIDDLE);
//	ui_widget_add_child(linear_widget, linear_title, 0, 16);
	ui_widget_t linear_image = ui_image_widget_create(g_button_bg);
	ui_widget_add_child(linear_widget, linear_image, START_X_POSITION, Y_POSITION);
	//ui_widget_add_child(linear_widget, create_center_button("UU", btn_clock_cb), BUTTON_X, 240 - 60 - 32);
	ui_widget_add_child(container, linear_widget, 0, 0);

	ui_widget_set_visible(container, false);
	ui_view_add_widget(view, container, 0, 0);

	//sleep(3);
	
	ui_widget_set_visible(container, true);
	//sleep(3);
	//ui_widget_set_visible(g_animation_page, false);
	//ui_widget_set_visible(linear_widget, false);
	ui_widget_set_visible(linear_widget, true);
	printf("Now start animation!!\n");
	ui_widget_tween_moveto(linear_image, 0, 200, ani_time, TWEEN_LINEAR, linear_end_cb);
	//ui_widget_tween_moveto(linear_title, 100, 16, ani_time, TWEEN_LINEAR, linear_end_cb);*/
#endif
}

static void view_destroy_cb(ui_view_t view)
{
/*	if (ui_widget_destroy(g_canvas) != UI_OK) {
		printf("error: failed to destroy the widget.\n");
	}

	if (ui_view_destroy(view) != UI_OK) {
		printf("error: failed to destroy the view.\n");
	}*/
	int32_t i;

	ui_font_asset_destroy(g_default_font_20);
	ui_font_asset_destroy(g_digital_7_mono_48);
	ui_font_asset_destroy(g_arial_num_64);
	ui_image_asset_destroy(g_button_bg);
	ui_image_asset_destroy(g_weather_icon_cloudy);
	ui_image_asset_destroy(g_weather_icon_cloudy_night);
	ui_image_asset_destroy(g_weather_icon_hot);
	ui_image_asset_destroy(g_weather_icon_rainy);
	ui_image_asset_destroy(g_weather_icon_sunny);
	ui_image_asset_destroy(g_weather_icon_thunder);

	for (i = 0; i < 60; i++) {
		ui_image_asset_destroy(g_loading_icon[i]);
	}

	ui_image_asset_destroy(g_heartrate_icon);
}

static void view_show_cb(ui_view_t view)
{
}

static void view_hide_cb(ui_view_t view)
{
}

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int araui_sample_main(int argc, char *argv[])
#endif
{
printf("araui_sample -> %s\n", __func__);
	ani_time = (argc > 1) ? atoi(argv[1]) : 1000;
	
	ui_view_t view;

	ui_start();

	//view = ui_view_create(view_create_cb, view_destroy_cb, view_show_cb, view_hide_cb);
	g_view = ui_view_create(view_create_cb, view_destroy_cb, view_show_cb, view_hide_cb);
	if (!g_view) {
		printf("error: failed to create view.\n");
	}

	//ui_view_destroy(g_view);
//	ui_stop();

	return 0;
}
