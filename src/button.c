#include "button.h"

void game_handle_button(struct Game_Button *self, struct Wmi_Struct *wmi, struct Ponghou_Font *font) {

    int x = self->x;
    int y = self->y;
    int w = self->w;
    int h = self->h;
    struct Wmi_Point m = wmi->mouse_pos;
    if (m.x > x && m.y > y && m.x < (x+w) && m.y < (y+h)) {
        if (wmi_is_lbutton_just_down(wmi))
            self->was_pressed = true;
        self->offset *= 1.5;          
    }
    else {
        self->offset /= 1.5;
    }
    if (self->offset > 50) self->offset = 50;
    if (self->offset < 0.1) self->offset = 0.1;

    wmi_set_color(wmi, 0, 0, 64, 255);
    wmi_draw_rect(wmi, x-(int)self->offset*4-12, y, w+(int)self->offset, h);
    wmi_set_color(wmi, 0, 128, 0, 255);
    wmi_draw_rect(wmi, x-(int)self->offset*3-6, y, w+(int)self->offset, h);
    wmi_set_color(wmi, 255, 0, 0, 255);
    wmi_draw_rect(wmi, x-(int)self->offset*2, y, w+(int)self->offset, h);
    wmi_set_color(wmi, 255, 255, 255, 255);
    wmi_draw_rect(wmi, x-(int)self->offset, y, w+(int)self->offset, h);
    wmi_set_color(wmi, 64, 64, 64, 255);
    font_render_text(font, wmi, self->text, x+6-(int)self->offset, y+6, 6);
}
