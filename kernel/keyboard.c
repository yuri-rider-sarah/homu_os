#include "types.h"
#include "keyboard.h"
#include "framebuffer.h"
#include "keycodes.h"

u8 keycodes[0x84] = {
     KEY_NONE,    KEY_F9,  KEY_NONE,    KEY_F5,    KEY_F3,    KEY_F1,    KEY_F2,   KEY_F12,
     KEY_NONE,   KEY_F10,    KEY_F8,    KEY_F6,    KEY_F4,   KEY_TAB, KEY_GRAVE,  KEY_NONE,
     KEY_NONE, KEY_L_ALT, KEY_LSHFT,  KEY_NONE, KEY_LCTRL,     KEY_Q,     KEY_1,  KEY_NONE,
     KEY_NONE,  KEY_NONE,     KEY_Z,     KEY_S,     KEY_A,     KEY_W,     KEY_2,  KEY_NONE,
     KEY_NONE,     KEY_C,     KEY_X,     KEY_D,     KEY_E,     KEY_4,     KEY_3,  KEY_NONE,
     KEY_NONE, KEY_SPACE,     KEY_V,     KEY_F,     KEY_T,     KEY_R,     KEY_5,  KEY_NONE,
     KEY_NONE,     KEY_N,     KEY_B,     KEY_H,     KEY_G,     KEY_Y,     KEY_6,  KEY_NONE,
     KEY_NONE,  KEY_NONE,     KEY_M,     KEY_J,     KEY_U,     KEY_7,     KEY_8,  KEY_NONE,
     KEY_NONE, KEY_COMMA,     KEY_K,     KEY_I,     KEY_O,     KEY_0,     KEY_9,  KEY_NONE,
     KEY_NONE,   KEY_DOT, KEY_SLASH,     KEY_L, KEY_SMCLN,     KEY_P, KEY_MINUS,  KEY_NONE,
     KEY_NONE,  KEY_NONE, KEY_APOST,  KEY_NONE, KEY_LBRCK, KEY_EQUAL,  KEY_NONE,  KEY_NONE,
    KEY_CPSLK, KEY_RSHFT, KEY_ENTER, KEY_RBRCK,  KEY_NONE, KEY_BKSLH,  KEY_NONE,  KEY_NONE,
     KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE, KEY_BKSPC,  KEY_NONE,
     KEY_NONE,  KEY_KP_1,  KEY_NONE,  KEY_KP_4,  KEY_KP_7,  KEY_NONE,  KEY_NONE,  KEY_NONE,
     KEY_KP_0, KEY_KPDOT,  KEY_KP_2,  KEY_KP_5,  KEY_KP_6,  KEY_KP_8,   KEY_ESC, KEY_NUMLK,
      KEY_F11, KEY_KPPLS,  KEY_KP_3, KEY_KPMIN, KEY_KPAST,  KEY_KP_9, KEY_SCRLK,  KEY_NONE,
     KEY_NONE,  KEY_NONE,  KEY_NONE,    KEY_F7,
};

u8 ext_keycodes[0x7E] = {
     KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,
     KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,
     KEY_NONE, KEY_R_ALT,  KEY_NONE,  KEY_NONE, KEY_RCTRL,  KEY_NONE,  KEY_NONE,  KEY_NONE,
     KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE, KEY_LMETA,
     KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE, KEY_RMETA,
     KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_MENU,
     KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,
     KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,
     KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,
     KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,
     KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,
     KEY_NONE,  KEY_NONE, KEY_KPENT,  KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,
     KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,  KEY_NONE,
     KEY_NONE,   KEY_END,  KEY_NONE,  KEY_LEFT,  KEY_HOME,  KEY_NONE,  KEY_NONE,  KEY_NONE,
      KEY_INS,   KEY_DEL,  KEY_DOWN,  KEY_NONE, KEY_RIGHT,    KEY_UP,  KEY_NONE,  KEY_NONE,
     KEY_NONE,  KEY_NONE,  KEY_PGDN,  KEY_NONE,  KEY_NONE,  KEY_PGUP,
};

enum KeyboardState {
    KBST_START,     // initial state
    KBST_BREAK,     // F0
    KBST_EXT,       // E0
    KBST_EXT_BREAK, // E0 F0
    KBST_PRTSC_2,
    KBST_PRTSC_3,
    KBST_PRTSC_BREAK_3,
    KBST_PRTSC_BREAK_4,
    KBST_PRTSC_BREAK_5,
    KBST_PAUSE_1,
    KBST_PAUSE_2,
    KBST_PAUSE_3,
    KBST_PAUSE_4,
    KBST_PAUSE_5,
    KBST_PAUSE_6,
    KBST_PAUSE_7,
};

