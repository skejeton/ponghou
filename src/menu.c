#include "menu.h"
#include "button.h"
#include "font.h"
#include "game.h"
#include "gameplay.h"
#include "wminterop.h"

#define GAME_VERSION "0.2.1"
#define BOTTOM_TEXT "BY ISHIDEX2\nv" GAME_VERSION

struct Game_Menu_State game_menu_reconstruct() {
    return (struct Game_Menu_State) { .logo = { 0.0 }, .buttons = { {"Play with your buddy", 0.0, 0, 0, 0, 0, false }, {"Play versus AI", 0.0, 0, 0, 0, 0, false }, {"AI versus AI", 0.0, 0, 0, 0, 0, false }, {"Controls", 0.0, 0, 0, 0, 0, false } } };
}



static void render_version_and_name(struct Ponghou_Font *font, struct Wmi_Struct *wmi, f64 pulse) {
    const u8 COLORS[][4] = { { 0, 0, 255, 255 }, { 0, 255, 0, 255 }, { 255, 0, 0, 255 }, { 255, 255, 255, 255 } }; 
    
    usize ci = (usize)(pulse * 1) % 4;

    wmi_set_color(wmi, COLORS[ci][0], COLORS[ci][1], COLORS[ci][2], COLORS[ci][3]);
    struct Wmi_Size wsize = wmi_get_window_size(wmi);
    struct Wmi_Size text_size =  font_measure_text(font, BOTTOM_TEXT, 4);
    font_render_text(font, wmi, BOTTOM_TEXT, 0, wsize.y-text_size.y, 4);
}
void game_render_menu(struct Game_Menu_State *self, struct Ponghou_Font *font, struct Wmi_Struct *wmi, f64 pulse, enum Game_State *state, struct Gameplay_State *gameplay) {
    struct Wmi_Size wsize = wmi_get_window_size(wmi);
    render_version_and_name(font, wmi, pulse);

    if (self->buttons[0].was_pressed) {
        self->buttons[0].was_pressed = false;
        *gameplay = gameplay_reset(PLAYER_1, PLAYER_2);
        *state = GAME_GAMEPLAY;
    }
    if (self->buttons[1].was_pressed) {
        self->buttons[1].was_pressed = false;
        *gameplay = gameplay_reset(PLAYER_1, PLAYER_AI);
        *state = GAME_GAMEPLAY;
    }
    if (self->buttons[2].was_pressed) {
        self->buttons[2].was_pressed = false;
        *gameplay = gameplay_reset(PLAYER_AI, PLAYER_AI);
        *state = GAME_GAMEPLAY;
    }
    else if (self->buttons[3].was_pressed) {
        self->buttons[3].was_pressed = false;
        *state = GAME_CONTROLS;
    }
    logo_paint_and_update(&self->logo, wmi);

    for (int i = 0; i < 4; i += 1) {
        int x = wsize.x-600;
        int y = (wsize.y/6)*i+((wsize.y/2)-((wsize.y/6)*2+75)/2);
        int w = 600;
        int h = 75;
        self->buttons[i].x = x;
        self->buttons[i].y = y;
        self->buttons[i].w = w;
        self->buttons[i].h = h;
        game_handle_button(&self->buttons[i], wmi, font);
    }
}
