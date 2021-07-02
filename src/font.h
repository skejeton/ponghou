#pragma once
#include "wminterop.h"

struct Ponghou_Font {
    struct Wmi_Bitmap bitmap;
};

void font_render_text(const struct Ponghou_Font *self, struct Wmi_Struct *wmi, const char *str, int x, int y, int scale);

struct Wmi_Size font_measure_text(const struct Ponghou_Font *self, const char *str, int scale);
