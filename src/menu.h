/**
 * Game menu
 * ishidex2 2021, MIT license
 */
#pragma once
#include "gameplay.h"
#include "logo.h"
#include "font.h"
#include "wminterop.h"
#include "button.h"
#include "game_state.h"

struct Game_Menu_State {
    struct LogoState logo;
    struct Game_Button buttons[4];
};

struct Game_Menu_State game_menu_reconstruct();
void game_render_menu(struct Game_Menu_State *self, struct Ponghou_Font *font, struct Wmi_Struct *wmi, f64 pulse, enum Game_State *state, struct Gameplay_State *gameplay);
