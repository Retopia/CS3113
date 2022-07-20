#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define PLATFORM_COUNT 15
#define GRAVITY 0.4f

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "cmath"
#include <ctime>
#include <vector>
#include <SDL_mixer.h>
#include "Entity.h"
#include <source_location>


/**
 STRUCTS AND ENUMS
 */
struct GameState
{
    Entity* player;
    Entity* platforms;
    // Entity* background;
};

/**
 CONSTANTS
 */
const int WINDOW_WIDTH = 640,
WINDOW_HEIGHT = 480;

const float BG_RED = 255/255.0f,
BG_BLUE = 111.0f/255,
BG_GREEN = 97.0f/255,
BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl";
const char F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const float MILLISECONDS_IN_SECOND = 1000.0;
const char SPRITESHEET_FILEPATH[] = "assets/lander_spritesheet.png";
const char PLATFORM_FILEPATH[] = "assets/surface_green.png";
const char GOAL_FILEPATH[] = "assets/goal.png";
const char BGM_FILEPATH[] = "assets/space.png";
const char FONT_FILEPATH[] = "assets/font1.png";

const int NUMBER_OF_TEXTURES = 1; // to be generated, that is
const GLint LEVEL_OF_DETAIL = 0;  // base image level; Level n is the nth mipmap reduction image
const GLint TEXTURE_BORDER = 0;   // this value MUST be zero

const int FONTBANK_SIZE = 16;

/**
 VARIABLES
 */
GameState state;

SDL_Window* display_window;
bool game_is_running = true;

ShaderProgram program;
glm::mat4 view_matrix, projection_matrix;

float previous_ticks = 0.0f;
float accumulator = 0.0f;
Mix_Music* bgm;

/**
 GENERAL FUNCTIONS
 */
int random(int from, int to) {
    return rand() % (to - from + 1) + from;
}

GLuint load_texture(const char* filepath)
{
    // STEP 1: Loading the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    // STEP 2: Generating and binding a texture ID to our image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    // STEP 3: Setting our texture filter modes
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // STEP 4: Setting our texture wrapping modes
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // the last argument can change depending on what you are looking for
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // STEP 5: Releasing our file from memory and returning our texture id
    stbi_image_free(image);

    return textureID;
}

