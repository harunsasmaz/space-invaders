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

typedef struct bullet
{
    size_t x, y;
    int dir;
}Bullet;

typedef struct Game{
    size_t width, height;
    int num_aliens;
    int num_bullets;
    Alien* aliens;
    Player player;
    Bullet bullets[GAME_MAX_BULLETS];
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

void init_game(Game* game, size_t width, size_t height)
{
    game->width = width;
    game->height = height;
    game->num_bullets = 0;
    game->num_aliens = 55;
    game->aliens = new Alien[55];

    game->player.x = 112 - 5;
    game->player.y = 32;
    game->player.life = 3;
}

bool sprite_overlap_check(
    const Sprite& sp_a, size_t x_a, size_t y_a,
    const Sprite& sp_b, size_t x_b, size_t y_b
)
{
    if(x_a < x_b + sp_b.width && x_a + sp_a.width > x_b &&
       y_a < y_b + sp_b.height && y_a + sp_a.height > y_b)
    {
        return true;
    }

    return false;
}

void buffer_draw_sprite(Buffer* buffer, const Sprite& sprite, size_t x, size_t y, uint32_t color)
{
    for(size_t xi = 0; xi < sprite.width; ++xi)
    {
        for(size_t yi = 0; yi < sprite.height; ++yi)
        {
            if(sprite.data[yi * sprite.width + xi] &&
               (sprite.height - 1 + y - yi) < buffer->height &&
               (x + xi) < buffer->width)
            {
                buffer->data[(sprite.height - 1 + y - yi) * buffer->width + (x + xi)] = color;
            }
        }
    }
}

void buffer_draw_number(
    Buffer* buffer,
    const Sprite& number_spritesheet, size_t number,
    size_t x, size_t y,
    uint32_t color)
{
    uint8_t digits[64];
    size_t num_digits = 0;

    size_t current_number = number;
    do
    {
        digits[num_digits++] = current_number % 10;
        current_number = current_number / 10;
    }
    while(current_number > 0);

    size_t xp = x;
    size_t stride = number_spritesheet.width * number_spritesheet.height;
    Sprite sprite = number_spritesheet;
    for(size_t i = 0; i < num_digits; ++i)
    {
        uint8_t digit = digits[num_digits - i - 1];
        sprite.data = number_spritesheet.data + digit * stride;
        buffer_draw_sprite(buffer, sprite, xp, y, color);
        xp += sprite.width + 1;
    }
}

void buffer_draw_text(
    Buffer* buffer,
    const Sprite& text_spritesheet,
    const char* text,
    size_t x, size_t y,
    uint32_t color)
{
    size_t xp = x;
    size_t stride = text_spritesheet.width * text_spritesheet.height;
    Sprite sprite = text_spritesheet;
    for(const char* charp = text; *charp != '\0'; ++charp)
    {
        char character = *charp - 32;
        if(character < 0 || character >= 65) continue;

        sprite.data = text_spritesheet.data + character * stride;
        buffer_draw_sprite(buffer, sprite, xp, y, color);
        xp += sprite.width + 1;
    }
}
