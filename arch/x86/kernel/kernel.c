#include "utils/framebuffer.h"
#include "utils/font.h"
#include "cli.h"

void kernel_main(framebuffer_info_t *fb)
{
    cli(fb);
    while (1)
        ;
}
