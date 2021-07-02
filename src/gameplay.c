#include "gameplay.h"
#include "controls.h"
#include "font.h"
#include <time.h>
#include "game_state.h"
#include "wminterop.h"
#include <stdbool.h>
#include <stdio.h>


const struct Skill SHOOTING_SUBSKILLS[] = {{
    .name = "Bullet Speed",
    .desc = "",
    .property = PROP_BULLET_SPEED,
    .children = NULL,
    .children_count = 0
}, {
    .name = "Bullet Count",
    .desc = "",
    .property = PROP_BULLET_COUNT,
    .children = NULL,
    .children_count = 0
}};

const struct Skill ENCHANCING_SUBSKILLS[] = {{
    .name = "Paddle Speed",
    .desc = "",
    .property = PROP_PLAYER_SPEED,
    .children = NULL,
    .children_count = 0
}, {
    .name = "Increase Player Size",
    .desc = "",
    .property = PROP_PLAYER_SIZE,
    .children = NULL,
    .children_count = 0
}, {
    .name = "Decrease Player Size",
    .desc = "",
    .property = PROP_PLAYER_SIZE_DECREASE,
    .children = NULL,
    .children_count = 0
}};

const struct Skill DUALITY_SUBSKILLS[] = {{
    .name = "Attraction",
    .desc = "Your ball will move towards your paddle more",
    .property = PROP_PLAYER_CONVENIENCE_POSITIVE,
    .children = NULL,
    .children_count = 0
}, {
    .name = "Fear",
    .desc = "Your ball will move away from your enemy more (mutually exclusive with attraction)",
    .property = PROP_PLAYER_CONVENIENCE_NEGATIVE,
    .children = NULL,
    .children_count = 0
}};

const struct Skill BASE_SKILLS[] = {{
    .name = "Enchancing",
    .desc = "Enchancing your paddle by speed, size, and other",
    .property = PROP_PLAYER_SPEED,
    .children = (struct Skill*)&ENCHANCING_SUBSKILLS,
    .children_count = 3
}, {
    .name = "Shooting",
    .desc = "Allows ability to shoot making your player lose their health",
    .property = PROP_BULLET_COUNT,
    .children = (struct Skill*)&SHOOTING_SUBSKILLS,
    .children_count = 2
}, {
    .name = "Duality",
    .desc = "Creates a ball that deals less damage to you when you miss it",
    .property = PROP_NEW_BALL,
    .children = (struct Skill*)&DUALITY_SUBSKILLS,
    .children_count = 2
}};

const struct Skill ROOT_SKILL = {
    .name = "Base skill",
    .desc = "This should not be accessible to read",
    .property = PROP_NUL,
    .children = (struct Skill*) &BASE_SKILLS,
    .children_count = 3
};

struct Game_Color { 
    u8 r;
    u8 g;
    u8 b;
    u8 a;
};

void set_game_color(struct Wmi_Struct *wmi, struct Game_Color color) {
    wmi_set_color(wmi, color.r, color.g, color.b, color.a);
}

const f64 TEAM_X_OFFSETS[] = {
    [TEAM_LEFT] = -0.45,
    [TEAM_RIGHT] = 0.45
};

const f64 TEAM_X_DIRECTIONS[] = {
    [TEAM_LEFT] = 1,
    [TEAM_RIGHT] = -1
};

const struct Game_Color TEAM_COLORS[] = {
    [TEAM_NONE] = { 255, 255, 255, 255 },
    [TEAM_LEFT] = { 255, 255, 0, 255 },
    [TEAM_RIGHT] = { 0, 255, 255, 255 }
};

struct Gameplay_Ball reset_ball(enum Player_Team team, f64 convenience) {
    return (struct Gameplay_Ball) { convenience, team, 0.0, 0.0, (rand() % 2 == 0 ? 1 : -1) * 1/(sqrt(2)*(f64)(rand()%2+1)), (rand() % 2 == 0 ? 1 : -1) * 1/(sqrt(2)*(f64)(rand()%2+1)), 0.01 };
}

