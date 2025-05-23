#include "utils/font.h"

// 8x8 bitmap font для 'A' до 'Z'
const uint8_t font[131][8] = {
    // ASCII 0–31: control characters (empty)
    [0 ... 31] = {0},

    // ASCII 32: Space
    [32] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},

    // Symbols
    ['!'] = {0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x10, 0x00},
    ['.'] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00},
    [','] = {0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x10, 0x20},
    [':'] = {0x00, 0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00},
    [';'] = {0x00, 0x00, 0x18, 0x18, 0x00, 0x18, 0x10, 0x20},
    ['?'] = {0x3C, 0x42, 0x02, 0x0C, 0x10, 0x00, 0x10, 0x00},
    ['+'] = {0x00, 0x10, 0x10, 0x7C, 0x10, 0x10, 0x00, 0x00},
    ['-'] = {0x00, 0x00, 0x00, 0x7C, 0x00, 0x00, 0x00, 0x00},
    ['='] = {0x00, 0x7C, 0x00, 0x7C, 0x00, 0x00, 0x00, 0x00},
    ['*'] = {0x00, 0x24, 0x18, 0x7E, 0x18, 0x24, 0x00, 0x00},
    ['/'] = {0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x00, 0x00},
    ['%'] = {0x62, 0x64, 0x08, 0x10, 0x26, 0x46, 0x00, 0x00},

    ['|'] = {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00},
    ['_'] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0x00},
    ['\\'] = {0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x00, 0x00},
    ['('] = {0x08, 0x10, 0x20, 0x20, 0x20, 0x10, 0x08, 0x00},
    [')'] = {0x20, 0x10, 0x08, 0x08, 0x08, 0x10, 0x20, 0x00},
    ['<'] = {0x08, 0x10, 0x20, 0x40, 0x20, 0x10, 0x08, 0x00},
    ['>'] = {0x20, 0x10, 0x08, 0x04, 0x08, 0x10, 0x20, 0x00},

    // Digits
    ['0'] = {0x3C, 0x46, 0x4A, 0x52, 0x62, 0x46, 0x3C, 0x00},
    ['1'] = {0x10, 0x30, 0x10, 0x10, 0x10, 0x10, 0x7C, 0x00},
    ['2'] = {0x3C, 0x42, 0x02, 0x1C, 0x20, 0x40, 0x7E, 0x00},
    ['3'] = {0x3C, 0x42, 0x02, 0x1C, 0x02, 0x42, 0x3C, 0x00},
    ['4'] = {0x08, 0x18, 0x28, 0x48, 0x7E, 0x08, 0x08, 0x00},
    ['5'] = {0x7E, 0x40, 0x7C, 0x02, 0x02, 0x42, 0x3C, 0x00},
    ['6'] = {0x1C, 0x20, 0x40, 0x7C, 0x42, 0x42, 0x3C, 0x00},
    ['7'] = {0x7E, 0x02, 0x04, 0x08, 0x10, 0x10, 0x10, 0x00},
    ['8'] = {0x3C, 0x42, 0x42, 0x3C, 0x42, 0x42, 0x3C, 0x00},
    ['9'] = {0x3C, 0x42, 0x42, 0x3E, 0x02, 0x04, 0x38, 0x00},

    // Uppercase A-Z
    ['A'] = {0x18, 0x24, 0x42, 0x7E, 0x42, 0x42, 0x42, 0x00},
    ['B'] = {0x7C, 0x42, 0x42, 0x7C, 0x42, 0x42, 0x7C, 0x00},
    ['C'] = {0x3C, 0x42, 0x40, 0x40, 0x40, 0x42, 0x3C, 0x00},
    ['D'] = {0x78, 0x44, 0x42, 0x42, 0x42, 0x44, 0x78, 0x00},
    ['E'] = {0x7E, 0x40, 0x40, 0x7C, 0x40, 0x40, 0x7E, 0x00},
    ['F'] = {0x7E, 0x40, 0x40, 0x7C, 0x40, 0x40, 0x40, 0x00},
    ['G'] = {0x3C, 0x42, 0x40, 0x4E, 0x42, 0x42, 0x3C, 0x00},
    ['H'] = {0x42, 0x42, 0x42, 0x7E, 0x42, 0x42, 0x42, 0x00},
    ['I'] = {0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x00},
    ['J'] = {0x0E, 0x04, 0x04, 0x04, 0x44, 0x44, 0x38, 0x00},
    ['K'] = {0x42, 0x44, 0x48, 0x70, 0x48, 0x44, 0x42, 0x00},
    ['L'] = {0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x7E, 0x00},
    ['M'] = {0x42, 0x66, 0x5A, 0x5A, 0x42, 0x42, 0x42, 0x00},
    ['N'] = {0x42, 0x62, 0x52, 0x4A, 0x46, 0x42, 0x42, 0x00},
    ['O'] = {0x3C, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3C, 0x00},
    ['P'] = {0x7C, 0x42, 0x42, 0x7C, 0x40, 0x40, 0x40, 0x00},
    ['Q'] = {0x3C, 0x42, 0x42, 0x42, 0x52, 0x4C, 0x3A, 0x00},
    ['R'] = {0x7C, 0x42, 0x42, 0x7C, 0x48, 0x44, 0x42, 0x00},
    ['S'] = {0x3C, 0x40, 0x40, 0x3C, 0x02, 0x02, 0x7C, 0x00},
    ['T'] = {0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00},
    ['U'] = {0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3C, 0x00},
    ['V'] = {0x42, 0x42, 0x42, 0x42, 0x42, 0x24, 0x18, 0x00},
    ['W'] = {0x42, 0x42, 0x42, 0x5A, 0x5A, 0x66, 0x42, 0x00},
    ['X'] = {0x42, 0x24, 0x18, 0x18, 0x18, 0x24, 0x42, 0x00},
    ['Y'] = {0x42, 0x24, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00},
    ['Z'] = {0x7E, 0x04, 0x08, 0x10, 0x20, 0x40, 0x7E, 0x00},

    // Lowercase a-z
    ['a'] = {0x00, 0x00, 0x3C, 0x02, 0x3E, 0x42, 0x3E, 0x00},
    ['b'] = {0x40, 0x40, 0x5C, 0x62, 0x42, 0x62, 0x5C, 0x00},
    ['c'] = {0x00, 0x00, 0x3C, 0x40, 0x40, 0x40, 0x3C, 0x00},
    ['d'] = {0x02, 0x02, 0x3A, 0x46, 0x42, 0x46, 0x3A, 0x00},
    ['e'] = {0x00, 0x00, 0x3C, 0x42, 0x7E, 0x40, 0x3C, 0x00},
    ['f'] = {0x0C, 0x12, 0x10, 0x7C, 0x10, 0x10, 0x10, 0x00},
    ['g'] = {0x00, 0x00, 0x3A, 0x46, 0x46, 0x3A, 0x02, 0x3C},
    ['h'] = {0x40, 0x40, 0x5C, 0x62, 0x42, 0x42, 0x42, 0x00},
    ['i'] = {0x10, 0x00, 0x30, 0x10, 0x10, 0x10, 0x38, 0x00},
    ['j'] = {0x04, 0x00, 0x0C, 0x04, 0x04, 0x44, 0x44, 0x38},
    ['k'] = {0x40, 0x40, 0x48, 0x50, 0x60, 0x50, 0x48, 0x00},
    ['l'] = {0x30, 0x10, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00},
    ['m'] = {0x00, 0x00, 0x6C, 0x52, 0x52, 0x42, 0x42, 0x00},
    ['n'] = {0x00, 0x00, 0x5C, 0x62, 0x42, 0x42, 0x42, 0x00},
    ['o'] = {0x00, 0x00, 0x3C, 0x42, 0x42, 0x42, 0x3C, 0x00},
    ['p'] = {0x00, 0x00, 0x5C, 0x62, 0x62, 0x5C, 0x40, 0x40},
    ['q'] = {0x00, 0x00, 0x3A, 0x46, 0x46, 0x3A, 0x02, 0x02},
    ['r'] = {0x00, 0x00, 0x5C, 0x62, 0x40, 0x40, 0x40, 0x00},
    ['s'] = {0x00, 0x00, 0x3C, 0x40, 0x3C, 0x02, 0x7C, 0x00},
    ['t'] = {0x10, 0x10, 0x7C, 0x10, 0x10, 0x12, 0x0C, 0x00},
    ['u'] = {0x00, 0x00, 0x42, 0x42, 0x42, 0x46, 0x3A, 0x00},
    ['v'] = {0x00, 0x00, 0x42, 0x42, 0x24, 0x24, 0x18, 0x00},
    ['w'] = {0x00, 0x00, 0x42, 0x52, 0x52, 0x6C, 0x44, 0x00},
    ['x'] = {0x00, 0x00, 0x42, 0x24, 0x18, 0x24, 0x42, 0x00},
    ['y'] = {0x00, 0x00, 0x42, 0x42, 0x3E, 0x02, 0x3C, 0x00},
    ['z'] = {0x00, 0x00, 0x7E, 0x04, 0x18, 0x20, 0x7E, 0x00},

    // Remaining ASCII 127
    [127] = {0}};