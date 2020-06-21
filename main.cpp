#include "struct.h"
#include "validate.h"

using namespace std;

bool game_running = false;
int move_dir = 0;
bool fire_pressed = 0;

#define GL_ERROR_CASE(glerror)\
    case glerror: snprintf(error, sizeof(error), "%s", #glerror)

inline void gl_debug(const char* file, int line)
{
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR)
    {
        char error[256];
        switch(err) {
            GL_ERROR_CASE(GL_INVALID_ENUM); break;
            GL_ERROR_CASE(GL_INVALID_VALUE); break;
            GL_ERROR_CASE(GL_INVALID_OPERATION); break;
            GL_ERROR_CASE(GL_INVALID_FRAMEBUFFER_OPERATION); break;
            GL_ERROR_CASE(GL_OUT_OF_MEMORY); break;
            default: snprintf(error, sizeof(error), "%s", "UNKNOWN_ERROR"); break;
        }

        fprintf(stderr, "%s - %s: %d\n", error, file, line);
    }
}

#undef GL_ERROR_CASE

enum AlienType: uint8_t
{
    ALIEN_DEAD   = 0,
    ALIEN_TYPE_A = 1,
    ALIEN_TYPE_B = 2,
    ALIEN_TYPE_C = 3
};

void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
    switch(key){
    case GLFW_KEY_ESCAPE:
        if(action == GLFW_PRESS) game_running = false;
        break;
    case GLFW_KEY_RIGHT:
        if(action == GLFW_PRESS) move_dir += 1;
        else if(action == GLFW_RELEASE) move_dir -= 1;
        break;
    case GLFW_KEY_LEFT:
        if(action == GLFW_PRESS) move_dir -= 1;
        else if(action == GLFW_RELEASE) move_dir += 1;
        break;
    case GLFW_KEY_SPACE:
        if(action == GLFW_RELEASE) fire_pressed = true;
        break;
    default:
        break;
    }
}



