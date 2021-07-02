/**
 * Game state
 * ishidex2 2021, MIT license
 */
#pragma once
#include "menu.h"
#include "controls.h"
#include "wminterop.h"
#include "logo.h"
#include "gameplay.h"
#include "font.h"


struct Game_Struct {
    enum Game_State state;
    struct Game_Menu_State menu_state;
    struct Gameplay_State gameplay;
    struct Ponghou_Font font;
    struct Game_Controls controls_state;
    f64 pulse;
};

void game_start(struct Wmi_Struct *wmi);
void game_loop(struct Wmi_Struct *wmi);


