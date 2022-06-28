#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include <fstream>

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#define PI 3.14159265359
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"

class Rectoid {
public:
    Rectoid() {}

    Rectoid(glm::mat4 model_matrix, float x, float y, float scale_x, float scale_y, float rotationAngles, GLuint texture_id, float speed)
        : model_matrix(model_matrix), texture_id(texture_id), speed(speed) {
        rotation = rotationAngles;
        position = glm::vec3(x, y, 0.0f);
        movement = glm::vec3(0.0f, 0.0f, 0.0f);
        scale_vector = glm::vec3(scale_x, scale_y, 0.0f);
        model_matrix = glm::translate(model_matrix, position);
        model_matrix = glm::scale(model_matrix, scale_vector);
        model_matrix = glm::rotate(model_matrix, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    }

    void render(ShaderProgram program) {
        // Vertices
        float vertices[] = {
            -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
            -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
        };

        // Textures
        float texture_coords[] = {
            0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
            0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
        };

        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program.positionAttribute);

        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coords);
        glEnableVertexAttribArray(program.texCoordAttribute);

        // Bind texture
        program.SetModelMatrix(model_matrix);
        glBindTexture(GL_TEXTURE_2D, texture_id);
        glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so we use 6 instead of 3
    }

    void setScale(glm::vec3 scale) {
        scale_vector = scale;
    }

    void setSpeed(float speed) {
        this->speed = speed;
    }

    void setRotation(float rotationAngles) {
        rotation += rotationAngles;
    }

    void setMovement(glm::vec3 direction) {
        if (glm::length(direction) > 1.0f) {
            glm::normalize(direction);
        }
        movement = direction;
    }

    void update(float delta_time) {
        model_matrix = glm::mat4(1.0f);

        movement *= speed * delta_time;

        position += movement;
        model_matrix = glm::translate(model_matrix, position);
        model_matrix = glm::scale(model_matrix, scale_vector);
        model_matrix = glm::rotate(model_matrix, glm::radians(rotation) * delta_time, glm::vec3(0.0f, 0.0f, 1.0f));

        movement = glm::vec3(0.0f, 0.0f, 0.0f);
    }

    glm::vec3 getPosition() const {
        return position;
    }

    glm::vec3 getMovement() const {
        return movement;
    }

    glm::vec3 getScaleVector() const {
        return position;
    }

private:
    glm::mat4 model_matrix;
    glm::vec3 position, movement, scale_vector;
    float speed, rotation;
    GLuint texture_id;
};

#define LOG(argument) std::cout << argument << '\n'

const int WINDOW_WIDTH = 640,
WINDOW_HEIGHT = 480;

const float BG_RED = 0.1f,
BG_BLUE = 0.1f,
BG_GREEN = 0.1f,
BG_OPACITY = 0.5f;

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

const char player1_sprite[] = "images/player_blob.png";
const char player2_sprite[] = "images/enemy_blob.png";

SDL_Window* display_window;
bool game_is_running = true;

ShaderProgram program;
glm::mat4 view_matrix, projection_matrix, trans_matrix;
float previous_ticks = 0.0f;

GLuint player_texture_id;
GLuint enemy_texture_id;

Rectoid player1, player2, ball;

glm::vec3 prevBallMovement;
bool bouncedOffPlayer1 = true;

GLuint load_texture(const char* filepath) {
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_image_free(image);

    return textureID;
}

void initialise() {
    SDL_Init(SDL_INIT_VIDEO);

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

    player1 = Rectoid(glm::mat4(1.0f), -4.7f, 0.0f, 0.25f, 2.0f, 0.0f, load_texture(player1_sprite), 3.0f);
    player2 = Rectoid(glm::mat4(1.0f), 4.7f, 0.0f, 0.25f, 2.0f, 0.0f, load_texture(player2_sprite), 3.0f);
    ball = Rectoid(glm::mat4(1.0f), 0.0f, 0.0f, 0.25f, 0.25f, 0.0f, load_texture(player2_sprite), 4.0f);

    ball.setMovement(glm::vec3(1.0f, 0.5f, 0.0f));
    prevBallMovement = ball.getMovement();

    view_matrix = glm::mat4(1.0f);  // Defines the position (location and orientation) of the camera
    projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);  // Defines the characteristics of your camera, such as clip planes, field of view, projection method etc.

    program.SetProjectionMatrix(projection_matrix);
    program.SetViewMatrix(view_matrix);
    
    glUseProgram(program.programID);

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {

        case SDL_WINDOWEVENT_CLOSE:
        case SDL_QUIT:
            game_is_running = false;
            break;

        default:
            break;

        }
    }

    const Uint8* key_states = SDL_GetKeyboardState(NULL);
    
    // For some reason I need to use such a small number?
    if (key_states[SDL_SCANCODE_W]) {
        player1.setMovement(glm::vec3(0.0f, 1.0f, 0.0f));
    } else if (key_states[SDL_SCANCODE_S]) {
        player1.setMovement(glm::vec3(0.0f, -1.0f, 0.0f));
    }

    if (key_states[SDL_SCANCODE_UP]) {
        player2.setMovement(glm::vec3(0.0f, 1.0f, 0.0f));
    }
    else if (key_states[SDL_SCANCODE_DOWN]) {
        player2.setMovement(glm::vec3(0.0f, -1.0f, 0.0f));
    }
}