struct Gameplay_State gameplay_reset(enum Gameplay_Controller left_side, enum Gameplay_Controller right_side) {
    struct Gameplay_State self = (struct Gameplay_State) {
        .finished = false,
        .splash = "First to 10",
        .splashcountdown = 1,
        .menuflags = 0x3,
        .counter = 0.01,
        .scoreleft = 0,
        .scoreright = 0,
        .left_side = {
            .xg = {
                .controller = left_side,
                .current_skill = (struct Skill*)&ROOT_SKILL,
                .bullet_speed = 0.01,
                .bullet_count = 0,
                .speed = 0.01,
                .paddle_scale = 1.0,
            },
            .team = TEAM_LEFT,
            .paddle_offset = 0,
            .paddle_health = 100,
        },
        .right_side = {
            .xg = {
                .controller = right_side,
                .current_skill = (struct Skill*)&ROOT_SKILL,
                .bullet_speed = 0.01,
                .bullet_count = 0,
                .speed = 0.01,
                .paddle_scale = 1.0,
            },
            .team = TEAM_RIGHT,
            .paddle_offset = 0,
            .paddle_health = 100,
        },
        .projectiles = enty_new(sizeof(struct Gameplay_Projectile)),
        .balls = enty_new(sizeof(struct Gameplay_Ball)) 
    };

    struct Gameplay_Ball initial = reset_ball(TEAM_NONE, 0);
    enty_insert(&self.balls, &initial);
    return self;
}   


static struct Game_Rect get_field_rect(struct Wmi_Size wsize) {
    // The field rect is the center of the screen with radius of minimal axis divided y 2$
    int min_axis = wsize.x < wsize.y ? wsize.x : wsize.y;
    struct Wmi_Point midpoint = { wsize.x / 2, wsize.y / 2 };
    return (struct Game_Rect) {
        .x = midpoint.x - wsize.x/2,
        .y = midpoint.y - min_axis/2,
        .w = wsize.x,
        .h = min_axis
    };
}

static void render_field(struct Gameplay_State *self, struct Wmi_Struct *wmi) {
    (void) self;
    // Draw frame
    struct Game_Rect rect = get_field_rect(wmi_get_window_size(wmi));
    wmi_set_color(wmi, 255, 255, 255, 255);
    wmi_draw_rect(wmi, rect.x, rect.y, rect.w, rect.h);
    wmi_set_color(wmi, 0, 0, 0, 255);
    wmi_draw_rect(wmi, rect.x+6, rect.y+6, rect.w-12, rect.h-12);
}

static struct Wmi_Point map_to_screen(struct Wmi_Struct *wmi, f64 x, f64 y) {
    struct Game_Rect rect = get_field_rect(wmi_get_window_size(wmi));
    return (struct Wmi_Point) {
        rect.x+(rect.w/2)+(int)(x*(f64)rect.w),
        rect.y+(rect.h/2)+(int)(y*(f64)rect.h),
    };
}

struct Rectf {
    f64 x;
    f64 y;
    f64 w;
    f64 h;
};

static struct Rectf paddle_world_rect(struct Gameplay_Player *player) {
    f64 paddlex = TEAM_X_OFFSETS[player->team];
    f64 paddlew = 0.025;
    f64 paddleh = ((f64)(player->paddle_health)/500)*player->xg.paddle_scale;
    f64 paddley = player->paddle_offset;
    return (struct Rectf) {
        paddlex-paddlew/2,
        paddley-paddleh/2,
        paddlew,
        paddleh
    };
}

