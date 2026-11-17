// -----------------------------------------------------------------------------
//  Pikachu Volleyball - C++ source port
//
//  Based on the 1997 game "対戦ぴかちゅ～！ ﾋﾞｰﾁﾊﾞﾚｰ編(ベータ版)"
//  (C) 1997 SACHI SOFT / SAWAYAKAN Programmers
//  (C) 1997 Satoshi Takenouchi
// -----------------------------------------------------------------------------
#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cstdarg>

// ---------------------------------------------------------------------------
// Court / screen constants
// ---------------------------------------------------------------------------
constexpr int GROUND_WIDTH  = 432;   // 0x1B0
constexpr int GROUND_HEIGHT = 304;   // 0x130
constexpr int GROUND_HALF_WIDTH = GROUND_WIDTH / 2;  // 216, 0xD8

constexpr int PLAYER_LENGTH      = 64;
constexpr int PLAYER_HALF_LENGTH = 32;
constexpr int PLAYER_GROUND_Y    = 244;   // 0xF4

constexpr int BALL_RADIUS        = 20;
constexpr int BALL_GROUND_Y      = 252;   // 0xFC

constexpr int NET_HALF_WIDTH          = 25;
constexpr int NET_TOP_TOP_Y           = 176;  // 0xB0
constexpr int NET_TOP_BOTTOM_Y        = 192;  // 0xC0

constexpr int SCREEN_WIDTH  = 432;
constexpr int SCREEN_HEIGHT = 304;

// ---------------------------------------------------------------------------
// Debug logging (enabled only with /DPIKA_DEBUG)
// ---------------------------------------------------------------------------
#ifdef PIKA_DEBUG
inline void pika_log(const char* fmt, ...)
{
    FILE* f = fopen("pika_log.txt", "a");
    if (!f) return;
    va_list ap; va_start(ap, fmt);
    vfprintf(f, fmt, ap);
    va_end(ap);
    fputc('\n', f);
    fclose(f);
}
#else
#define pika_log(...) ((void)0)
#endif

// ---------------------------------------------------------------------------
// Tiny helpers
// ---------------------------------------------------------------------------
inline int iabs(int v) { return v < 0 ? -v : v; }
inline int isign(int v) { return (v > 0) - (v < 0); }
inline bool pika_contains(const char* text, const char* needle)
{
    if (!needle || !*needle) return true;
    for (; *text; text++) {
        const char* t = text;
        const char* n = needle;
        while (*t && *n && *t == *n) { t++; n++; }
        if (!*n) return true;
    }
    return false;
}

// Deterministic rand(): seed = seed*214013 + 2531011; return (seed>>16)&0x7FFF
// Used for AI boldness, power-hit search order and the "vx == 0" tie
// breaker, so the AI behaves the same on every run.
inline unsigned int g_randSeed = 1;
inline void pika_srand(unsigned int seed) { g_randSeed = seed; }
inline int pika_rand()
{
    g_randSeed = g_randSeed * 214013u + 2531011u;
    return (int)((g_randSeed >> 16) & 0x7FFF);
}
inline int rand_range(int n) { return n > 0 ? pika_rand() % n : 0; }

// Decoded per-frame input for one player (array of 8 ints).
struct CUserInput {
    int x = 0;       // -1 left, 0 none, +1 right
    int y = 0;       // -1 up,   0 none, +1 down
    int power = 0;   // 1 = power hit / dive
    int extra = 0;   // bit 0x20, unused by the player code
};
