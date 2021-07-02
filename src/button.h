#pragma once
#include "../deps/common/types.h"
#include "font.h"
#include "wminterop.h"

struct Game_Button {
    const char *text;
    f64 offset;
    int x;
    int y;
    int w;
    int h;
    bool was_pressed;
};

void game_handle_button(struct Game_Button *self, struct Wmi_Struct *wmi, struct Ponghou_Font *font);
