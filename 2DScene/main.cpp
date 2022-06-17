#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include <fstream>

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"



enum Coordinate {
    x_coordinate, y_coordinate
};

#define LOG(argument) std::cout << argument << '\n'

const int WINDOW_WIDTH = 640,
WINDOW_HEIGHT = 480;

const float BG_RED = 0.1922f,
BG_BLUE = 0.549f,
BG_GREEN = 0.9059f,
BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

//const int TRIANGLE_RED = 1.0,
//          TRIANGLE_BLUE = 0.4,
//          TRIANGLE_GREEN = 0.4,
//          TRIANGLE_OPACITY = 1.0;

const float MILLISECONDS_IN_SECOND = 1000.0;
const float DEGREES_PER_SECOND = 90.0f;

const int NUMBER_OF_TEXTURES = 1; // to be generated, that is
const GLint LEVEL_OF_DETAIL = 0;  // base image level; Level n is the nth mipmap reduction image
const GLint TEXTURE_BORDER = 0;   // this value MUST be zero

const char PLAYER_SPRITE_FILEPATH[] = "player_blob.png";
const char ENEMY_SPRITE_FILEPATH[] = "enemy_blob.png";

SDL_Window* display_window;
bool game_is_running = true;
bool is_growing = true;

ShaderProgram program;
glm::mat4 view_matrix, player_model_matrix, projection_matrix, trans_matrix, enemy_model_matrix;

float triangle_x = 0.0f;
float triangle_rotate = 0.0f;
float previous_ticks = 0.0f;

GLuint player_texture_id;
GLuint enemy_texture_id;
glm::vec3 enemy_scale_vector;
glm::vec3 player_scale_vector;
SDL_Joystick* playerOneController;

glm::vec3 player_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 player_movement = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 enemy_position = glm::vec3(0.0f, 0.0f, 0.0f);

float get_screen_to_ortho(float coordinate, Coordinate axis) {
    switch (axis) {
    case x_coordinate:
        return ((coordinate / WINDOW_WIDTH) * 10.0) - (10.0f / 2.0);

    case y_coordinate:
        return (((WINDOW_HEIGHT - coordinate) / WINDOW_HEIGHT) * 7.5f) - (7.5f / 2.0);

    default:
        return 0.0;
    }
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

    // STEP 3: Setting our texture filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // STEP 4: Releasing our file from memory and returning our texture id
    stbi_image_free(image);

    return textureID;
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);

    playerOneController = SDL_JoystickOpen(0);

    display_window = SDL_CreateWindow("Preston Tang - 2D Scene - 6/14/2022",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(display_window);
    SDL_GL_MakeCurrent(display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    program.Load(V_SHADER_PATH, F_SHADER_PATH);

    player_model_matrix = glm::mat4(1.0f);
    player_scale_vector = glm::vec3(0.5f, 0.5f, 0.5f);
    player_model_matrix = glm::scale(player_model_matrix, player_scale_vector);

    enemy_model_matrix = glm::mat4(1.0f);
    enemy_scale_vector = glm::vec3(0.5f, 0.5f, 0.5f);
    enemy_model_matrix = glm::scale(enemy_model_matrix, enemy_scale_vector);
    enemy_position.x += 3.0f;
    enemy_model_matrix = glm::translate(enemy_model_matrix, enemy_position);

    view_matrix = glm::mat4(1.0f);  // Defines the position (location and orientation) of the camera
    projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);  // Defines the characteristics of your camera, such as clip planes, field of view, projection method etc.

    program.SetProjectionMatrix(projection_matrix);
    program.SetViewMatrix(view_matrix);
    // Notice we haven't set our model matrix yet!

//    program.SetColor(TRIANGLE_RED, TRIANGLE_BLUE, TRIANGLE_GREEN, TRIANGLE_OPACITY);

    glUseProgram(program.programID);

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    player_texture_id = load_texture(PLAYER_SPRITE_FILEPATH);
    enemy_texture_id = load_texture(ENEMY_SPRITE_FILEPATH);

    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input() {

    player_movement = glm::vec3(0.0f);

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {

        case SDL_WINDOWEVENT_CLOSE:
        case SDL_QUIT:
            game_is_running = false;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_w:

                break;

            case SDLK_a:
                
                break;

            case SDLK_s:

                break;

            case SDLK_d:

                break;
            }

        default:
            break;

        }
    }

    const Uint8* key_states = SDL_GetKeyboardState(NULL);

    if (key_states[SDL_SCANCODE_A]) {
        player_movement.x = -1.0f;
    } else if (key_states[SDL_SCANCODE_D]) {
        player_movement.x = 1.0f;
    }
    if (key_states[SDL_SCANCODE_W]) {
        player_movement.y = 1.0f;
    } else if (key_states[SDL_SCANCODE_S]) {
        player_movement.y = -1.0f;
    }

    if (glm::length(player_movement) > 1.0f) {
        player_movement = glm::normalize(player_movement);
    }

}

