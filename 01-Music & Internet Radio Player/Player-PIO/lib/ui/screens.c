#include <string.h>

#include "screens.h"
#include "images.h"
#include "fonts.h"
#include "actions.h"
#include "vars.h"
#include "styles.h"
#include "ui.h"

#include <string.h>

objects_t objects;
lv_obj_t *tick_value_change_obj;
uint32_t active_theme_index = 0;

void create_screen_main() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.main = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 360, 360);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // arc brightness
            lv_obj_t *obj = lv_arc_create(parent_obj);
            objects.arc_brightness = obj;
            lv_obj_set_pos(obj, 10, 10);
            lv_obj_set_size(obj, 340, 340);
            lv_arc_set_value(obj, 50);
            lv_arc_set_bg_start_angle(obj, 140);
            lv_arc_set_bg_end_angle(obj, 225);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);
            add_style_arc_controlls(obj);
        }
        {
            // arc volume
            lv_obj_t *obj = lv_arc_create(parent_obj);
            objects.arc_volume = obj;
            lv_obj_set_pos(obj, 10, 10);
            lv_obj_set_size(obj, 340, 340);
            lv_arc_set_value(obj, 50);
            lv_arc_set_bg_start_angle(obj, 315);
            lv_arc_set_bg_end_angle(obj, 400);
            lv_arc_set_mode(obj, LV_ARC_MODE_REVERSE);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);
            add_style_arc_controlls(obj);
        }
        {
            // button mode
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.button_mode = obj;
            lv_obj_set_pos(obj, 85, -2);
            lv_obj_set_size(obj, 191, 60);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_CHECKABLE);
            lv_obj_set_style_bg_img_src(obj, &img_icon_radio, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xfff8473c), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_src(obj, &img_icon_music, LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_bg_img_opa(obj, 255, LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_opa(obj, 100, LV_PART_MAIN | LV_STATE_PRESSED);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_PRESSED);
            lv_obj_set_style_bg_img_opa(obj, 0, LV_PART_MAIN | LV_STATE_PRESSED);
        }
        {
            // bar status
            lv_obj_t *obj = lv_bar_create(parent_obj);
            objects.bar_status = obj;
            lv_obj_set_pos(obj, 65, 190);
            lv_obj_set_size(obj, 230, 8);
            lv_bar_set_value(obj, 25, LV_ANIM_OFF);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff2bddce), LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff202020), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // label playbacktime
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_playbacktime = obj;
            lv_obj_set_pos(obj, 217, 201);
            lv_obj_set_size(obj, 78, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "--:--");
        }
        {
            // container playback controlls
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.container_playback_controlls = obj;
            lv_obj_set_pos(obj, 66, 216);
            lv_obj_set_size(obj, 228, 68);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_layout(obj, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_flex_flow(obj, LV_FLEX_FLOW_ROW, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_flex_main_place(obj, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_flex_cross_place(obj, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_flex_track_place(obj, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // button previous
                    lv_obj_t *obj = lv_btn_create(parent_obj);
                    objects.button_previous = obj;
                    lv_obj_set_pos(obj, 100, 75);
                    lv_obj_set_size(obj, 54, 54);
                    lv_obj_set_style_bg_img_src(obj, &img_icon_previous, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_img_src(obj, &img_icon_previous, LV_PART_MAIN | LV_STATE_PRESSED);
                    lv_obj_set_style_bg_img_opa(obj, 100, LV_PART_MAIN | LV_STATE_PRESSED);
                }
                {
                    // button playpause
                    lv_obj_t *obj = lv_btn_create(parent_obj);
                    objects.button_playpause = obj;
                    lv_obj_set_pos(obj, 100, 75);
                    lv_obj_set_size(obj, 54, 54);
                    lv_obj_add_flag(obj, LV_OBJ_FLAG_CHECKABLE);
                    lv_obj_set_style_bg_img_src(obj, &img_icon_play, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_img_src(obj, &img_icon_pause, LV_PART_MAIN | LV_STATE_CHECKED);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_PRESSED);
                    lv_obj_set_style_opa(obj, 100, LV_PART_MAIN | LV_STATE_PRESSED);
                    lv_obj_set_style_bg_img_opa(obj, 0, LV_PART_MAIN | LV_STATE_PRESSED);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_CHECKED | LV_STATE_PRESSED);
                }
                {
                    // button next
                    lv_obj_t *obj = lv_btn_create(parent_obj);
                    objects.button_next = obj;
                    lv_obj_set_pos(obj, 100, 75);
                    lv_obj_set_size(obj, 54, 54);
                    lv_obj_set_style_bg_img_src(obj, &img_icon_next, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_img_src(obj, &img_icon_next, LV_PART_MAIN | LV_STATE_PRESSED);
                    lv_obj_set_style_bg_img_opa(obj, 100, LV_PART_MAIN | LV_STATE_PRESSED);
                }
            }
        }
        {
            // roller list
            lv_obj_t *obj = lv_roller_create(parent_obj);
            objects.roller_list = obj;
            lv_obj_set_pos(obj, 66, 284);
            lv_obj_set_size(obj, 229, 64);
            lv_roller_set_options(obj, "-\n-", LV_ROLLER_MODE_NORMAL);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_line_space(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 150, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_SELECTED | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, LV_PART_SELECTED | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_SELECTED | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xff2addce), LV_PART_SELECTED | LV_STATE_DEFAULT);
            lv_obj_set_style_text_decor(obj, LV_TEXT_DECOR_NONE, LV_PART_SELECTED | LV_STATE_DEFAULT);
        }
        {
            // icon brightness
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.icon_brightness = obj;
            lv_obj_set_pos(obj, 57, 291);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_icon_brightness);
            lv_obj_set_style_img_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_img_recolor(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_img_recolor_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // icon brightness_1
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.icon_brightness_1 = obj;
            lv_obj_set_pos(obj, 283, 291);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_icon_volume);
            lv_obj_set_style_img_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_img_recolor(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_img_recolor_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // label time
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_time = obj;
            lv_obj_set_pos(obj, 78, 78);
            lv_obj_set_size(obj, 205, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_k2_75, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "-");
        }
        {
            // label date
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_date = obj;
            lv_obj_set_pos(obj, 44, 150);
            lv_obj_set_size(obj, 272, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_k2_25, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "-");
        }
        {
            // button volume
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.button_volume = obj;
            lv_obj_set_pos(obj, 303, 51);
            lv_obj_set_size(obj, 70, 260);
            lv_obj_set_style_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // button brightness
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.button_brightness = obj;
            lv_obj_set_pos(obj, -13, 58);
            lv_obj_set_size(obj, 70, 233);
            lv_obj_set_style_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // image battery
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.image_battery = obj;
            lv_obj_set_pos(obj, 233, 41);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_battery);
        }
        {
            // bar battery
            lv_obj_t *obj = lv_bar_create(parent_obj);
            objects.bar_battery = obj;
            lv_obj_set_pos(obj, 236, 45);
            lv_obj_set_size(obj, 5, 12);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffffff), LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_INDICATOR | LV_STATE_DEFAULT);
        }
        {
            // label battery
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_battery = obj;
            lv_obj_set_pos(obj, 250, 41);
            lv_obj_set_size(obj, 47, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "-");
        }
        {
            // label wifi
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_wifi = obj;
            lv_obj_set_pos(obj, 63, 41);
            lv_obj_set_size(obj, 46, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "-");
        }
        {
            // label playbackinfo
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_playbackinfo = obj;
            lv_obj_set_pos(obj, 66, 201);
            lv_obj_set_size(obj, 141, LV_SIZE_CONTENT);
            lv_label_set_long_mode(obj, LV_LABEL_LONG_SCROLL_CIRCULAR);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "-");
        }
        {
            // image wifi
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.image_wifi = obj;
            lv_obj_set_pos(obj, 112, 39);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_wifi_00);
        }
        {
            // container brivol controlls
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.container_brivol_controlls = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, 360, 360);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_CHECKABLE);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 230, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_translate_x(obj, -360, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_translate_x(obj, 0, LV_PART_MAIN | LV_STATE_CHECKED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // arc brivol controlls
                    lv_obj_t *obj = lv_arc_create(parent_obj);
                    objects.arc_brivol_controlls = obj;
                    lv_obj_set_pos(obj, 30, 30);
                    lv_obj_set_size(obj, 300, 300);
                    lv_arc_set_value(obj, 25);
                    lv_obj_set_style_border_width(obj, 5, LV_PART_KNOB | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_color(obj, lv_color_hex(0xff000000), LV_PART_KNOB | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xffecf322), LV_PART_KNOB | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 10, LV_PART_KNOB | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 10, LV_PART_KNOB | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 10, LV_PART_KNOB | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 10, LV_PART_KNOB | LV_STATE_DEFAULT);
                    lv_obj_set_style_arc_color(obj, lv_color_hex(0xffedf321), LV_PART_INDICATOR | LV_STATE_DEFAULT);
                }
                {
                    // label brivol controlls
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.label_brivol_controlls = obj;
                    lv_obj_set_pos(obj, 103, 154);
                    lv_obj_set_size(obj, 155, LV_SIZE_CONTENT);
                    lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &ui_font_k2_25, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "Volume\n10%");
                }
            }
        }
    }
    
    tick_screen_main();
}

