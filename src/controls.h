#pragma once
#include "font.h"
#include "button.h"
#include "game_state.h"
#include "wminterop.h"

struct Game_Controls {
    Wmi_Key leftup;
    Wmi_Key leftdown;
    Wmi_Key rightup;
    Wmi_Key rightdown;
    int reassigning;
    struct Game_Button _backbtn;
};

struct Game_Controls game_controls_default();
void game_controls_handle(struct Game_Controls *self, enum Game_State *state, struct Wmi_Struct *wmi, struct Ponghou_Font *font);