static void decide_move(struct Gameplay_State *self, struct Gameplay_Player *player, struct Gameplay_Player *foe) {
    struct Rectf world = paddle_world_rect(player);
    int weight = 0;
    for (
        usize i = enty_first_index(&self->balls);
        !enty_is_free(&self->balls, i);
        i = enty_advance_index(&self->balls, i)
    ) {
        struct Gameplay_Ball *the_ball = enty_get(&self->balls, i);

        int teamfactor = the_ball->team == player->team ? 5 : 10;
        if ((player->team == TEAM_LEFT ? the_ball->vx < 0 : the_ball->vx > 0) && fabs(the_ball->y-player->paddle_offset) > 0.03) {
            if (the_ball->y < player->paddle_offset) {
                weight -= teamfactor;
            }
            else {
                weight += teamfactor;
            }
        }
        else if (fabs(foe->paddle_offset-player->paddle_offset)>0.01) {
            if (foe->paddle_offset < player->paddle_offset) {
                weight -= player->xg.bullet_count;
            }
            else {
                weight += player->xg.bullet_count;
            }
        }
    }

    for (
        usize i = enty_first_index(&self->projectiles);
        !enty_is_free(&self->projectiles, i);
        i = enty_advance_index(&self->projectiles, i)
    ) {
        struct Gameplay_Projectile *proj = enty_get(&self->projectiles, i);
        if (proj->team == player->team || (proj->y < world.y && proj->y > (world.y+world.h)))
            continue;
        if (proj->y < player->paddle_offset) {
            weight += 2;
        }
        else {
            weight -= 2;
        }
    }

    if (weight >= 2)
        player->paddle_offset += player->xg.speed;
    else if (weight <= -2)
        player->paddle_offset -= player->xg.speed;
}   

static void handle_input(struct Gameplay_State *state, struct Wmi_Struct *wmi, struct Game_Controls *controls) {
    (void)wmi;
    if (state->left_side.xg.controller == PLAYER_AI)
        decide_move(state, &state->left_side, &state->right_side);
    else {
        struct Gameplay_Player *player = &state->left_side;
        if (wmi_is_key_pressed(wmi, controls->leftup)) {
            player->paddle_offset -= player->xg.speed;
        }
        if (wmi_is_key_pressed(wmi, controls->leftdown)) {
            player->paddle_offset += player->xg.speed;
        }
    }
    if (state->right_side.xg.controller == PLAYER_AI)
        decide_move(state, &state->right_side, &state->left_side);
    else {
        struct Gameplay_Player *player = &state->right_side;
        if (wmi_is_key_pressed(wmi, controls->rightup)) {
            player->paddle_offset -= player->xg.speed;
        }
        if (wmi_is_key_pressed(wmi, controls->rightdown)) {
            player->paddle_offset += player->xg.speed;
        }
    }
}

static void paddle(struct Gameplay_Player *player, struct Wmi_Struct *wmi) {
    struct Rectf world_loc = paddle_world_rect(player);
    struct Wmi_Point xy = map_to_screen(wmi, world_loc.x, world_loc.y);
    struct Wmi_Point xy2 = map_to_screen(wmi, world_loc.x+world_loc.w, world_loc.y+world_loc.h);

    set_game_color(wmi, TEAM_COLORS[player->team]);   
    wmi_draw_rect(wmi, xy.x, xy.y, xy2.x-xy.x, xy2.y-xy.y);


    world_loc = paddle_world_rect(player);

    if ((world_loc.y+world_loc.h) > 0.5)
        player->paddle_offset = 0.5-world_loc.h/2;

    if ((world_loc.y) < -0.5)
        player->paddle_offset = -0.5+world_loc.h/2;

}

static bool point_in_rect(f64 x, f64 y, struct Rectf rect) {
    return x > rect.x && y > rect.y && x < (rect.x+rect.w) && y < (rect.y+rect.h);
}