void draw_text(ShaderProgram* program, GLuint font_texture_id, std::string text, float screen_size, float spacing, glm::vec3 position) {
    // Scale the size of the fontbank in the UV-plane
    // We will use this for spacing and positioning
    float width = 1.0f / FONTBANK_SIZE;
    float height = 1.0f / FONTBANK_SIZE;

    // Instead of having a single pair of arrays, we'll have a series of pairs—one for each character
    // Don't forget to include <vector>!
    std::vector<float> vertices;
    std::vector<float> texture_coordinates;

    // For every character...
    for (int i = 0; i < text.size(); i++) {
        // 1. Get their index in the spritesheet, as well as their offset (i.e. their position
        //    relative to the whole sentence)
        int spritesheet_index = (int)text[i];  // ascii value of character
        float offset = (screen_size + spacing) * i;

        // 2. Using the spritesheet index, we can calculate our U- and V-coordinates
        float u_coordinate = (float)(spritesheet_index % FONTBANK_SIZE) / FONTBANK_SIZE;
        float v_coordinate = (float)(spritesheet_index / FONTBANK_SIZE) / FONTBANK_SIZE;

        // 3. Inset the current pair in both vectors
        vertices.insert(vertices.end(), {
            offset + (-0.5f * screen_size), 0.5f * screen_size,
            offset + (-0.5f * screen_size), -0.5f * screen_size,
            offset + (0.5f * screen_size), 0.5f * screen_size,
            offset + (0.5f * screen_size), -0.5f * screen_size,
            offset + (0.5f * screen_size), 0.5f * screen_size,
            offset + (-0.5f * screen_size), -0.5f * screen_size,
            });

        texture_coordinates.insert(texture_coordinates.end(), {
            u_coordinate, v_coordinate,
            u_coordinate, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate + width, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate, v_coordinate + height,
            });
    }

    // 4. And render all of them using the pairs
    glm::mat4 model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, position);

    program->SetModelMatrix(model_matrix);
    glUseProgram(program->programID);

    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices.data());
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates.data());
    glEnableVertexAttribArray(program->texCoordAttribute);

    glBindTexture(GL_TEXTURE_2D, font_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, (int)(text.size() * 6));

    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    display_window = SDL_CreateWindow("Preston Tang - Lunar Lander - 7/17/2022",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(display_window);
    SDL_GL_MakeCurrent(display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    bgm = Mix_LoadMUS("assets/mochi.mp3");
    Mix_PlayMusic(bgm, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME);

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    program.Load(V_SHADER_PATH, F_SHADER_PATH);

    view_matrix = glm::mat4(1.0f);  // Defines the position (location and orientation) of the camera
    projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);  // Defines the characteristics of your camera, such as clip planes, field of view, projection method etc.

    program.SetProjectionMatrix(projection_matrix);
    program.SetViewMatrix(view_matrix);

    glUseProgram(program.programID);

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    //state.background = new Entity();
    //state.background->texture_id = load_texture(BGM_FILEPATH);;
    //state.background->set_position(glm::vec3(0.0f, 0.0f, 0.0f));
    //state.background->set_height(5.0f);
    //state.background->set_width(5.0f);
    //state.background->update(0.0f, NULL, 0);

    state.player = new Entity();
    state.player->set_position(glm::vec3(-3.5f, 3.0f, 0.0f));
    state.player->set_movement(glm::vec3(0.0f));
    state.player->speed = 1.0f;
    state.player->set_acceleration(glm::vec3(0.0f, -GRAVITY, 0.0f));
    state.player->texture_id = load_texture(SPRITESHEET_FILEPATH);
    state.player->jumping_power = 1.5f;

    state.player->walking[state.player->LEFT] = new int[4]{ 1, 5, 9, 13 };
    state.player->walking[state.player->RIGHT] = new int[4]{ 3, 7, 11, 15 };
    state.player->walking[state.player->UP] = new int[4]{ 2, 6, 10, 14 };
    state.player->walking[state.player->DOWN] = new int[4]{ 0, 4, 8, 12 };

    state.player->animation_indices = state.player->walking[state.player->LEFT];  // start George looking left
    state.player->animation_frames = 4;
    state.player->animation_index = 2;
    state.player->animation_time = 0.0f;
    state.player->animation_cols = 3;
    state.player->animation_rows = 1;
    state.player->is_dangerous = false;
    state.player->is_goal = false;

    GLuint platform_texture_id = load_texture(PLATFORM_FILEPATH);

    state.platforms = new Entity[PLATFORM_COUNT];

    for (int i = 0; i < PLATFORM_COUNT - 5; i++) {
        state.platforms[i].animation_indices = state.player->walking[state.player->LEFT];
        state.platforms[i].texture_id = platform_texture_id;
        state.platforms[i].set_position(glm::vec3(i - 4.5f, -3.3f, 0.0f));
        state.platforms[i].update(0.0f, NULL, 0);
        state.platforms[i].animation_cols = 18;
        state.platforms[i].animation_rows = 1;
        state.platforms[i].animation_index = 1;
        state.platforms[i].is_dangerous = true;
        state.platforms[i].is_goal = false;
    }

    state.platforms[PLATFORM_COUNT - 1].texture_id = load_texture(GOAL_FILEPATH);;
    state.platforms[PLATFORM_COUNT - 1].set_position(glm::vec3(3.0f, 1.0f, 0.0f));
    state.platforms[PLATFORM_COUNT - 1].update(0.0f, NULL, 0);
    state.platforms[PLATFORM_COUNT - 1].is_dangerous = false;
    state.platforms[PLATFORM_COUNT - 1].is_goal = true;

    state.platforms[PLATFORM_COUNT - 4].animation_indices = state.player->walking[state.player->LEFT];
    state.platforms[PLATFORM_COUNT - 4].texture_id = platform_texture_id;
    state.platforms[PLATFORM_COUNT - 4].set_position(glm::vec3(0.0f, 0.0f, 0.0f));
    state.platforms[PLATFORM_COUNT - 4].update(0.0f, NULL, 0);
    state.platforms[PLATFORM_COUNT - 4].animation_cols = 18;
    state.platforms[PLATFORM_COUNT - 4].animation_rows = 1;
    state.platforms[PLATFORM_COUNT - 4].animation_index = 1;
    state.platforms[PLATFORM_COUNT - 4].is_dangerous = true;
    state.platforms[PLATFORM_COUNT - 4].is_goal = false;

    state.platforms[PLATFORM_COUNT - 3].animation_indices = state.player->walking[state.player->LEFT];
    state.platforms[PLATFORM_COUNT - 3].texture_id = platform_texture_id;
    state.platforms[PLATFORM_COUNT - 3].set_position(glm::vec3(0.0f, 1.0f, 0.0f));
    state.platforms[PLATFORM_COUNT - 3].update(0.0f, NULL, 0);
    state.platforms[PLATFORM_COUNT - 3].animation_cols = 18;
    state.platforms[PLATFORM_COUNT - 3].animation_rows = 1;
    state.platforms[PLATFORM_COUNT - 3].animation_index = 1;
    state.platforms[PLATFORM_COUNT - 3].is_dangerous = true;
    state.platforms[PLATFORM_COUNT - 3].is_goal = false;

    state.platforms[PLATFORM_COUNT - 2].animation_indices = state.player->walking[state.player->LEFT];
    state.platforms[PLATFORM_COUNT - 2].texture_id = platform_texture_id;
    state.platforms[PLATFORM_COUNT - 2].set_position(glm::vec3(0.0f, 2.0f, 0.0f));
    state.platforms[PLATFORM_COUNT - 2].update(0.0f, NULL, 0);
    state.platforms[PLATFORM_COUNT - 2].animation_cols = 18;
    state.platforms[PLATFORM_COUNT - 2].animation_rows = 1;
    state.platforms[PLATFORM_COUNT - 2].animation_index = 1;
    state.platforms[PLATFORM_COUNT - 2].is_dangerous = true;
    state.platforms[PLATFORM_COUNT - 2].is_goal = false;

    state.platforms[PLATFORM_COUNT - 5].animation_indices = state.player->walking[state.player->LEFT];
    state.platforms[PLATFORM_COUNT - 5].texture_id = platform_texture_id;
    state.platforms[PLATFORM_COUNT - 5].set_position(glm::vec3(0.0f, -1.0f, 0.0f));
    state.platforms[PLATFORM_COUNT - 5].update(0.0f, NULL, 0);
    state.platforms[PLATFORM_COUNT - 5].animation_cols = 18;
    state.platforms[PLATFORM_COUNT - 5].animation_rows = 1;
    state.platforms[PLATFORM_COUNT - 5].animation_index = 1;
    state.platforms[PLATFORM_COUNT - 5].is_dangerous = true;
    state.platforms[PLATFORM_COUNT - 5].is_goal = false;

    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    // VERY IMPORTANT: If nothing is pressed, we don't want to go anywhere
    state.player->set_movement(glm::vec3(0.0f));

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            // End game
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            game_is_running = false;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_q:
                // Quit the game with a keystroke
                game_is_running = false;
                break;

            case SDLK_w:
                if (state.player->can_jump) {
                    state.player->is_jumping = true;
                }
                state.player->can_jump = false;
                break;
            default:
                break;
            }

        case SDL_KEYUP:
            switch (event.key.keysym.sym) {
            case SDLK_w:
                state.player->can_jump = true;
                break;

            default:
                break;
            }

        default:
            break;
        }
    }

    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_LEFT])
    {
        state.player->movement.x = -1.0f;
    }
    else if (key_state[SDL_SCANCODE_RIGHT])
    {
        state.player->movement.x = 1.0f;
    }

    if (key_state[SDL_SCANCODE_A])
    {
        state.player->movement.x = -1.0f;
    }
    else if (key_state[SDL_SCANCODE_D])
    {
        state.player->movement.x = 1.0f;
    }

        // This makes sure that the player can't move faster diagonally
    if (glm::length(state.player->movement) > 1.0f)
    {
        state.player->movement = glm::normalize(state.player->movement);
    }
}

