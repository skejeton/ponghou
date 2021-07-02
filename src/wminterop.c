#include "wminterop.h"
#include <gdiplus/gdiplusenums.h>
#include <gdiplus/gdiplusflat.h>
#include <gdiplus/gdiplusgpstubs.h>
#include <gdiplus/gdiplustypes.h>
#include <stdio.h>
#include <stringapiset.h>
#include <winnls.h>
#include <winnt.h>

void wmi_set_color(struct Wmi_Struct *self, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    u32 ax = ((u32) a) << 24;
    u32 rx = ((u32) r) << 16;
    u32 gx = ((u32) g) << 8;
    u32 bx = ((u32) b) << 0;
    self->current_color = ax | rx | gx | bx;
}

void wmi_failure(struct Wmi_Struct *self, char *message) {
    WCHAR buf[2048];
    MultiByteToWideChar(CP_UTF8, 0, message, -1, (LPWSTR) buf, 2048);
    MessageBox(self->_vendor.win, (LPCWSTR) &buf, (LPCWSTR) L"Ponghou: Fatal error", 0x00000000L|0x00000010L);
    PostQuitMessage(-1);
}

void wmi_draw_rect(struct Wmi_Struct *self, int x, int y, int w, int h) { 
    if ((self->current_color & 0xFF000000) >> 24 < 0x80) return;
    if (x + w > self->window_size.x) w = self->window_size.x-x;
    if (y + h > self->window_size.y) h = self->window_size.y-y;
    if (x < 0) { w += x; x = 0;  }
    if (y < 0) { h += y; y = 0;  }

    for (int dx = 0; dx < w; dx += 1) {
        for (int dy = 0; dy < h; dy += 1) {
            self->buf[(x+dx)+(y+dy)*self->window_size.x] = (0xFF000000 - (self->current_color & 0xFF000000)) | (self->current_color & 0x00FFFFFF); 
        }
    }
}

void wmi_draw_bitmap(struct Wmi_Struct *self, const struct Wmi_Bitmap *bitmap, int x, int y, int size_scale) {
    u32 color_rem = self->current_color;
    for (int i = 0; i < (int) bitmap->w; i += 1) {
        for (int j = 0; j < (int) bitmap->h; j += 1) {
            self->current_color = *wmi_bitmap_get_point_unsafe((struct Wmi_Bitmap*)bitmap, (usize) i, (usize) j);
            wmi_draw_rect(self, x+i*size_scale, y+j*size_scale, size_scale, size_scale);
        }
    }
    self->current_color = color_rem;
}

void wmi_draw_bitmap_region(struct Wmi_Struct *self, const struct Wmi_Bitmap *bitmap, usize srcx, usize srcy, usize srcw, usize srch, int x, int y, int size_scale) {
    u32 color_rem = self->current_color;
    for (int i = 0; i < (int) srcw; i += 1) {
        for (int j = 0; j < (int) srch; j += 1) {
            u32 *point = wmi_bitmap_get_point((struct Wmi_Bitmap*)bitmap, (usize) i + srcx, (usize) j + srcy);
            if (point) {
                self->current_color = (*point)&color_rem;
                wmi_draw_rect(self, x+i*size_scale, y+j*size_scale, size_scale, size_scale);
            }
        }
    }
    self->current_color = color_rem;
}

// Text conversion buffer
/*
struct Wmi_Size wmi_measure_text(struct Wmi_Struct *self, const char *text, int size) {
    MultiByteToWideChar(CP_UTF8, 0, text, -1, (LPWSTR) tconvb, 20000);
    GpFontFamily *family;
    GdipGetGenericFontFamilySansSerif(&family);
    GpFont *font;
    GdipCreateFont(family, (float) size, FontStyleRegular, UnitPixel, &font);
    GpBrush *brush;
    GdipCreateSolidFill(self->current_color, &brush);
    GpStringFormat *format;
    GdipCreateStringFormat(0, LANG_NEUTRAL, &format);
    GdipStringFormatGetGenericDefault(&format);

    RectF box;

    GdipMeasureString(self->_vendor.gs, tconvb, -1, font, &(RectF) { 0, 0, 10000, 10000}, format, &box, NULL, NULL);
    return (struct Wmi_Size) { (int) box.Width, (int) box.Height };
}

void wmi_draw_text(struct Wmi_Struct *self, const char *text, int x, int y, int size) {
    MultiByteToWideChar(CP_UTF8, 0, text, -1, (LPWSTR) tconvb, 20000);
    GpFontFamily *family;
    GdipGetGenericFontFamilySansSerif(&family);
    GpFont *font;
    GdipCreateFont(family, (float) size, FontStyleRegular, UnitPixel, &font);
    GpBrush *brush;
    GdipCreateSolidFill(self->current_color, &brush);
    GpStringFormat *format;
    GdipCreateStringFormat(0, LANG_NEUTRAL, &format);
    GdipStringFormatGetGenericDefault(&format);
 // GdipMeasureString(gp, text, -1, font, 
    GdipDrawString(self->_vendor.gs, tconvb, -1, font, &(RectF) { (float) x, (float) y, 10000, 10000}, format, brush);
}
*/
void wmi_clear(struct Wmi_Struct *self) {
    wmi_draw_rect(self, 0, 0, self->window_size.x, self->window_size.y);
}

struct Wmi_Size wmi_get_window_size(struct Wmi_Struct *self) {
    return self->window_size;
}

struct Wmi_Bitmap wmi_bitmap_new(usize w, usize h) {
    return (struct Wmi_Bitmap) {
        .w = w,
        .h = h,
        .buffer = malloc(w*h*4)
    };
}

void wmi_bitmap_drop(struct Wmi_Bitmap *self) {
    free(self->buffer);
}

bool wmi_is_key_pressed(struct Wmi_Struct *self, Wmi_Key key) {
    return self->_vendor.pressed[key];
}

bool wmi_is_key_released(struct Wmi_Struct *self, Wmi_Key key) {
     bool ret = self->_vendor.pressed[key];   
     self->_vendor.released[key] = false;
     return ret;
}

bool wmi_is_lbutton_down(struct Wmi_Struct *self) {
    return self->_vendor.lmousedown;
}
bool wmi_is_lbutton_just_down(struct Wmi_Struct *self) {
    return self->_vendor.justlmousedown;
}

