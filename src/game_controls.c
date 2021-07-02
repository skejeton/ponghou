#include "button.h"
#include "controls.h"
#include "font.h"
#include "game_state.h"
#include "math.h"
#include "wminterop.h"

struct Game_Controls game_controls_default() {
    return (struct Game_Controls) {
        .leftup = 'W',
        .leftdown = 'S',
        .rightup = VK_UP,
        .rightdown = VK_DOWN,
        .reassigning = -1,
        ._backbtn = (struct Game_Button) {
            .text = "Back",
            .offset = 0.0,
            .x = 0,
            .y = 0,
            .w = 0,
            .h = 0,
            .was_pressed = false
        }
    };
}

void map_vkey(Wmi_Key key, char *res) {
    if ((key >= 'A' && key <= 'Z') || (key >= '0' && key <= '9')) {
        res[0] = (char)key; 
        res[1] = 0;
    }
    else if (key == VK_UP) strcpy(res, "Up");
    else if (key == VK_DOWN) strcpy(res, "Down");
    else if (key == VK_LEFT) strcpy(res, "Left");
    else if (key == VK_RIGHT) strcpy(res, "Right");
    else if (key == VK_RETURN) strcpy(res, "Enter");
    else if (key == VK_SPACE) strcpy(res, "Space");
    else if (key == VK_TAB) strcpy(res, "Tab");
    else strcpy(res, "Name unknown");
}

void game_controls_handle(struct Game_Controls *self, enum Game_State *state, struct Wmi_Struct *wmi, struct Ponghou_Font *font) {
    struct Wmi_Size wsize = wmi_get_window_size(wmi);
    (void) state;
    (void) font;
    (void) self;
    int jy[] = { wsize.y/2-64-6, wsize.y/2+6 };
    Wmi_Key *keys[2][2] = { { &self->leftup, &self->leftdown }, { &self->rightup, &self->rightdown } };
    wmi_set_color(wmi, 255, 255, 0, 255);
    font_render_text(font, wmi, "Player 1 controls", wsize.x/2-font_measure_text(font, "Player 1 controls", 5).x-40, 30, 5);
    wmi_set_color(wmi, 128, 128, 128, 255);
    font_render_text(font, wmi, "Press to reassign", wsize.x/2-font_measure_text(font, "Press to reassign", 3).x-40, 100, 3);
    wmi_set_color(wmi, 0, 255, 255, 255);
    font_render_text(font, wmi, "Player 2 controls", wsize.x/2+40, 30, 5);
    wmi_set_color(wmi, 128, 128, 128, 255);
    font_render_text(font, wmi, "Press to reassign", wsize.x/2+40, 100, 3);
    wmi_set_color(wmi, 255, 255, 255, 255);
    wmi_draw_rect(wmi, wsize.x/2-3, 0, 5, wsize.y);
    for (int i = 0; i < 2; i += 1)
        for (int j = 0; j < 2; j += 1) {
            struct Game_Rect rect = {
                .x = (i+1)*+wsize.x/3-32,
                .y = jy[j],
                .w = 64,
                .h = 64,
            };
            bool reassigned = (i + j * 2) == self->reassigning;
            struct Wmi_Point m = wmi->mouse_pos;
            char str[1024] = { 0 };

            // this is terrible, a direct winapi call, but the time is shrinking!!
            //int r = GetKeyNameTextA(((LONG)MapVirtualKey(*keys[i][j], 0)<<16), str, 1024);
            //if (r == 0) str[0] = '?';
            map_vkey(*keys[i][j], str);
            char *sp = str;
            if (strncmp(str, "KP_", 3) == 0) sp += 3;
            struct Wmi_Size fsize = font_measure_text(font, sp, 4);
            rect.w = rect.w > fsize.x ? rect.w : fsize.x;
 //           rect.x -= fsize.x/2+16;
            rect.w += 12;

            bool overmouse = m.x > rect.x && m.y > rect.y && m.x < (rect.x+rect.w) && m.y < (rect.y+rect.h);
            if (overmouse && wmi_is_lbutton_just_down(wmi))
                self->reassigning = i + j * 2;
            if (wmi->_vendor.lastkey == VK_ESCAPE)
                self->reassigning = -1;
            else if (reassigned && wmi->_vendor.lastkey != 0xFFFF) {
                self->reassigning = -1;
                *keys[i][j] = wmi->_vendor.lastkey;
            }

            if (reassigned)
                 wmi_set_color(wmi, 255, 0, 0, 255);
            else
                if (overmouse)
                    wmi_set_color(wmi, 255*(i==0), 255, 255*(i==1), 255);
                else
                    wmi_set_color(wmi, 255, 255, 255, 255);

            wmi_draw_rect(wmi, rect.x, rect.y, rect.w, rect.h+8);
            wmi_set_color(wmi, 0, 0, 0, 255);
            wmi_draw_rect(wmi, rect.x+4, rect.y+4, rect.w-8, rect.h-8);
            if (reassigned)
                 wmi_set_color(wmi, 255, 0, 0, 255);
            else
                if (overmouse)
                    wmi_set_color(wmi, 255*(i==0), 255, 255*(i==1), 255);
                else
                    wmi_set_color(wmi, 255, 255, 255, 255);

            font_render_text(font, wmi, sp, rect.x+8, rect.y+8, 4);
        }  

    struct Game_Rect btn_rect = {
        .x = wsize.x-300,
        .y = (wsize.y*2)/3,
        .w = 300,
        .h = 75
    };
    self->_backbtn.x = btn_rect.x;
    self->_backbtn.y = btn_rect.y;
    self->_backbtn.w = btn_rect.w;
    self->_backbtn.h = btn_rect.h;
    game_handle_button(&self->_backbtn, wmi, font);
    if (self->_backbtn.was_pressed) {
        self->_backbtn.was_pressed = false;
        *state = GAME_MENU;
    }
}