static void ballvspaddle(struct Gameplay_Player *player, struct Gameplay_Ball *ball) {
    struct Rectf player_paddle = paddle_world_rect(player);

    if (point_in_rect(ball->x, ball->y, player_paddle)) {
        ball->vx = -ball->vx + TEAM_X_DIRECTIONS[player->team]*fabs(ball->y-(player_paddle.y+player_paddle.h/2))*7.0;
        if (player->team == TEAM_LEFT)
            ball->x = player_paddle.x+player_paddle.w;
        else
            ball->x = player_paddle.x;
    }
}

static void exchange_paddle(struct Gameplay_State *self, int amount) {
    self->left_side.paddle_health += amount;
    self->right_side.paddle_health -= amount;
    if (self->left_side.paddle_health < 0)
        self->left_side.paddle_health = 0;
    if (self->right_side.paddle_health < 0)
        self->right_side.paddle_health = 0;
}

static void normalize(f64 *vx, f64 *vy) {
    f64 len = sqrt(*vx**vx+*vy**vy);
    *vx = *vx/len;
    *vy = *vy/len;
}

static void ball(struct Gameplay_State *self, struct Wmi_Struct *wmi, struct Gameplay_Ball *ball) {
    struct Wmi_Point point = map_to_screen(wmi, ball->x, ball->y);

    if (ball->vx > -0.2 && ball->vx < 0.2) {
        if (ball->vx < 0)
            ball->vx = -0.2;
        else 
            ball->vx = 0.2;
    }


    normalize(&ball->vx, &ball->vy);
    ball->x += ball->vx*ball->speed;
    ball->y += ball->vy*ball->speed;

    if (ball->convenience > 0) {
        if (ball->team == TEAM_LEFT && ball->vx < 0) {
            ball->y += (self->left_side.paddle_offset-ball->y)*ball->convenience;
        }
        else if (ball->team == TEAM_RIGHT && ball->vx > 0) {
            ball->y += (self->right_side.paddle_offset-ball->y)*ball->convenience;
        }
    }
    else if (ball->convenience < 0) {
        if (ball->team == TEAM_LEFT && ball->vx > 0) {
            ball->y -= (self->right_side.paddle_offset > 0 ? 1 : -1)*(-ball->convenience);
        }
        else if (ball->team == TEAM_RIGHT && ball->vx < 0) {
            ball->y -= (self->left_side.paddle_offset > 0 ? 1 : -1)*(-ball->convenience);
        }
    }


    ball->speed += 0.000016;
    set_game_color(wmi, TEAM_COLORS[ball->team]);
    wmi_draw_rect(wmi, point.x-10, point.y-10, 20, 20);
    if (ball->x < -0.5) {
        *ball = reset_ball(ball->team, ball->convenience);
        if (ball->team == TEAM_LEFT)
            exchange_paddle(self, -self->left_side.paddle_health/4);
        else
            exchange_paddle(self, -self->left_side.paddle_health/2);
    }
    if (ball->x > 0.5) {
        *ball = reset_ball(ball->team, ball->convenience);
        if (ball->team == TEAM_RIGHT)
            exchange_paddle(self, self->right_side.paddle_health/4);
        else
            exchange_paddle(self, self->right_side.paddle_health/2);
    }
    if (ball->y < -0.5) {
        ball->y = -0.5;
        ball->vy = -ball->vy;
    }
    if (ball->y > 0.5) {
        ball->y = 0.5;
        ball->vy = -ball->vy; 
    }
}

static bool projectilevspaddle(struct Gameplay_State *self, struct Gameplay_Player *player, struct Gameplay_Projectile *proj) {
    if (point_in_rect(proj->x, proj->y, paddle_world_rect(player))) {
        exchange_paddle(self, (int)TEAM_X_DIRECTIONS[player->team]*-10);
        return true;
    }
    return false;
}

