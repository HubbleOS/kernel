#include "framebuffer.h"
#include "font.h"
#include "cli.h"

void kernel_main(framebuffer_info_t *fb)
{

    uint32_t *pixels = (uint32_t *)fb->base;
    for (int i = 0; i < (fb->pitch / 4) * fb->height; ++i)
    {
        pixels[i] = 0x000000;
    }

    cli(fb);
    while (1)
        ;
}
