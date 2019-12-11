#pragma once

typedef enum Keycode {
    KEY_NONE,
    KEY_ESCAPE,
    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12,
    KEY_GRAVE,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,
    KEY_0,
    KEY_MINUS,
    KEY_EQUAL,
    KEY_BACKSPACE,
    KEY_TAB,
    KEY_Q,
    KEY_W,
    KEY_E,
    KEY_R,
    KEY_T,
    KEY_Y,
    KEY_U,
    KEY_I,
    KEY_O,
    KEY_P,
    KEY_LEFT_BRACKET,
    KEY_RIGHT_BRACKET,
    KEY_BACKSLASH,
    KEY_CAPS_LOCK,
    KEY_A,
    KEY_S,
    KEY_D,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_SEMICOLON,
    KEY_APOSTROPHE,
    KEY_ENTER,
    KEY_LEFT_SHIFT,
    KEY_Z,
    KEY_X,
    KEY_C,
    KEY_V,
    KEY_B,
    KEY_N,
    KEY_M,
    KEY_COMMA,
    KEY_PERIOD,
    KEY_SLASH,
    KEY_RIGHT_SHIFT,
    KEY_LEFT_CTRL,
    KEY_LEFT_META,
    KEY_LEFT_ALT,
    KEY_SPACE,
    KEY_RIGHT_ALT,
    KEY_RIGHT_META,
    KEY_MENU,
    KEY_RIGHT_CTRL,
    KEY_PRINT_SCREEN,
    KEY_SCROLL_LOCK,
    KEY_PAUSE,
    KEY_INSERT,
    KEY_DELETE,
    KEY_HOME,
    KEY_END,
    KEY_PAGE_UP,
    KEY_PAGE_DOWN,
    KEY_UP,
    KEY_LEFT,
    KEY_DOWN,
    KEY_RIGHT,
    KEY_NUM_LOCK,
    KEY_KP_SLASH,
    KEY_KP_ASTERISK,
    KEY_KP_MINUS,
    KEY_KP_PLUS,
    KEY_KP_ENTER,
    KEY_KP_PERIOD,
    KEY_KP_0,
    KEY_KP_1,
    KEY_KP_2,
    KEY_KP_3,
    KEY_KP_4,
    KEY_KP_5,
    KEY_KP_6,
    KEY_KP_7,
    KEY_KP_8,
    KEY_KP_9,
} Keycode;

#define KEY_RELEASED 0x80

// Alternative abbreviated keycode names
#define KEY_ESC   KEY_ESCAPE
#define KEY_BKSPC KEY_BACKSPACE
#define KEY_LBRCK KEY_LEFT_BRACKET
#define KEY_RBRCK KEY_RIGHT_BRACKET
#define KEY_BKSLH KEY_BACKSLASH
#define KEY_CPSLK KEY_CAPS_LOCK
#define KEY_SMCLN KEY_SEMICOLON
#define KEY_APOST KEY_APOSTROPHE
#define KEY_LSHFT KEY_LEFT_SHIFT
#define KEY_DOT   KEY_PERIOD
#define KEY_RSHFT KEY_RIGHT_SHIFT
#define KEY_LCTRL KEY_LEFT_CTRL
#define KEY_LMETA KEY_LEFT_META
#define KEY_L_ALT KEY_LEFT_ALT
#define KEY_R_ALT KEY_RIGHT_ALT
#define KEY_RMETA KEY_RIGHT_META
#define KEY_RCTRL KEY_RIGHT_CTRL
#define KEY_PRTSC KEY_PRINT_SCREEN
#define KEY_SCRLK KEY_SCROLL_LOCK
#define KEY_INS   KEY_INSERT
#define KEY_DEL   KEY_DELETE
#define KEY_PGUP  KEY_PAGE_UP
#define KEY_PGDN  KEY_PAGE_DOWN
#define KEY_NUMLK KEY_NUM_LOCK
#define KEY_KPSLH KEY_KP_SLASH
#define KEY_KPAST KEY_KP_ASTERISK
#define KEY_KPMIN KEY_KP_MINUS
#define KEY_KPPLS KEY_KP_PLUS
#define KEY_KPENT KEY_KP_ENTER
#define KEY_KPDOT KEY_KP_PERIOD