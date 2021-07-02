#include "font.h"
#include "wminterop.h"

typedef const char *Letter_T[10];

Letter_T LETTER_N_BIG = {
    "#     ",
    "#   # ",
    "#   # ",
    "#   # ",
    "#   # ",
    "# # # ",
    "#   # ",
    "#   # ",
    "#   # ",
    "    # "
};

Letter_T LETTER_N_SMALL = {
    "      ",
    "      ",
    "      ",
    "      ",
    "#     ",
    "#   # ",
    "# # # ",
    "#   # ",
    "#   # ",
    "    # "
};

void font_render_text(const struct Ponghou_Font *self, struct Wmi_Struct *wmi, const char *_str, int x, int y, int scale) {
    const unsigned char *str = (const unsigned char*)_str; 
    int ox = x;
    int oy = y;
    for (usize i = 0; str[i]; i += 1) {
        if (str[i] == '\n') {
            ox = x;
            oy += 11*scale;
            continue;
        }
        if (str[i] == ' ') {
            ox += 4*scale;
            continue;
        }
        if (str[i] == '\t') {
            ox += 4*4*scale;
            continue;
        }
        if (str[i] < 33) {
            wmi_draw_bitmap_region(wmi, &self->bitmap, 376, 0, 4, 10, ox, oy, scale);
            ox += 4*scale;
            continue;    
        }
        if (str[i] == 'N' || str[i] == 'n') {
            Letter_T *letter = str[i] == 'N' ? &LETTER_N_BIG : &LETTER_N_SMALL;
            for (int dx = 0; dx < 6; dx += 1) {
                for (int dy = 0; dy < 10; dy += 1) {
                    if ((*letter)[dy][dx] == '#') {
                        wmi_draw_rect(wmi, ox+dx*scale, oy+dy*scale, scale, scale);
                    }
                }               
            }
            ox += 6*scale;
        }
        else {
            wmi_draw_bitmap_region(wmi, &self->bitmap, (usize)(str[i]-33)*4, 0, 4, 10, ox, oy, scale);
            ox += 4*scale;
        }
    }
}

struct Wmi_Size font_measure_text(const struct Ponghou_Font *self, const char *str, int scale) {
    (void) self;
    struct Wmi_Size size = { 0, 0 };
    int x = 0;
    int y = 0; 
    for (usize i = 0; str[i]; i += 1) {
        if (str[i] == '\n') {
            x = 0;
            y += 11;
            continue;
        }
        if (str[i] == '\t') {
            x += 4*4;
            continue;
        }
        int glyph_size = str[i] == 'N' || str[i] == 'n' ? 6 : 4;
        x += glyph_size;
        size.x = size.x > x ? size.x : x;
    }
    size.y = y + 11;
    size.x *= scale;
    size.y *= scale;
    return size;
}
