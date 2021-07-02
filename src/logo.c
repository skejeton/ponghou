#include "logo.h"
#include "wminterop.h"

const char *logo[10] = {
    "###               # #",
    "# #               # #",
    "# #               # #",
    "# #               # #",
    "# #     #         # #",
    "###     #   #     ###",
    "#   ### # # # ### # # ### # #",
    "#   # # #   # #   # # # # # #",
    "#   # # #   # # # # # # # # #",
    "#   ###     # ### # # ### ###"
};

const int LOGO_SIZE_X = 28;
const int LOGO_SIZE_Y = 10;
const int LOGO_SIZE_FAC = 6;

static void draw_logo(struct Wmi_Struct *wmi, int x, int y) {
    const char *lp;
    for (int i = 0; i < LOGO_SIZE_Y; i += 1) {
        lp = logo[i];
        int j = 0;
        while (*lp) {
            if (*lp == '#')
                wmi_draw_rect(wmi, j*LOGO_SIZE_FAC+x, i*LOGO_SIZE_FAC+y, LOGO_SIZE_FAC, LOGO_SIZE_FAC);
            lp += 1;
            j += 1;
        }
    }
}

void logo_paint_and_update(struct LogoState *self, struct Wmi_Struct *wmi) {
    const u8 COLORS[][4] = { { 0, 0, 255, 255 }, { 0, 255, 0, 255 }, { 255, 0, 0, 255 }, { 255, 255, 255, 255 } }; 
    for (int _i = 0; _i < (self->angle > 4 ? 4 : self->angle); _i += 1) {
        f32 angle_delta = (f32)_i/4.0f;
        int i = _i % 4;
        f32 x_offset = cosf(self->angle+angle_delta)*30;
        f32 y_offset = sinf(self->angle+angle_delta)*30;
        struct Wmi_Size wsize = wmi_get_window_size(wmi);
        wmi_set_color(wmi, COLORS[i][0], COLORS[i][1], COLORS[i][2], COLORS[i][3]);
        int x = wsize.x/2-LOGO_SIZE_X*LOGO_SIZE_FAC/2;
        int y = LOGO_SIZE_FAC*LOGO_SIZE_Y;
        draw_logo(wmi, x+(int)x_offset, y+(int)y_offset);
    }
    self->angle += 0.1f;
}
