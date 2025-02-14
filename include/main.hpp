#pragma once

#include <string>
#include <cmath> // places()
#include <unordered_map>
#include <random>

#include <SDL2/SDL.h>

#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

#include "types.hpp"

s32 rand_range(s32 min, s32 max);
s32 rand_range_const(s32 min, s32 max);
void rand_const_reset();

struct WASD {
    bool w, a, s, d;
};

struct Player {
    s32 x;
    s32 y;
    bool alive;
    s32 life;
    bool exists;
    // std::unordered_map<std::string, bool> keyboard;
    // PlayerMouse mouse; // TODO: unused? SEE ALSO below: InitArrayWithSize(players,...)
    WASD wasd;
    s32 deathTimer;
};

struct PlayerMouse {
	s32 x;
	s32 y;
	// MouseButtonMap down;
	std::unordered_map<int, bool> down;
};


struct Bullet {
    s32 x;
    s32 y;
    f32 xf;
    f32 yf;
    s32 dx;
    s32 dy;
    bool alive;
    s32 life;
};

struct Color {
    u8 r;
    u8 g;
    u8 b;
    u8 a;
};

// struct Rect {
//     s32 x;
//     s32 y;
//     s32 w;
//     s32 h;
// };

typedef SDL_Rect Rect;


///////////



// Function declarations

f32 places(f32 num, s32 places);

SDL_GameController *findController();

bool init();

bool loadMedia();

void closeApp();

void mainLoop();

void update(s32 time);

void update_players(s32 dt);

// bool player_is_key_down(s32 id, std::string key);

void respawn_player(s32 id);

void update_player(s32 id, s32 dt);

void update_bullets(s32 dt);

void draw(s32 _time);

void draw_background();

void draw_players();

void draw_player(Player p, s32 id);

void draw_bullets();

void draw_bullet(Bullet b);

void SetFillStyle(Color color);

void ClearBackground(Color color);

void DrawRect(Rect r, Color c);

void DrawRectArgs(s32 x, s32 y, s32 w, s32 h, Color c);

void DrawRectArgsCam(s32 x, s32 y, s32 w, s32 h, Color color);

v2 GetCameraOffset(s32 x, s32 y);

void renderTextCam(s32 x, s32 y, std::string text, f32 alpha, s32 scale);

void renderText(s32 x, s32 y, std::string text, f32 alpha = 1.0, s32 scale = 1);

Color color_from_index(s32 idx);

void you_died();

void playSoundById(s32 snd_id);

extern "C" void net_get_bullet(s32 idx, s32 x, s32 y, s32 dx, s32 dy);

extern "C" void net_kill_bullet(s32 idx);

extern "C" void net_player_died(s32 idx);

extern "C" void net_delete_player(s32 plr_id);

void handleMessage(std::string msg);

void handleStateMessage(std::string msg);

void tts_you_died();

void resizeCanvas();
