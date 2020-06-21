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
    const size_t buffer_width = 256;
    const size_t buffer_height = 256;
    glfwSetErrorCallback(error_callback);
    if(!glfwInit()) return -1;

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window;
    window = glfwCreateWindow(480,480, "Space Invaders", NULL, NULL);

    if(!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    GLenum err = glewInit();
    if(err != GLEW_OK)
    {
        fprintf(stderr, "Error initializing glew!\n");
        glfwTerminate();
        return -1;
    }

    int glVersion[2] = {-1, -1};
    glGetIntegerv(GL_MAJOR_VERSION, &glVersion[0]);
    glGetIntegerv(GL_MINOR_VERSION, &glVersion[1]);

    gl_debug(__FILE__, __LINE__);

    printf("Using OpenGL: %d.%d\n", glVersion[0], glVersion[1]);
    printf("Renderer used: %s\n", glGetString(GL_RENDERER));
    printf("Shading Language: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
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
    glBindVertexArray(fullscreen_triangle_vao);

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
    fill_alien_shapes(alien_sprites, 6);

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

    Sprite alien_bullet_sprite[2];
    alien_bullet_sprite[0].width = alien_bullet_sprite[1].width = 3;
    alien_bullet_sprite[0].height = alien_bullet_sprite[1].height = 7;
    alien_bullet_sprite[0].data = alien_bullet_0;
    alien_bullet_sprite[1].data = alien_bullet_1;

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
    init_game(&game, buffer_width, buffer_height);

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

    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