void delete_screen_main() {
    lv_obj_del(objects.main);
    objects.main = 0;
    objects.arc_brightness = 0;
    objects.arc_volume = 0;
    objects.button_mode = 0;
    objects.bar_status = 0;
    objects.label_playbacktime = 0;
    objects.container_playback_controlls = 0;
    objects.button_previous = 0;
    objects.button_playpause = 0;
    objects.button_next = 0;
    objects.roller_list = 0;
    objects.icon_brightness = 0;
    objects.icon_brightness_1 = 0;
    objects.label_time = 0;
    objects.label_date = 0;
    objects.button_volume = 0;
    objects.button_brightness = 0;
    objects.image_battery = 0;
    objects.bar_battery = 0;
    objects.label_battery = 0;
    objects.label_wifi = 0;
    objects.label_playbackinfo = 0;
    objects.image_wifi = 0;
    objects.container_brivol_controlls = 0;
    objects.arc_brivol_controlls = 0;
    objects.label_brivol_controlls = 0;
}

void tick_screen_main() {
}

typedef void (*create_screen_func_t)();
create_screen_func_t create_screen_funcs[] = {
    create_screen_main,
};
void create_screen(int screen_index) {
    create_screen_funcs[screen_index]();
}
void create_screen_by_id(enum ScreensEnum screenId) {
    create_screen_funcs[screenId - 1]();
}

typedef void (*delete_screen_func_t)();
delete_screen_func_t delete_screen_funcs[] = {
    delete_screen_main,
};
void delete_screen(int screen_index) {
    delete_screen_funcs[screen_index]();
}
void delete_screen_by_id(enum ScreensEnum screenId) {
    delete_screen_funcs[screenId - 1]();
}

typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_main,
};
void tick_screen(int screen_index) {
    tick_screen_funcs[screen_index]();
}
void tick_screen_by_id(enum ScreensEnum screenId) {
    tick_screen_funcs[screenId - 1]();
}

void create_screens() {
    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), true, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    
    create_screen_main();
}
