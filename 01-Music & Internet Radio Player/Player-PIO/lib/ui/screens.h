#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *arc_brightness;
    lv_obj_t *arc_volume;
    lv_obj_t *button_mode;
    lv_obj_t *bar_status;
    lv_obj_t *label_playbacktime;
    lv_obj_t *container_playback_controlls;
    lv_obj_t *button_previous;
    lv_obj_t *button_playpause;
    lv_obj_t *button_next;
    lv_obj_t *roller_list;
    lv_obj_t *icon_brightness;
    lv_obj_t *icon_brightness_1;
    lv_obj_t *label_time;
    lv_obj_t *label_date;
    lv_obj_t *button_volume;
    lv_obj_t *button_brightness;
    lv_obj_t *image_battery;
    lv_obj_t *bar_battery;
    lv_obj_t *label_battery;
    lv_obj_t *label_wifi;
    lv_obj_t *label_playbackinfo;
    lv_obj_t *image_wifi;
    lv_obj_t *container_brivol_controlls;
    lv_obj_t *arc_brivol_controlls;
    lv_obj_t *label_brivol_controlls;
} objects_t;

extern objects_t objects;

enum ScreensEnum {
    SCREEN_ID_MAIN = 1,
};

void create_screen_main();
void delete_screen_main();
void tick_screen_main();

void create_screen_by_id(enum ScreensEnum screenId);
void delete_screen_by_id(enum ScreensEnum screenId);
void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/