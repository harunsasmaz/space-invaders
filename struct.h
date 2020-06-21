#include "util.h"
#include "shapes.h"

typedef struct buffer{
    size_t width, height;
    uint32_t* data;
}Buffer;

typedef struct sprite{
    size_t width, height;
    uint8_t* data;
}Sprite;

typedef struct alien{
    size_t x,y;
    uint8_t type;
}Alien;

typedef struct player{
    size_t x,y;
    size_t life;
}Player;

typedef struct game{
    size_t width, height;
    int num_aliens;
    Alien* aliens;
    Player player;
}Game;

typedef struct spriteAnimation
{
    bool loop;
    size_t num_frames;
    size_t frame_duration;
    size_t time;
    Sprite** frames;
}SpriteAnimation;

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

void fill_alien_shapes(Sprite* sprites, int n)
{
    sprites[0].height = 8;
    sprites[0].width = 8;
    sprites->data = alien_shape_0;

    sprites[1].height = 8;
    sprites[1].width = 8;
    sprites->data = alien_shape_1;

    sprites[2].height = 8;
    sprites[2].width = 11;
    sprites->data = alien_shape_2;

    sprites[3].height = 8;
    sprites[3].width = 11;
    sprites->data = alien_shape_3;

    sprites[4].height = 8;
    sprites[4].width = 12;
    sprites->data = alien_shape_4;

    sprites[5].height = 8;
    sprites[5].width = 12;
    sprites->data = alien_shape_5;
}