#include "util.h"

typedef struct buffer{
    size_t width, height;
    uint32_t* data;
}Buffer;

typedef struct sprite{
    size_t width, height;
    uint8_t* data;
}Sprite;

void buffer_clear(Buffer* buffer, uint32_t color)
{
    size_t bound = buffer->width * buffer->height;
    for(size_t i = 0; i < bound; ++i)
    {
        buffer->data[i] = color;
    }
}

void buffer_sprite_draw(Buffer* buffer, const Sprite& sprite,
    size_t x, size_t y, uint32_t color)
{
    for(size_t xi = 0; xi < sprite.width; ++xi)
    {
        for(size_t yi = 0; yi < sprite.height; ++yi)
        {
            size_t sy = sprite.height - 1 + y - yi;
            size_t sx = x + xi;
            if(sprite.data[yi * sprite.width + xi] &&
               sy < buffer->height && sx < buffer->width) 
            {
                buffer->data[sy * buffer->width + sx] = color;
            }
        }
    }
}