void update() {
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND; // get the current number of ticks
    float delta_time = ticks - previous_ticks; // the delta time is the difference from the last frame
    previous_ticks = ticks;

    //ball.setRotation(10000);
    ball.setMovement(prevBallMovement);

    // Paddle bounds checking
    if (player1.getPosition().y > 2.75) {
        player1.setMovement(glm::vec3(0.0f, -1.0f, 0.0f));
    }

    if (player1.getPosition().y < -2.75) {
        player1.setMovement(glm::vec3(0.0f, 1.0f, 0.0f));
    }

    if (player2.getPosition().y > 2.75) {
        player2.setMovement(glm::vec3(0.0f, -1.0f, 0.0f));
    }

    if (player2.getPosition().y < -2.75) {
        player2.setMovement(glm::vec3(0.0f, 1.0f, 0.0f));
    }

    ball.setSpeed(4.0f);

    // Ball collision checking
    if (ball.getPosition().y > 3.6) {
        glm::vec3 direction = ball.getMovement();
        float radians = atan2(direction.y, direction.x);
        ball.setMovement(glm::vec3(cos(-radians), sin(-radians), 0.0f));
        ball.setSpeed(8.0f);
        prevBallMovement = ball.getMovement();
    }

    if (ball.getPosition().y < -3.6) {
        glm::vec3 direction = ball.getMovement();
        float radians = atan2(direction.y, direction.x);
        ball.setMovement(glm::vec3(cos(-radians), sin(-radians), 0.0f));
        ball.setSpeed(8.0f);
        prevBallMovement = ball.getMovement();
    }

    float x_distance1 = fabs(ball.getPosition().x - player1.getPosition().x) - ((0.1) / 2.0f);
    float y_distance1 = fabs(ball.getPosition().y - player1.getPosition().y) - ((0.25 + 1.75) / 2.0f);
    float x_distance2 = fabs(ball.getPosition().x - player2.getPosition().x) - ((0.1) / 2.0f);
    float y_distance2 = fabs(ball.getPosition().y - player2.getPosition().y) - ((0.25 + 1.75) / 2.0f);

    if (x_distance1 < 0 && y_distance1 < 0 && !bouncedOffPlayer1) {
        //std::cout << "bounced1" << std::endl;
        glm::vec3 direction = ball.getMovement();
        float radians = atan2(direction.y, direction.x);
        float result = radians;

        if (radians < 0) {
            radians += 2 * PI;
        }

        if (radians > PI) {
            result = -(radians - PI);
        } else {
            result = (PI - radians);
        }

        ball.setMovement(glm::vec3(cos(result), sin(result), 0.0f));
        //std::cout << radians * 180/PI << " to " << result * 180 / PI << std::endl;
        prevBallMovement = ball.getMovement();
        bouncedOffPlayer1 = true;
    }

    if (x_distance2 < 0 && y_distance2 < 0 && bouncedOffPlayer1) {
        //std::cout << "bounced2" << std::endl;
        glm::vec3 direction = ball.getMovement();
        float radians = atan2(direction.y, direction.x);
        float result = radians;
        if (radians < 0) {
            radians += 2 * PI;
        }

        if (radians > PI) {
            result = 2 * PI - radians + PI;
        } else {
            result = PI - radians;
        }

        ball.setMovement(glm::vec3(cos(result), sin(result), 0.0f));
        //std::cout << radians * 180 / PI << " to " << result * 180 / PI << std::endl;
        prevBallMovement = ball.getMovement();
        bouncedOffPlayer1 = false;
    }

    if (ball.getPosition().x > 5.0) {
        // End game
        game_is_running = false;
    }

    if (ball.getPosition().x < -5.0) {
        // End game
        game_is_running = false;
    }

    player1.update(delta_time);
    player2.update(delta_time);
    ball.update(delta_time);
}


void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    player1.render(program);
    player2.render(program);
    ball.render(program);

    SDL_GL_SwapWindow(display_window);
}

void shutdown() {
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    initialise();

    while (game_is_running) {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}