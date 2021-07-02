/**
 * Windowing & Multimedia interop
 * ishidex2 2021, MIT license
 */
#pragma once
#include "../deps/common/types.h"
#include <stdbool.h>
#include <windows.h>
#include <gdiplus.h>

typedef WPARAM Wmi_Key;

struct Wmi_Bitmap {
    u32 *buffer;    
    usize w;
    usize h;
};

struct Wmi_Size {
    int x;
    int y;
};

struct Wmi_Point {
    int x;
    int y;
};

struct Wmi_Vendor_Windows {
    bool lmousedown;
    bool justlmousedown;
    Wmi_Key lastkey;
    bool pressed[256];
    bool released[256];
    HWND win;
    HDC hdc;
    PAINTSTRUCT ps;
    GpGraphics *gs;
};

struct Wmi_Struct {
    // struct Wmi_Bitmap font;
    u32 *buf;
    struct Wmi_Vendor_Windows _vendor;
    u32 current_color;
    struct Wmi_Point mouse_pos;
    struct Wmi_Size window_size;
};

void wmi_set_color(struct Wmi_Struct *self, u8 r, u8 g, u8 b, u8 a);
void wmi_failure(struct Wmi_Struct *self, char *message);
void wmi_draw_rect(struct Wmi_Struct *self, i32 x, i32 y, i32 w, i32 h);
void wmi_draw_bitmap(struct Wmi_Struct *self, const struct Wmi_Bitmap *bitmap, int x, int y, int size_scale);
void wmi_draw_bitmap_region(struct Wmi_Struct *self, const struct Wmi_Bitmap *bitmap, usize srcx, usize srcy, usize srcw, usize srch, int x, int y, int size_scale);

//struct Wmi_Size wmi_measure_text(struct Wmi_Struct *self, const char *text, int size);
//void wmi_draw_text(struct Wmi_Struct *self, const char *text, int x, int y, int size);

void wmi_clear(struct Wmi_Struct *self);
struct Wmi_Size wmi_get_window_size(struct Wmi_Struct *self);
struct Wmi_Bitmap wmi_bitmap_new(usize w, usize h);

// NO RANGE CHECKS ARE DONE!
static inline uint32_t* wmi_bitmap_get_point_unsafe(struct Wmi_Bitmap *self, usize x, usize y) {
    return self->buffer+(x+y*self->w);
}

static inline uint32_t* wmi_bitmap_get_point(struct Wmi_Bitmap *self, usize x, usize y) {
    if (x >= self->w || y >= self->h) return NULL;
    return self->buffer+(x+y*self->w);
}

void wmi_bitmap_drop(struct Wmi_Bitmap *self);
bool wmi_is_key_pressed(struct Wmi_Struct *self, Wmi_Key key);
bool wmi_is_key_released(struct Wmi_Struct *self, Wmi_Key key);
bool wmi_is_lbutton_down(struct Wmi_Struct *self);
bool wmi_is_lbutton_just_down(struct Wmi_Struct *self);
