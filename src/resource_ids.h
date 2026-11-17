#pragma once

// Bitmaps (RT_BITMAP)
enum BitmapId {
    BMP_PLAYER        = 103,  // 0x67  320x464 player sprite sheet
    BMP_BALL          = 104,  // 0x68  200x80  ball + effects
    BMP_COURT         = 106,  // 0x6A  432x112 court tiles / landscape
    BMP_MEGA          = 113,  // 0x71  160x160 title burst
    BMP_SHADOW        = 114,  // 0x72  32x8    shadow
    BMP_POKEMON_LOGO  = 115,  // 0x73  200x32  "포켓몬"
    BMP_TITLE_LOGO    = 119,  // 0x77  276x79  "피카츄 배구"
    BMP_PIKA_FACE     = 120,  // 0x78  104x104 title background
    BMP_TITLE_ART     = 121,  // 0x79  88x110  title art
    BMP_GAME_START    = 122,  // 0x7A  96x24
    BMP_DIGITS        = 123,  // 0x7B  320x32  digits 0..9
    BMP_READY         = 124,  // 0x7C  80x24
    BMP_CONTROLS      = 125,  // 0x7D  120x56  control instructions
    BMP_COPYRIGHT     = 126,  // 0x7E  360x20
    BMP_GAME_SET      = 131,  // 0x83  96x24
    BMP_BLACK_PIXEL   = 133,  // 0x85  1x1
};

// WAVE resources
enum SoundId {
    WAV_WIN       = 140,  // 0x8C
    WAV_PIKA      = 141,  // 0x8D
    WAV_CHU       = 142,  // 0x8E
    WAV_TITLE_2   = 143,  // 0x8F  (loaded, not referenced by code)
    WAV_TITLE_1   = 144,  // 0x90
    WAV_BALL_HIT  = 145,  // 0x91
    WAV_BALL_GROUND = 146 // 0x92
};

// Menu command IDs
enum MenuId {
    IDM_QUIT       = 0x9C41,
    IDM_INFO       = 0x9C42,
    IDM_RESET      = 0x9C43,
    IDM_SPEED_SLOW = 0x9C44,
    IDM_SPEED_NORM = 0x9C45,
    IDM_SPEED_FAST = 0x9C46,
    IDM_DEBUG      = 0x9C47,
    IDM_SCORE_5    = 0x9C49,
    IDM_SCORE_10   = 0x9C4A,
    IDM_SCORE_15   = 0x9C4B,
    IDM_RESTORE    = 0x9C4C,
    IDM_PAUSE      = 0x9C4D,
};

// Resource names
#define IDI_PIKAICON  MAKEINTRESOURCEA(1)
#define IDR_MENU1     MAKEINTRESOURCEA(4)
#define IDD_INFO      117
