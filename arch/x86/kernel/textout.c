#include "font.h"
#include "framebuffer.h"

void draw_char(framebuffer_info_t* fb, char c, int x, int y, uint32_t color) {
    if (c < 'A' || c > 'Z') return;

    const uint8_t* glyph = font_AZ[c - 'A'];
    int pitch = fb->pitch / 4;

    for (int row = 0; row < CHAR_HEIGHT; ++row) {
        uint8_t line = glyph[row];
        for (int col = 0; col < CHAR_WIDTH; ++col) {
            if (line & (0x80 >> col)) {
                int px = x + col;
                int py = y + row;
                ((uint32_t*)fb->base)[py * pitch + px] = color;
            }
        }
    }
}

void print_text(framebuffer_info_t* fb, const char* str, int x, int y, uint32_t color) {
    while (*str) {
        if (*str >= 'A' && *str <= 'Z') {
            draw_char(fb, *str, x, y, color);
            x += CHAR_WIDTH;
        } else if (*str == ' ') {
            x += CHAR_WIDTH;
        }
        ++str;
    }
}