int main(int argc, char** argv)
{   
    const size_t buffer_width = 224;
    const size_t buffer_height = 256;

    glfwSetErrorCallback(error_callback);
    if(!glfwInit()) return -1;

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window;
    window = glfwCreateWindow(2 * buffer_width, 2 * buffer_height, "Space Invaders", NULL, NULL);

    if(!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwSetKeyCallback(window, key_callback);
    glfwMakeContextCurrent(window);

    GLenum err = glewInit();
    if(err != GLEW_OK)
    {
        fprintf(stderr, "Error initializing glew!\n");
        glfwTerminate();
        return -1;
    }

    int glVersion[2] = {-1, 1};
    glGetIntegerv(GL_MAJOR_VERSION, &glVersion[0]);
    glGetIntegerv(GL_MINOR_VERSION, &glVersion[1]);

    gl_debug(__FILE__, __LINE__);

    printf("Using OpenGL: %d.%d\n", glVersion[0], glVersion[1]);
    printf("Renderer used: %s\n", glGetString(GL_RENDERER));
    printf("Shading Language: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    glfwSwapInterval(1);
    glClearColor(1.0, 0.0, 0.0,1.0);

    Buffer buffer;
    buffer.width = buffer_width;
    buffer.height = buffer_height;
    buffer.data = new uint32_t[buffer_height * buffer_width];
    buffer_clear(&buffer, 0);

    GLuint buffer_texture;
    glGenTextures(1, &buffer_texture);
    glBindTexture(GL_TEXTURE_2D, buffer_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, buffer.width, buffer.height, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, buffer.data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GLuint fullscreen_triangle_vao;
    glGenVertexArrays(1, &fullscreen_triangle_vao);
    //glBindVertexArray(fullscreen_triangle_vao);

    GLuint shader_id = glCreateProgram();

    GLuint shader_vp = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(shader_vp, 1, &vertex_shader, 0);
    glCompileShader(shader_vp);
    validate_shader(shader_vp, vertex_shader);
    glAttachShader(shader_id, shader_vp);
    glDeleteShader(shader_vp);

    GLuint shader_fp = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(shader_fp, 1, &fragment_shader, 0);
    glCompileShader(shader_fp);
    validate_shader(shader_fp, fragment_shader);
    glAttachShader(shader_id, shader_fp);
    glDeleteShader(shader_fp);

    glLinkProgram(shader_id);
    if(!validate_program(shader_id))
    {
        fprintf(stderr, "Error while validationg program: %d\n", shader_id);
        glfwTerminate();
        glDeleteVertexArrays(1, &fullscreen_triangle_vao);
        delete[] buffer.data;
        return -1;
    }

    glUseProgram(shader_id);

    GLint location = glGetUniformLocation(shader_id, "buffer");
    glUniform1i(location, 0);

    //OpenGL setup
    glDisable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);

    glBindVertexArray(fullscreen_triangle_vao);

    Sprite alien_sprites[6];
    fill_alien_shapes(&alien_sprites[0], 6);

    Sprite alien_death_sprite;
    alien_death_sprite.width = 13;
    alien_death_sprite.height = 7;
    alien_death_sprite.data = alien_death;

    Sprite player_sprite;
    player_sprite.width = 11;
    player_sprite.height = 7;
    player_sprite.data = player_shape;

    Sprite text_spritesheet;
    text_spritesheet.width = 5;
    text_spritesheet.height = 7;
    text_spritesheet.data = text_spreadsheet_shape;

    Sprite number_spritesheet = text_spritesheet;
    number_spritesheet.data += 16 * 35;

    Sprite player_bullet_sprite;
    player_bullet_sprite.width = 1;
    player_bullet_sprite.height = 3;
    player_bullet_sprite.data = player_bullet;

    Sprite alien_bullet_sprite[2];
    alien_bullet_sprite[0].width = alien_bullet_sprite[1].width = 3;
    alien_bullet_sprite[0].height = alien_bullet_sprite[1].height = 7;
    alien_bullet_sprite[0].data = alien_bullet_0;
    alien_bullet_sprite[1].data = alien_bullet_1;

    SpriteAnimation alien_bullet_animation;
    alien_bullet_animation.loop = true;
    alien_bullet_animation.num_frames = 2;
    alien_bullet_animation.frame_duration = 5;
    alien_bullet_animation.time = 0;

    alien_bullet_animation.frames = new Sprite*[2];
    alien_bullet_animation.frames[0] = &alien_bullet_sprite[0];
    alien_bullet_animation.frames[1] = &alien_bullet_sprite[1];

    SpriteAnimation alien_animation[3];

    size_t alien_update_frequency = 120;

    for(size_t i = 0; i < 3; ++i)
    {
        alien_animation[i].loop = true;
        alien_animation[i].num_frames = 2;
        alien_animation[i].frame_duration = alien_update_frequency;
        alien_animation[i].time = 0;

        alien_animation[i].frames = new Sprite*[2];
        alien_animation[i].frames[0] = &alien_sprites[2 * i];
        alien_animation[i].frames[1] = &alien_sprites[2 * i + 1];
    }

    Game game;
    game = init_game(buffer_width, buffer_height);

    size_t alien_swarm_position = 24;
    size_t alien_swarm_max_position = game.width - 16 * 11 - 3;

    size_t aliens_killed = 0;
    size_t alien_update_timer = 0;
    bool should_change_speed = false;

    for(size_t xi = 0; xi < 11; ++xi)
    {
        for(size_t yi = 0; yi < 5; ++yi)
        {
            Alien& alien = game.aliens[xi * 5 + yi];
            alien.type = (5 - yi) / 2 + 1;

            const Sprite& sprite = alien_sprites[2 * (alien.type - 1)];

            alien.x = 16 * xi + alien_swarm_position + (alien_death_sprite.width - sprite.width)/2;
            alien.y = 17 * yi + 128;
        }
    }

    uint8_t* death_counters = new uint8_t[game.num_aliens];
    std::fill(death_counters, death_counters + game.num_aliens, 10);

    uint32_t clear_color = rgb_to_uint32(0, 128, 0);
    uint32_t rng = 13;

    int alien_move_dir = 4;
    int player_move_dir = 0;

    size_t score = 0;
    size_t credits = 0;
    game_running = true;

    while (!glfwWindowShouldClose(window) && game_running)
    {  
        buffer_clear(&buffer, clear_color);

        cout << "here 1" << endl;

        if(game.player.life == 0)
        {
            buffer_draw_text(&buffer, text_spritesheet, "GAME OVER", game.width / 2 - 30, game.height / 2, rgb_to_uint32(128, 0, 0));
            buffer_draw_text(&buffer, text_spritesheet, "SCORE", 4, game.height - text_spritesheet.height - 7, rgb_to_uint32(128, 0, 0));
            buffer_draw_number(&buffer, number_spritesheet, score, 4 + 2 * number_spritesheet.width, game.height - 2 * number_spritesheet.height - 12, rgb_to_uint32(128, 0, 0));

            glTexSubImage2D(
                GL_TEXTURE_2D, 0, 0, 0,
                buffer.width, buffer.height,
                GL_RGBA, GL_UNSIGNED_INT_8_8_8_8,
                buffer.data
            );
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

            glfwSwapBuffers(window);
            glfwPollEvents();
            continue;
        }

        cout << "here 2" << endl;

        buffer_draw_text(&buffer, text_spritesheet, "SCORE", 4, game.height - text_spritesheet.height - 7, rgb_to_uint32(128, 0, 0));
        buffer_draw_number(&buffer, number_spritesheet, score, 4 + 2 * number_spritesheet.width, game.height - 2 * number_spritesheet.height - 12, rgb_to_uint32(128, 0, 0));

        char credit_text[16];
        sprintf(credit_text, "CREDIT %02lu", credits);
        buffer_draw_text(&buffer, text_spritesheet, credit_text, 164, 7, rgb_to_uint32(128, 0, 0));

        buffer_draw_number(&buffer, number_spritesheet, game.player.life, 4, 7, rgb_to_uint32(128, 0, 0));
        size_t xp =  11 + number_spritesheet.width;

        for(size_t i = 0; i < game.player.life - 1; ++i)
        {
            buffer_draw_sprite(&buffer, player_sprite, xp, 7, rgb_to_uint32(128, 0, 0));
            xp += player_sprite.width + 2;
        }

        for(size_t i = 0; i < game.width; ++i)
        {
            buffer.data[game.width * 16 + i] = rgb_to_uint32(128, 0, 0);
        }

        cout << "here 3" << endl;

        for(size_t ai = 0; ai < game.num_aliens; ++ai)
        {
            if(death_counters[ai] == 0) continue;

            const Alien& alien = game.aliens[ai];
            if(alien.type == ALIEN_DEAD)
            {   
                buffer_draw_sprite(&buffer, alien_death_sprite, alien.x, alien.y, rgb_to_uint32(128, 0, 0));
            }
            else
            {
                const SpriteAnimation& animation = alien_animation[alien.type - 1];
                size_t current_frame = animation.time / animation.frame_duration;
                const Sprite& sprite = *animation.frames[current_frame];
                buffer_draw_sprite(&buffer, sprite, alien.x, alien.y, rgb_to_uint32(128, 0, 0));
            }
        }

        cout << "here 4" << endl;

        for(size_t bi = 0; bi < game.num_bullets; ++bi)
        {
            const Bullet& bullet = game.bullets[bi];
            const Sprite* sprite;
            if(bullet.dir > 0) sprite = &player_bullet_sprite;
            else
            {
                size_t cf = alien_bullet_animation.time / alien_bullet_animation.frame_duration;
                sprite = &alien_bullet_sprite[cf];
            }
            buffer_draw_sprite(&buffer, *sprite, bullet.x, bullet.y, rgb_to_uint32(128, 0, 0));
        }

        cout << "here 5" << endl;

        buffer_draw_sprite(&buffer, player_sprite, game.player.x, game.player.y, rgb_to_uint32(128, 0, 0));

        glTexSubImage2D(
            GL_TEXTURE_2D, 0, 0, 0,
            buffer.width, buffer.height,
            GL_RGBA, GL_UNSIGNED_INT_8_8_8_8,
            buffer.data
        );
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glfwSwapBuffers(window);

        cout << "here 6" << endl;

        // Simulate bullets
        for(size_t bi = 0; bi < game.num_bullets; ++bi)
        {
            game.bullets[bi].y += game.bullets[bi].dir;
            if(game.bullets[bi].y >= game.height || game.bullets[bi].y < player_bullet_sprite.height)
            {
                game.bullets[bi] = game.bullets[game.num_bullets - 1];
                --game.num_bullets;
                continue;
            }

            // Alien bullet
            if(game.bullets[bi].dir < 0)
            {
                bool overlap = sprite_overlap_check(
                    alien_bullet_sprite[0], game.bullets[bi].x, game.bullets[bi].y,
                    player_sprite, game.player.x, game.player.y
                );

                if(overlap)
                {
                    --game.player.life;
                    game.bullets[bi] = game.bullets[game.num_bullets - 1];
                    --game.num_bullets;
                    break;
                }
            }

            // Player bullet
            else
            {
                // Check if player bullet hits an alien bullet
                for(size_t bj = 0; bj < game.num_bullets; ++bj)
                {
                    if(bi == bj) continue;

                    bool overlap = sprite_overlap_check(
                        player_bullet_sprite, game.bullets[bi].x, game.bullets[bi].y,
                        alien_bullet_sprite[0], game.bullets[bj].x, game.bullets[bj].y
                    );

                    if(overlap)
                    {
                        // NOTE: Make sure it works.
                        if(bj == game.num_bullets - 1)
                        {
                            game.bullets[bi] = game.bullets[game.num_bullets - 2];
                        }
                        else if(bi == game.num_bullets - 1)
                        {
                            game.bullets[bj] = game.bullets[game.num_bullets - 2];
                        }
                        else
                        {
                            game.bullets[(bi < bj)? bi: bj] = game.bullets[game.num_bullets - 1];
                            game.bullets[(bi < bj)? bj: bi] = game.bullets[game.num_bullets - 2];
                        }
                        game.num_bullets -= 2;
                        break;
                    }
                }

                // Check hit
                for(size_t ai = 0; ai < game.num_aliens; ++ai)
                {
                    const Alien& alien = game.aliens[ai];
                    if(alien.type == ALIEN_DEAD) continue;

                    const SpriteAnimation& animation = alien_animation[alien.type - 1];
                    size_t current_frame = animation.time / animation.frame_duration;
                    const Sprite& alien_sprite = *animation.frames[current_frame];
                    bool overlap = sprite_overlap_check(
                        player_bullet_sprite, game.bullets[bi].x, game.bullets[bi].y,
                        alien_sprite, alien.x, alien.y
                    );

                    if(overlap)
                    {
                        score += 10 * (4 - game.aliens[ai].type);
                        game.aliens[ai].type = ALIEN_DEAD;
                        game.aliens[ai].x -= (alien_death_sprite.width - alien_sprite.width)/2;
                        game.bullets[bi] = game.bullets[game.num_bullets - 1];
                        --game.num_bullets;
                        ++aliens_killed;

                        if(aliens_killed % 15 == 0) should_change_speed = true;

                        break;
                    }
                }
            }
        }

        // Simulate aliens
        if(should_change_speed)
        {
            should_change_speed = false;
            alien_update_frequency /= 2;
            for(size_t i = 0; i < 3; ++i)
            {
                alien_animation[i].frame_duration = alien_update_frequency;
            }
        }

        // Update death counters
        for(size_t ai = 0; ai < game.num_aliens; ++ai)
        {
            const Alien& alien = game.aliens[ai];
            if(alien.type == ALIEN_DEAD && death_counters[ai])
            {
                --death_counters[ai];
            }
        }

        if(alien_update_timer >= alien_update_frequency)
        {
            alien_update_timer = 0;

            if((int)alien_swarm_position + alien_move_dir < 0)
            {
                alien_move_dir *= -1;
                //TODO: Perhaps if aliens get close enough to player, we need to check
                //for overlap. What happens when alien moves over line y = 0 line?
                for(size_t ai = 0; ai < game.num_aliens; ++ai)
                {
                    Alien& alien = game.aliens[ai];
                    alien.y -= 8;
                }
            }
            else if(alien_swarm_position > alien_swarm_max_position - alien_move_dir)
            {
                alien_move_dir *= -1;
            }
            alien_swarm_position += alien_move_dir;

            for(size_t ai = 0; ai < game.num_aliens; ++ai)
            {
                Alien& alien = game.aliens[ai];
                alien.x += alien_move_dir;
            }

            if(aliens_killed < game.num_aliens)
            {
                size_t rai = game.num_aliens * random(&rng);
                while(game.aliens[rai].type == ALIEN_DEAD)
                {
                    rai = game.num_aliens * random(&rng);
                }
                const Sprite& alien_sprite = *alien_animation[game.aliens[rai].type - 1].frames[0];
                game.bullets[game.num_bullets].x = game.aliens[rai].x + alien_sprite.width / 2;
                game.bullets[game.num_bullets].y = game.aliens[rai].y - alien_bullet_sprite[0].height;
                game.bullets[game.num_bullets].dir = -2;
                ++game.num_bullets;
            }
        }

        // Update animations
        for(size_t i = 0; i < 3; ++i)
        {
            ++alien_animation[i].time;
            if(alien_animation[i].time >= alien_animation[i].num_frames * alien_animation[i].frame_duration)
            {
                alien_animation[i].time = 0;
            }
        }
        ++alien_bullet_animation.time;
        if(alien_bullet_animation.time >= alien_bullet_animation.num_frames * alien_bullet_animation.frame_duration)
        {
            alien_bullet_animation.time = 0;
        }

        ++alien_update_timer;

        // Simulate player
        player_move_dir = 2 * move_dir;

        if(player_move_dir != 0)
        {
            if(game.player.x + player_sprite.width + player_move_dir >= game.width)
            {
                game.player.x = game.width - player_sprite.width;
            }
            else if((int)game.player.x + player_move_dir <= 0)
            {
                game.player.x = 0;
            }
            else game.player.x += player_move_dir;
        }

        if(aliens_killed < game.num_aliens)
        {
            size_t ai = 0;
            while(game.aliens[ai].type == ALIEN_DEAD) ++ai;
            const Sprite& sprite = alien_sprites[2 * (game.aliens[ai].type - 1)];
            size_t pos = game.aliens[ai].x - (alien_death_sprite.width - sprite.width)/2;
            if(pos > alien_swarm_position) alien_swarm_position = pos;

            ai = game.num_aliens - 1;
            while(game.aliens[ai].type == ALIEN_DEAD) --ai;
            pos = game.width - game.aliens[ai].x - 13 + pos;
            if(pos > alien_swarm_max_position) alien_swarm_max_position = pos;
        }
        else
        {
            alien_update_frequency = 120;
            alien_swarm_position = 24;

            aliens_killed = 0;
            alien_update_timer = 0;

            alien_move_dir = 4;

            for(size_t xi = 0; xi < 11; ++xi)
            {
                for(size_t yi = 0; yi < 5; ++yi)
                {
                    size_t ai = xi * 5 + yi;

                    death_counters[ai] = 10;

                    Alien& alien = game.aliens[ai];
                    alien.type = (5 - yi) / 2 + 1;

                    const Sprite& sprite = alien_sprites[2 * (alien.type - 1)];

                    alien.x = 16 * xi + alien_swarm_position + (alien_death_sprite.width - sprite.width)/2;
                    alien.y = 17 * yi + 128;
                }
            }
        }

        // Process events
        if(fire_pressed && game.num_bullets < GAME_MAX_BULLETS)
        {
            game.bullets[game.num_bullets].x = game.player.x + player_sprite.width / 2;
            game.bullets[game.num_bullets].y = game.player.y + player_sprite.height;
            game.bullets[game.num_bullets].dir = 2;
            ++game.num_bullets;
        }
        fire_pressed = false;

        glfwPollEvents();

    }

    glfwDestroyWindow(window);
    glfwTerminate();

    glDeleteVertexArrays(1, &fullscreen_triangle_vao);

    for(size_t i = 0; i < 6; ++i)
    {
        delete[] alien_sprites[i].data;
    }

    delete[] text_spritesheet.data;
    delete[] alien_death_sprite.data;
    delete[] player_bullet_sprite.data;
    delete[] alien_bullet_sprite[0].data;
    delete[] alien_bullet_sprite[1].data;
    delete[] alien_bullet_animation.frames;

    for(size_t i = 0; i < 3; ++i)
    {
        delete[] alien_animation[i].frames;
    }
    delete[] buffer.data;
    delete[] game.aliens;
    delete[] death_counters;

    return 0;
}
