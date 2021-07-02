#include "game.h"
#include "gameplay.h"
#include "loadbmp.h"
#include "menu.h"
#include "wminterop.h"
#include <stdio.h>
#include "logo.h"

struct Game_Struct game_state;

void game_start(struct Wmi_Struct *wmi) {
    (void) wmi;
    struct Ponghou_Font font;
    enum Bitmap_Loading_Status status = load_bmp_from_file("font.bmp", &font.bitmap);
    if (status == BITMAP_FILE_DOESNT_EXIST) {
        wmi_failure(wmi, "font.bmp doesn't exist! make sure you have it alongside your executable");
    }
    game_state = (struct Game_Struct) {
        .font = font, 
        .state = GAME_MENU,
        .menu_state = game_menu_reconstruct(),
        .controls_state = game_controls_default(),
        .gameplay = gameplay_reset(PLAYER_1, PLAYER_2),
        .pulse = 0.0
    };
}

void game_loop(struct Wmi_Struct *wmi) {
    game_state.pulse += 0.016;
    wmi_set_color(wmi, 0, (u8)((sin(game_state.pulse)+1.0)*4), 0, 255);
    wmi_clear(wmi);
    
    if (game_state.state == GAME_MENU) {
        game_render_menu(&game_state.menu_state, &game_state.font, wmi, game_state.pulse, &game_state.state, &game_state.gameplay);
    }
    if (game_state.state == GAME_GAMEPLAY) {
        gameplay_update(&game_state.gameplay, wmi, &game_state.font, &game_state.controls_state, &game_state.state);
    }
    if (game_state.state == GAME_CONTROLS) {
        game_controls_handle(&game_state.controls_state, &game_state.state, wmi, &game_state.font);
    }
    // Draw cursor
    wmi_set_color(wmi, 255, 255, 255, 255);
    wmi_draw_rect(wmi, wmi->mouse_pos.x-1, wmi->mouse_pos.y-1, 12, 12);

    wmi_set_color(wmi, 0, 0, 0, 255);
    wmi_draw_rect(wmi, wmi->mouse_pos.x, wmi->mouse_pos.y, 10, 10);
    if (wmi_is_lbutton_down(wmi))
        wmi_set_color(wmi, 255, 0, 255, 255);
    else
        wmi_set_color(wmi, 255, 255, 255, 255);
    wmi_draw_rect(wmi, wmi->mouse_pos.x+5, wmi->mouse_pos.y+5, 10, 10);
}
