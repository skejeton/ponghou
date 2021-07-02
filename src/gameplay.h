#pragma once
#include "../deps/enty/enty.h"
#include "../deps/common/macros.h"
#include "./math.h"
#include "controls.h"
#include "../deps/common/types.h"
#include "font.h"
#include "game_state.h"
#include "wminterop.h"

enum Player_Property {
    PROP_BULLET_SPEED,
    PROP_BULLET_COUNT,
    PROP_PLAYER_SIZE,
    PROP_PLAYER_SPEED,
    PROP_PLAYER_SIZE_DECREASE,
    PROP_NEW_BALL,
    PROP_PLAYER_CONVENIENCE_POSITIVE,
    PROP_PLAYER_CONVENIENCE_NEGATIVE,
    PROP_NUL
};

enum Player_Team {
    TEAM_NONE,
    TEAM_LEFT,
    TEAM_RIGHT
};

struct Skill {
    const char *name;
    const char *desc;
    enum Player_Property property;
    struct Skill *children;
    usize children_count;
};


extern const struct Skill BASE_SKILLS[];
extern const struct Skill ROOT_SKILL;

enum Gameplay_Controller {
    PLAYER_1,
    PLAYER_2,
    PLAYER_AI
};

struct Player_Xg {
    enum Gameplay_Controller controller;
    struct Skill *current_skill;
    f64 bullet_speed;
    f64 paddle_scale;
    int bullet_count;
    f64 speed;
};

struct Gameplay_Player {
    struct Player_Xg xg;
    enum Player_Team team;
    f64 paddle_offset;
    int paddle_health;
};

struct Gameplay_Projectile {
    enum Player_Team team;
    f64 x;
    f64 y;
    f64 vx; 
    f64 vy;
};

struct Gameplay_Ball {
    f64 convenience;
    enum Player_Team team;
    f64 x;
    f64 y;
    // Normalized
    f64 vx;
    f64 vy;
    f64 speed;
};

struct Gameplay_State {
    bool finished;
    const char *splash;
    f64 splashcountdown;
    u8 menuflags;
    int scoreleft;
    int scoreright;
    f64 counter;
    struct Gameplay_Player left_side;
    struct Gameplay_Player right_side;
    struct Enty OF(struct Gameplay_Projectile) projectiles;
    struct Enty OF(struct Gameplay_Ball) balls;
};

struct Gameplay_State gameplay_reset(enum Gameplay_Controller left_side, enum Gameplay_Controller right_side);
void gameplay_update(struct Gameplay_State *self, struct Wmi_Struct *wmi, struct Ponghou_Font *font, struct Game_Controls *controls, enum Game_State *state);
