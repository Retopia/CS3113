#include "EncounterB.h"
#include "Utility.h"
#include <cmath>

#define LEVEL_WIDTH 18
#define LEVEL_HEIGHT 8
#define LOG(argument) std::cout << argument << '\n'

const int FONTBANK_SIZE = 16;

unsigned int EncounterB_DATA[] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

EncounterB::~EncounterB() {
    for (size_t i = 0; i < state.vec_enemies.size(); i++) {
        delete state.vec_enemies.at(i);
    }

    delete    this->state.player;
    delete    this->state.map;
    Mix_FreeChunk(this->state.jump_sfx);
    Mix_FreeMusic(this->state.bgm);
}

void EncounterB::initialise() {
    map_texture_id = Utility::load_texture("assets/tileset.png");
    fireball_small_texture_id = Utility::load_texture("assets/fireball_small.png");
    fireball_large_texture_id = Utility::load_texture("assets/fireball_large.png");

    state.next_scene_id = -1;

    this->state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, EncounterB_DATA, map_texture_id, 1.0f, 4, 1);

    // Code from main.cpp's initialise()
    /**
     George's Stuff
     */
     // Existing
    state.player = new Entity();
    state.player->set_entity_type(PLAYER);
    state.player->set_position(glm::vec3(5.0f, -3.5f, 0.0f));
    state.player->set_movement(glm::vec3(0.0f));
    state.player->speed = 3.5f;
    state.player->set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));
    state.player->texture_id = Utility::load_texture("assets/pokeball.png");
    state.player->height = 0.8f;
    state.player->width = 0.8f;

    // Walking
    //state.player->walking[state.player->LEFT]  = new int[4] { 1, 5, 9,  13 };
    //state.player->walking[state.player->RIGHT] = new int[4] { 3, 7, 11, 15 };
    //state.player->walking[state.player->UP]    = new int[4] { 2, 6, 10, 14 };
    //state.player->walking[state.player->DOWN]  = new int[4] { 0, 4, 8,  12 };

    //state.player->animation_indices = state.player->walking[state.player->RIGHT];  // start George looking left
    //state.player->animation_frames = 4;
    //state.player->animation_index  = 0;
    //state.player->animation_time   = 0.0f;
    //state.player->animation_cols   = 4;
    //state.player->animation_rows   = 4;
    //state.player->set_height(0.8f);
    //state.player->set_width(0.8f);

    // Jumping
    state.player->jumping_power = 6.0f;


    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    //state.bgm = Mix_LoadMUS("assets/marnie.mp3");
    //Mix_PlayMusic(state.bgm, -1);
    //Mix_VolumeMusic(80.0f);

    state.jump_sfx = Mix_LoadWAV("assets/bounce.wav");
    state.win_sfx = Mix_LoadWAV("assets/win.wav");
    state.lose_sfx = Mix_LoadWAV("assets/lose.wav");
}

void EncounterB::update(float delta_time) {
    passed_time += delta_time;
    LOG(fmod(passed_time, 2.0f));

    // Phase 1
    if (passed_time < 15.0f && passed_time - prevSpawnTime > 0.25f) {
        prevSpawnTime = passed_time;
        Entity* ball1 = new Entity();
        ball1->set_entity_type(ENEMY);
        ball1->set_ai_type(STANDER);
        ball1->set_ai_state(IDLE);
        ball1->texture_id = fireball_small_texture_id;
        ball1->set_position(glm::vec3(-1.0f,
            this->state.player->get_position().y, 0.0f));
        ball1->set_movement(glm::vec3(0.0f));
        ball1->speed = 3.0f;
        ball1->set_acceleration(glm::vec3(3.0f, 0.0f, 0.0f));
        ball1->height = 0.2f;
        ball1->width = 0.2f;
        state.vec_enemies.push_back(ball1);
    }

    if (passed_time > 12.0f) {
        state.next_scene_id = 1;
    }

    this->state.player->update(delta_time, state.player, state.vec_enemies, state.vec_enemies.size(), this->state.map);
    //LOG("Player: " << state.player->get_position().x << " " << state.player->get_position().y);

    for (size_t i = 0; i < state.vec_enemies.size(); i++) {
        auto enemy = state.vec_enemies.at(i);
        //LOG("Enemy " << i << ": " << enemy->get_position().x << " " << enemy->get_position().y);
        state.vec_enemies.at(i)->update(delta_time, state.player, state.vec_enemies, state.vec_enemies.size(), this->state.map);
    }
}

void EncounterB::render(ShaderProgram* program)
{
    this->state.map->render(program);
    this->state.player->render(program);

    for (int i = 0; i < state.vec_enemies.size(); i++) {
        state.vec_enemies.at(i)->render(program);
    }
}