void projectiles(struct Gameplay_State *self, struct Ponghou_Font *font, struct Wmi_Struct *wmi) {
    for (
        usize i = enty_first_index(&self->projectiles);
        !enty_is_free(&self->projectiles, i);
        i = enty_advance_index(&self->projectiles, i)
    ) {
        struct Gameplay_Projectile *proj = enty_get(&self->projectiles, i);
        
        proj->x += proj->vx;
        proj->y += proj->vy;
        
        struct Gameplay_Player *foe = 
            proj->team == TEAM_LEFT ?
            &self->right_side :
            &self->left_side;


       // proj->vy += sin(proj->x*10.0)/100.0;
       
        if (proj->x > 0.5 || proj->y > 0.5 || proj->x < -0.5 || proj->y < -0.5 || projectilevspaddle(self, foe, proj))
        {
            enty_free_index(&self->projectiles, i);
            continue;
        }

        struct Game_Color color = TEAM_COLORS[proj->team];
        color.g = 128;
        set_game_color(wmi, color);
        
        struct Wmi_Point screen = map_to_screen(wmi, proj->x, proj->y);

        const char *THINGS[] = { "*", "+", "-", "=" };
        font_render_text(font, wmi, THINGS[rand()%4], screen.x, screen.y, 4);
    }
}

bool isset = false;
static void apply_skill(struct Gameplay_State *self, struct Gameplay_Player *player, enum Player_Property prop) {
    if (prop == PROP_BULLET_COUNT) {
        player->xg.bullet_count += 1;
    }
    else if (prop == PROP_BULLET_SPEED) {
        player->xg.bullet_speed += 0.01;
    }
    else if (prop == PROP_PLAYER_SPEED) {
        player->xg.speed += 0.01;
    }
    else if (prop == PROP_PLAYER_SIZE) {
        player->xg.paddle_scale += 0.1;
    }
    else if (prop == PROP_PLAYER_SIZE_DECREASE) {
        player->xg.paddle_scale /= 1.1;
    }
    else if (prop == PROP_NEW_BALL) {
        struct Gameplay_Ball ball = reset_ball(player->team, 0);
        enty_insert(&self->balls, &ball);
    }
    else if (prop == PROP_PLAYER_CONVENIENCE_NEGATIVE) {
        for (
            usize i = enty_first_index(&self->balls);
            !enty_is_free(&self->balls, i);
            i = enty_advance_index(&self->balls, i)
        ) {
            struct Gameplay_Ball *the_ball = enty_get(&self->balls, i);
            if (the_ball->team == player->team)
                the_ball->convenience -= 0.001;
        }
    }
    else if (prop == PROP_PLAYER_CONVENIENCE_POSITIVE) {
        for (
            usize i = enty_first_index(&self->balls);
            !enty_is_free(&self->balls, i);
            i = enty_advance_index(&self->balls, i)
        ) {
            struct Gameplay_Ball *the_ball = enty_get(&self->balls, i);
            if (the_ball->team == player->team)
                the_ball->convenience += 0.005;
        }
    }
}

const char *STR_PROPS[] = {
    [PROP_BULLET_SPEED] = "Makes your bullets faster",
    [PROP_BULLET_COUNT] = "Adds additional bullet",
    [PROP_PLAYER_SPEED] = "Makes you move faster",
    [PROP_PLAYER_SIZE] = "Makes you bigger (no health gained)",
    [PROP_PLAYER_SIZE_DECREASE] = "Makes you smaller (no health lost)",
    [PROP_NEW_BALL] = "Creates a new ball",
    [PROP_PLAYER_CONVENIENCE_NEGATIVE] = "Ball will move towards you",
    [PROP_PLAYER_CONVENIENCE_POSITIVE] = "Ball will move away from your enemy",
};