void update() {
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND; // get the current number of ticks
    float delta_time = ticks - previous_ticks; // the delta time is the difference from the last frame
    previous_ticks = ticks;

    triangle_x += 1.0f * delta_time;
    triangle_rotate += 500 * delta_time; // 90-degrees per second

    player_model_matrix = glm::mat4(1.0f);
    enemy_model_matrix = glm::mat4(1.0f);

    // For some reason normalize returns NaN? Does it have something to do with negatives? I'm not sure :(
    glm::vec3 normalized_direction = player_position - enemy_position;
    glm::vec3 enemy_velocity = normalized_direction * 0.5f * delta_time;

    enemy_position += enemy_velocity;

    std::ofstream outfile;

    outfile.open("output.txt", std::ios_base::app);
    outfile << normalized_direction.x << " " << normalized_direction.y;

    enemy_model_matrix = glm::translate(enemy_model_matrix, enemy_position);
    enemy_model_matrix = glm::scale(enemy_model_matrix, enemy_scale_vector);
    enemy_model_matrix = glm::rotate(enemy_model_matrix,
        std::atan2(normalized_direction.y, normalized_direction.x) + glm::radians(270.0f),glm::vec3(0.0f, 0.0f, 1.0f));

    /* Translate -> Rotate */
    player_position += player_movement * delta_time * 3.0f;

    player_model_matrix = glm::mat4(1.0f);
    player_model_matrix = glm::translate(player_model_matrix, player_position);
    player_model_matrix = glm::scale(player_model_matrix, player_scale_vector);
    player_model_matrix = glm::rotate(player_model_matrix, glm::radians(triangle_rotate), glm::vec3(0.0f, 0.0f, 1.0f));

    float x_distance = fabs(player_position.x - enemy_position.x) - ((0.5f + 0.5f) / 2.0f);
    float y_distance = fabs(player_position.y - enemy_position.y) - ((0.5f + 0.5f) / 2.0f);

    if (x_distance < 0 && y_distance < 0) {
        // Teleport player outside of screen (idk how to delete stuff)
        player_position.x = 999;
        player_position.y = 999;
    }
}

void draw_object(glm::mat4& object_model_matrix, GLuint& object_texture_id)
{
    program.SetModelMatrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so we use 6 instead of 3
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Vertices
    float player_vertices[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
    };

    // Textures
    float player_texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
    };

    // Vertices
    float enemy_vertices[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
    };

    // Textures
    float enemy_texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
    };

    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, player_vertices);
    glEnableVertexAttribArray(program.positionAttribute);

    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, player_texture_coordinates);
    glEnableVertexAttribArray(program.texCoordAttribute);

    // Bind texture
    draw_object(player_model_matrix, player_texture_id);

    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, enemy_vertices);
    glEnableVertexAttribArray(program.positionAttribute);

    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, enemy_texture_coordinates);
    glEnableVertexAttribArray(program.texCoordAttribute);

    // Bind texture
    draw_object(enemy_model_matrix, enemy_texture_id);

    // We disable two attribute arrays now
    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);

    SDL_GL_SwapWindow(display_window);
}

void shutdown() {
    SDL_JoystickClose(playerOneController);
    SDL_Quit();
}

/**
 Start here—we can see the general structure of a game loop without worrying too much about the details yet.
 */
int main(int argc, char* argv[])
{
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

