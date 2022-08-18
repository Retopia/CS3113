#include "World.h"
#include "Utility.h"

#define LEVEL_WIDTH 30
#define LEVEL_HEIGHT 8
#define LOG(argument) std::cout << argument << '\n'

const int FONTBANK_SIZE = 16;

unsigned int WORLD_DATA[] =
{
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2
};

World::~World()
{
    delete [] this->state.enemies;
    delete    this->state.player;
    delete    this->state.map;
    Mix_FreeChunk(this->state.jump_sfx);
    Mix_FreeMusic(this->state.bgm);
}

void World::initialise()
{
    state.next_scene_id = -1;
    
    GLuint map_texture_id = Utility::load_texture("assets/tileset.png");
    this->state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, WORLD_DATA, map_texture_id, 1.0f, 4, 1);
    
    // Code from main.cpp's initialise()
    /**
     George's Stuff
     */
    // Existing
    state.player = new Entity();
    state.player->set_entity_type(PLAYER);
    state.player->set_position(glm::vec3(3.0f, 1.0f, 0.0f));
    state.player->set_movement(glm::vec3(0.0f));
    state.player->speed = 2.5f;
    state.player->set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    state.player->texture_id = Utility::load_texture("assets/marnie_0.png");
    
    // Walking
    state.player->walking[state.player->LEFT]  = new int[4] { 1, 5, 9,  13 };
    state.player->walking[state.player->RIGHT] = new int[4] { 3, 7, 11, 15 };
    state.player->walking[state.player->UP]    = new int[4] { 2, 6, 10, 14 };
    state.player->walking[state.player->DOWN]  = new int[4] { 0, 4, 8,  12 };

    state.player->animation_indices = state.player->walking[state.player->RIGHT];  // start George looking left
    state.player->animation_frames = 4;
    state.player->animation_index  = 0;
    state.player->animation_time   = 0.0f;
    state.player->animation_cols   = 4;
    state.player->animation_rows   = 4;
    state.player->set_height(0.8f);
    state.player->set_width(0.8f);
    
    // Jumping
    state.player->jumping_power = 5.0f;
    
    /**
     Enemies' stuff */
    GLuint enemy1_texture_id = Utility::load_texture("assets/trainer3.png");
    GLuint enemy1_texture_id2 = Utility::load_texture("assets/trainer3_flip.png");
    GLuint enemy2_texture_id = Utility::load_texture("assets/trainer1.png");
    GLuint enemy3_texture_id = Utility::load_texture("assets/trainer2.png");
    
    state.enemies = new Entity[this->ENEMY_COUNT];
    state.enemies[0].set_entity_type(ENEMY);
    state.enemies[0].set_ai_type(STANDER);
    state.enemies[0].set_ai_state(IDLE);
    state.enemies[0].texture_id = enemy2_texture_id;
    state.enemies[0].set_position(glm::vec3(8.0f, 5.0f, 0.0f));
    state.enemies[0].set_movement(glm::vec3(0.0f));
    state.enemies[0].speed = 1.0f;
    state.enemies[0].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    state.enemies[0].set_height(0.8f);
    state.enemies[0].set_width(0.8f);

    state.enemies[1].set_entity_type(ENEMY);
    state.enemies[1].set_ai_type(STANDER);
    state.enemies[1].set_ai_state(IDLE);
    state.enemies[1].texture_id = enemy3_texture_id;
    state.enemies[1].set_position(glm::vec3(16.0f, 5.0f, 0.0f));
    state.enemies[1].set_movement(glm::vec3(0.0f));
    state.enemies[1].speed = 1.0f;
    state.enemies[1].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    state.enemies[1].set_height(0.8f);
    state.enemies[1].set_width(0.8f);

    state.enemies[2].set_entity_type(ENEMY);
    state.enemies[2].set_ai_type(STANDER);
    state.enemies[2].set_ai_state(IDLE);
    state.enemies[2].texture_id = enemy1_texture_id;
    state.enemies[2].backup1 = enemy1_texture_id2;
    state.enemies[2].backup2 = enemy1_texture_id;
    state.enemies[2].set_position(glm::vec3(24.0f, 10.0f, 0.0f));
    state.enemies[2].set_movement(glm::vec3(0.0f));
    state.enemies[2].speed = 1.0f;
    state.enemies[2].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    state.enemies[2].set_height(1.0f);
    state.enemies[2].set_width(1.0f);
    
    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    
    state.bgm = Mix_LoadMUS("assets/marnie.mp3");
    Mix_PlayMusic(state.bgm, -1);
    Mix_VolumeMusic(50.0f);
    
    state.jump_sfx = Mix_LoadWAV("assets/bounce.wav");
    state.win_sfx = Mix_LoadWAV("assets/win.wav");
    state.lose_sfx = Mix_LoadWAV("assets/lose.wav");
}

void World::update(float delta_time)
{
    this->state.player->update(delta_time, state.player, state.enemies, this->ENEMY_COUNT, this->state.map);

    for (int i = 0; i < ENEMY_COUNT; i++) {
        state.enemies[i].update(delta_time, state.player, state.enemies, this->ENEMY_COUNT, this->state.map);
    }
}

void World::render(ShaderProgram *program) {
    this->state.map->render(program);
    this->state.player->render(program);

    if (this->state.player->get_position().y < -10.0f) {
        Utility::draw_text(program, Utility::load_texture("assets/font1.png"), "You've won!", 0.5f, 0.001f, glm::vec3(3.0f, -3.0f, 0.0f));

        if (!played) {
            played = true;
            Mix_PlayChannel(-1, state.win_sfx, 0);
            Mix_HaltMusic();
            state.player->set_position(glm::vec3(3.0f, 1.0f, 0.0f));
        }
    }

    if (!state.player->is_active) {
        this->state.next_scene_id = 2;
    }

    for (int i = 0; i < ENEMY_COUNT; i++) {
        state.enemies[i].render(program);
    }
}