static void render_skill_menu(struct Gameplay_Player *player, struct Gameplay_State *self, struct Wmi_Struct *wmi, struct Ponghou_Font *font) {
    set_game_color(wmi, TEAM_COLORS[player->team]);   
    const char *text = player->team == TEAM_LEFT ? "Player 1: Choose your skill" : "Player 2: Choose your skill";
    font_render_text(font, wmi, text, wmi->window_size.x/2-font_measure_text(font, text, 8).x/2, 20, 8);

    if (player->xg.current_skill->children_count == 0 && player->xg.current_skill->children != NULL) {
        if (player->team == TEAM_LEFT)
            self->menuflags &= 0x2;
        else
            self->menuflags &= 0x1;
    }

    for (usize i = 0; i < player->xg.current_skill->children_count; i += 1) {
        wmi_set_color(wmi, 255, 255, 255, 255);
        const struct Skill *sk = &player->xg.current_skill->children[i];
        struct Wmi_Size fm = font_measure_text(font, sk->name, 4);
        int x = wmi->window_size.x/2-font_measure_text(font, sk->name, 4).x/2;
        int y = 200+(int)i*120;
        struct Wmi_Point m = wmi->mouse_pos;
        if (m.x > x && m.y > y && m.x < (x+fm.x) && m.y < (y+fm.y)) {
            set_game_color(wmi, TEAM_COLORS[player->team]);
            if (wmi_is_lbutton_just_down(wmi)) {
                if (player->team == TEAM_LEFT)
                    self->menuflags &= 0x2;
                else
                    self->menuflags &= 0x1;
                if (sk->children != NULL) {
                    player->xg.current_skill = (struct Skill*)sk;
                }
                apply_skill(self, player, sk->property);
            }
        }
        font_render_text(font, wmi, sk->name, wmi->window_size.x/2-font_measure_text(font, sk->name, 4).x/2, 200+(int)i*120, 4);
        font_render_text(font, wmi, sk->desc, wmi->window_size.x/2-font_measure_text(font, sk->desc, 2).x/2, 250+(int)i*120, 2);
            wmi_set_color(wmi, 128, 140, 128, 255);
        font_render_text(font, wmi, STR_PROPS[sk->property], wmi->window_size.x/2-font_measure_text(font, STR_PROPS[sk->property], 2).x/2, 280+(int)i*120, 2);
    }
}

static void shoot_from_paddle(struct Gameplay_Player *player, int index, struct Enty OF(struct Gameplay_Projectile) *projectiles) {
    enty_insert(projectiles, &(struct Gameplay_Projectile) {
        .team = player->team,
        .x = TEAM_X_OFFSETS[player->team],
        .y = player->paddle_offset,
        .vx =  TEAM_X_DIRECTIONS[player->team]*player->xg.bullet_speed*((f64)(rand()%1000+500)/1000.0),
        .vy = (f64)(index-player->xg.bullet_count/2)/(250.0*player->xg.bullet_count),
    });
}

static void next_round(struct Gameplay_State *self, int delta_left, int delta_right) {
    u8 menuflags = self->menuflags;
    struct Player_Xg left_side = self->left_side.xg;
    struct Player_Xg right_side = self->right_side.xg;
    struct Enty balls = self->balls;

    int scoreright = self->scoreright+delta_right;
    int scoreleft = self->scoreleft+delta_left;

    enty_drop(&self->projectiles);

    *self = gameplay_reset(self->left_side.xg.controller, self->right_side.xg.controller);
    
   
    enty_drop(&self->balls);

    self->balls = balls;
    for (
        usize i = enty_first_index(&self->balls);
        !enty_is_free(&self->balls, i);
        i = enty_advance_index(&self->balls, i)
    ) {
        struct Gameplay_Ball *the_ball = enty_get(&self->balls, i);
        *the_ball = reset_ball(the_ball->team, the_ball->convenience);
    }

    self->menuflags = menuflags;
    self->splashcountdown = 0;

    self->left_side.xg = left_side;
    self->right_side.xg = right_side;
    self->scoreleft = scoreleft;
    self->scoreright = scoreright;
}