static enum KeyboardState keyboard_state;

static u8 kb_buffer = KEY_NONE;

u8 kb_buffer_read(void) {
    asm volatile ("sti");
    while (kb_buffer == KEY_NONE) {
        asm volatile ("hlt" : : : "memory");
    }
    asm volatile ("cli");
    u8 key = kb_buffer;
    kb_buffer = KEY_NONE;
    return key;
}

static void kb_buffer_write(u8 code) {
    if (kb_buffer == KEY_NONE)
        kb_buffer = code;
}

__attribute__((interrupt)) void keyboard_irq_handler(void *frame) {
    u8 code;
    asm volatile ("in al, 0x60" : "=a"(code));
    switch (keyboard_state) {
    case KBST_START:
        switch (code) {
        case 0xF0:
            keyboard_state = KBST_BREAK;
            break;
        case 0xE0:
            keyboard_state = KBST_EXT;
            break;
        case 0xE1:
            keyboard_state = KBST_PAUSE_1;
            break;
        default:
            if (code < sizeof(keycodes) / sizeof(*keycodes) && keycodes[code] != KEY_NONE)
                kb_buffer_write(keycodes[code]);
            break;
        }
        break;
    case KBST_BREAK:
        if (code < sizeof(keycodes) / sizeof(*keycodes) && keycodes[code] != KEY_NONE)
            kb_buffer_write(KEY_RELEASED | keycodes[code]);
        keyboard_state = KBST_START;
        break;
    case KBST_EXT:
        switch (code) {
        case 0xF0:
            keyboard_state = KBST_EXT_BREAK;
            break;
        case 0x12:
            keyboard_state = KBST_PRTSC_2;
            break;
        default:
            if (code < sizeof(ext_keycodes) / sizeof(*ext_keycodes) && ext_keycodes[code] != KEY_NONE)
                kb_buffer_write(ext_keycodes[code]);
            keyboard_state = KBST_START;
            break;
        }
        break;
    case KBST_EXT_BREAK:
        if (code == 0x7C) {
            keyboard_state = KBST_PRTSC_BREAK_3;
            break;
        } else {
            if (code < sizeof(ext_keycodes) / sizeof(*ext_keycodes) && ext_keycodes[code] != KEY_NONE)
                kb_buffer_write(KEY_RELEASED | ext_keycodes[code]);
            keyboard_state = KBST_START;
        }
        break;
    case KBST_PRTSC_2:
        keyboard_state = code == 0xE0 ? KBST_PRTSC_3 : KBST_START;
        break;
    case KBST_PRTSC_3:
        if (code == 0x7C)
            kb_buffer_write(KEY_PRINT_SCREEN);
        keyboard_state = KBST_START;
        break;
    case KBST_PRTSC_BREAK_3:
        keyboard_state = code == 0xE0 ? KBST_PRTSC_BREAK_4 : KBST_START;
        break;
    case KBST_PRTSC_BREAK_4:
        keyboard_state = code == 0xF0 ? KBST_PRTSC_BREAK_5 : KBST_START;
        break;
    case KBST_PRTSC_BREAK_5:
        if (code == 0x12)
            kb_buffer_write(KEY_RELEASED | KEY_PRINT_SCREEN);
        keyboard_state = KBST_START;
        break;
    case KBST_PAUSE_1:
        keyboard_state = code == 0x14 ? KBST_PAUSE_2 : KBST_START;
        break;
    case KBST_PAUSE_2:
        keyboard_state = code == 0x77 ? KBST_PAUSE_3 : KBST_START;
        break;
    case KBST_PAUSE_3:
        keyboard_state = code == 0xE1 ? KBST_PAUSE_4 : KBST_START;
        break;
    case KBST_PAUSE_4:
        keyboard_state = code == 0xF0 ? KBST_PAUSE_5 : KBST_START;
        break;
    case KBST_PAUSE_5:
        keyboard_state = code == 0x14 ? KBST_PAUSE_6 : KBST_START;
        break;
    case KBST_PAUSE_6:
        keyboard_state = code == 0xF0 ? KBST_PAUSE_7 : KBST_START;
        break;
    case KBST_PAUSE_7:
        if (code == 0x77)
            kb_buffer_write(KEY_PAUSE);
        keyboard_state = KBST_START;
        break;
    }
    asm volatile ("out 0x20, al" : : "a"(0x20));
}