void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - previous_ticks;
    previous_ticks = ticks;

    delta_time += accumulator;

    if (delta_time < FIXED_TIMESTEP)
    {
        accumulator = delta_time;
        return;
    }

    while (delta_time >= FIXED_TIMESTEP) {
        // Update. Notice it's FIXED_TIMESTEP. Not deltaTime
        state.player->update(FIXED_TIMESTEP, state.platforms, PLATFORM_COUNT);
        state.player->update(FIXED_TIMESTEP, state.platforms, PLATFORM_COUNT);
        delta_time -= FIXED_TIMESTEP;
    }

    accumulator = delta_time;
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT);

    state.player->render(&program);

    for (int i = 0; i < PLATFORM_COUNT; i++) state.platforms[i].render(&program);

    if (!state.player->is_active) {
        if (state.player->won) {
            draw_text(&program, load_texture(FONT_FILEPATH), "You've won!", 0.58f, 0.001f, glm::vec3(-3.0f, 0.0f, 0.0f));
        }
        else {
            draw_text(&program, load_texture(FONT_FILEPATH), "You've lost!", 0.58f, 0.001f, glm::vec3(-3.0f, 0.0f, 0.0f));
        }
    }

    SDL_GL_SwapWindow(display_window);
}

void shutdown()
{
    SDL_Quit();
    Mix_FreeMusic(bgm);
    delete[] state.platforms;
    delete state.player;
}

/**
 DRIVER GAME LOOP
 */
int main(int argc, char* argv[]) {
    initialise();

    while (game_is_running)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}