static void random_skill(struct Gameplay_State *self, struct Gameplay_Player *player) {
    if (player->xg.current_skill->children_count != 0) {
        struct Skill *rndskill = &player->xg.current_skill->children[((usize)rand()%player->xg.current_skill->children_count)];
        apply_skill(self, player, rndskill->property);
        if (rndskill->children != NULL)
            player->xg.current_skill = rndskill;
    }
    if (player->team == TEAM_LEFT)
        self->menuflags &= 0x2;
    else
        self->menuflags &= 0x1;
}

void render_splash(struct Gameplay_State *self, struct Wmi_Struct *wmi, struct Ponghou_Font *font) {
    wmi_set_color(wmi, 255, 255, 255, 255);
    font_render_text(font, wmi, self->splash, wmi->window_size.x/2-font_measure_text(font, self->splash, 8).x/2, wmi->window_size.y/2-40, 8);
}


void gameplay_update(struct Gameplay_State *self, struct Wmi_Struct *wmi, struct Ponghou_Font *font, struct Game_Controls *controls, enum Game_State *state) {
    srand((u32)time(NULL)+(u32)(rand()%400));
    if (self->scoreleft >= 10 && !self->finished) {
        self->finished = true;
        self->splash = "Player 1 won!";
        self->splashcountdown = 1;
        self->menuflags = 0;
    }
    else if (self->scoreright >= 10 && !self->finished) {
        self->finished = true;
        self->splash = "Player 2 won!";
        self->splashcountdown = 1;
        self->menuflags = 0;
    }
    if (self->menuflags & 0x1) {
        if (self->left_side.xg.controller == PLAYER_AI) random_skill(self, &self->left_side);
        else render_skill_menu(&self->left_side, self, wmi, font);
        return;
    }
    if (self->menuflags & 0x2) {
        if (self->right_side.xg.controller == PLAYER_AI) random_skill(self, &self->right_side);
        else render_skill_menu(&self->right_side, self, wmi, font);
        return;
    }
    if (self->splashcountdown > 0) {
        self->splashcountdown -= 0.016;
        render_splash(self, wmi, font);
        return;
    }
    if (self->finished) {
        *state = GAME_MENU;
    }
    self->counter += 0.016;
    if (self->counter > 1.0) {
        for (int i = 0; i < self->right_side.xg.bullet_count; i += 1) {
            shoot_from_paddle(&self->right_side, i, &self->projectiles);
        }
        for (int i = 0; i < self->left_side.xg.bullet_count; i += 1) {
            shoot_from_paddle(&self->left_side, i, &self->projectiles);
        }
        self->counter = 0;
    }
    render_field(self, wmi);
    handle_input(self, wmi, controls);
    paddle(&self->left_side, wmi);
    paddle(&self->right_side, wmi);
    for (
        usize i = enty_first_index(&self->balls);
        !enty_is_free(&self->balls, i);
        i = enty_advance_index(&self->balls, i)
    ) {
        struct Gameplay_Ball *the_ball = enty_get(&self->balls, i);
        ballvspaddle(&self->left_side, the_ball);
        ballvspaddle(&self->right_side, the_ball);

        ball(self, wmi, the_ball);
    }
    if (self->left_side.paddle_health < 10) {
        self->menuflags |= 0x2;
        next_round(self, 0, 1);
    }
    if (self->right_side.paddle_health < 10) {
        self->menuflags |= 0x1;
        next_round(self, 1, 0);
    }
    wmi_set_color(wmi, 255, 255, 0, 255);   
    char buf[256] = { 0 }; 
    sprintf(buf, "%d", self->scoreleft);
    font_render_text(font, wmi, buf, wmi_get_window_size(wmi).x/2, 20, 10); 
    wmi_set_color(wmi, 0, 255, 255, 255);   
    sprintf(buf, "%d", self->scoreright);
    font_render_text(font, wmi, buf, wmi_get_window_size(wmi).x/2, wmi_get_window_size(wmi).y-120, 10); 

    projectiles(self, font, wmi);
}
