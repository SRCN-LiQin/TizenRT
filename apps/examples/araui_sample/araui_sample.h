#ifndef __ARAUI_SAMPLE_H__
#define __ARAUI_SAMPLE_H__

#include <araui/ui_view.h>
#include <araui/ui_widget.h>
#include <araui/ui_asset.h>

extern ui_asset_t g_default_font_20;
extern ui_asset_t g_digital_7_mono_48;
extern ui_asset_t g_arial_num_64;
extern ui_asset_t g_weather_bg;
extern ui_asset_t g_weather_icon_cloudy;
extern ui_asset_t g_weather_icon_cloudy_night;
extern ui_asset_t g_weather_icon_hot;
extern ui_asset_t g_weather_icon_rainy;
extern ui_asset_t g_weather_icon_sunny;
extern ui_asset_t g_weather_icon_thunder;

extern ui_widget_t g_clock_page;
extern ui_widget_t g_weather_page;

extern ui_view_t g_view;

ui_widget_t build_up_clock_page(void);
ui_widget_t build_up_weather_page(void);

#endif // __ARAUI_SAMPLE_H__
