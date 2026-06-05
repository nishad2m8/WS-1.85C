#ifndef EEZ_LVGL_UI_STYLES_H
#define EEZ_LVGL_UI_STYLES_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Style: arc controlls
lv_style_t *get_style_arc_controlls_MAIN_DEFAULT();
lv_style_t *get_style_arc_controlls_INDICATOR_DEFAULT();
lv_style_t *get_style_arc_controlls_KNOB_DEFAULT();
void add_style_arc_controlls(lv_obj_t *obj);
void remove_style_arc_controlls(lv_obj_t *obj);



#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_STYLES_H*/