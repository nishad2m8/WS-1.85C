#ifndef EEZ_LVGL_UI_IMAGES_H
#define EEZ_LVGL_UI_IMAGES_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const lv_img_dsc_t img_icon_brightness;
extern const lv_img_dsc_t img_icon_next;
extern const lv_img_dsc_t img_icon_pause;
extern const lv_img_dsc_t img_icon_play;
extern const lv_img_dsc_t img_icon_previous;
extern const lv_img_dsc_t img_icon_volume;
extern const lv_img_dsc_t img_icon_music;
extern const lv_img_dsc_t img_icon_radio;
extern const lv_img_dsc_t img_battery;
extern const lv_img_dsc_t img_wifi_00;
extern const lv_img_dsc_t img_wifi_01;
extern const lv_img_dsc_t img_wifi_02;
extern const lv_img_dsc_t img_wifi_03;
extern const lv_img_dsc_t img_wifi_04;
extern const lv_img_dsc_t img_wifi_05;

#ifndef EXT_IMG_DESC_T
#define EXT_IMG_DESC_T
typedef struct _ext_img_desc_t {
    const char *name;
    const lv_img_dsc_t *img_dsc;
} ext_img_desc_t;
#endif

extern const ext_img_desc_t images[15];


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_IMAGES_H*/