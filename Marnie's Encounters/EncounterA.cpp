#include "EncounterA.h"
#include "Utility.h"
#include <cmath>

#define LEVEL_WIDTH 18
#define LEVEL_HEIGHT 8
#define PHASE1LENGTH 10.0f
#define PHASE2LENGTH 10.0f
#define PHASE3LENGTH 10.0f
#define PHASE4LENGTH 10.0f
#define PHASE5LENGTH 20.0f
#define LOG(argument) std::cout << argument << '\n'

const int FONTBANK_SIZE = 16;

unsigned int ENCOUNTERA_DATA[] =
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

GLuint map_texture_id;
GLuint fireball_small_texture_id;
GLuint fireball_large_texture_id;

float passed_time = 0.0f;
float prevSpawnTime = 0.0f;

bool win = false;

EncounterA::~EncounterA() {
    for (size_t i = 0; i < state.vec_enemies.size(); i++) {
        delete state.vec_enemies.at(i);
    }

    delete    this->state.player;
    delete    this->state.map;
    Mix_FreeChunk(this->state.jump_sfx);
    Mix_FreeMusic(this->state.bgm);
}

void EncounterA::initialise() {
    map_texture_id = Utility::load_texture("assets/tileset.png");
    fireball_small_texture_id = Utility::load_texture("assets/fireball_small.png");
    fireball_large_texture_id = Utility::load_texture("assets/fireball_large.png");

    state.next_scene_id = -1;
    
    this->state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, ENCOUNTERA_DATA, map_texture_id, 1.0f, 4, 1);
    
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
    state.player->height = 0.5f;
    state.player->width = 0.5f;
    
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
    Mix_VolumeMusic(30.0f);
    
    state.jump_sfx = Mix_LoadWAV("assets/bounce.wav");
    state.win_sfx = Mix_LoadWAV("assets/win.wav");
    state.lose_sfx = Mix_LoadWAV("assets/lose.wav");
}

void EncounterA::update(float delta_time) {
    passed_time += delta_time;

    // Phase 1
    if (passed_time > 2.0f && passed_time < PHASE1LENGTH + 2.0f && passed_time - prevSpawnTime > 0.25f) {
        prevSpawnTime = passed_time;
        Entity* ball1 = new Entity();
        ball1->set_entity_type(ENEMY);
        ball1->set_ai_type(STANDER);
        ball1->set_ai_state(IDLE);
        ball1->texture_id = fireball_large_texture_id;
        ball1->set_position(glm::vec3(Utility::random(1.0f, 9.0f), 1.0f , 0.0f));
        ball1->set_movement(glm::vec3(0.0f, -1.0f, 0.0f));
        ball1->speed = 6.0f;
        ball1->set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));
        ball1->height = 0.6f;
        ball1->width = 0.6f;
        state.vec_enemies.push_back(ball1);
    }

    if (passed_time > PHASE1LENGTH + 4.0f && passed_time < PHASE1LENGTH + PHASE2LENGTH + 4.0f && passed_time - prevSpawnTime > 0.4f) {
        prevSpawnTime = passed_time;
        prevSpawnTime = passed_time;
        Entity* ball1 = new Entity();
        ball1->set_entity_type(ENEMY);
        ball1->set_ai_type(STANDER);
        ball1->set_ai_state(IDLE);
        ball1->texture_id = fireball_large_texture_id;
        ball1->set_position(glm::vec3(-1.0f,
            state.player->get_position().y, 0.0f));
        ball1->set_movement(glm::vec3(1.0f, 0.0f, 0.0f));
        ball1->speed = 2.5f;
        ball1->set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));
        ball1->height = 0.6f;
        ball1->width = 0.6f;
        state.vec_enemies.push_back(ball1);
    }

    if (passed_time > PHASE1LENGTH + PHASE2LENGTH + 6.0f && passed_time < 
        PHASE1LENGTH + PHASE2LENGTH + PHASE3LENGTH + 6.0f && passed_time - prevSpawnTime > 0.5f) {
        prevSpawnTime = passed_time;
        Entity* ball1 = new Entity();
        ball1->set_entity_type(ENEMY);
        ball1->set_ai_type(STANDER);
        ball1->set_ai_state(IDLE);
        ball1->texture_id = fireball_large_texture_id;
        ball1->set_position(glm::vec3(Utility::random(1.0f, 9.0f), 1.0f, 0.0f));
        ball1->set_movement(glm::vec3(0.0f, -1.0f, 0.0f));
        ball1->speed = 3.0f;
        ball1->set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));
        ball1->height = 0.5f;
        ball1->width = 0.5f;
        state.vec_enemies.push_back(ball1);

        Entity* ball2 = new Entity();
        ball2->set_entity_type(ENEMY);
        ball2->set_ai_type(STANDER);
        ball2->set_ai_state(IDLE);
        ball2->texture_id = fireball_large_texture_id;
        ball2->set_position(glm::vec3(Utility::random(1.0f, 9.0f), -11.0f, 0.0f));
        ball2->set_movement(glm::vec3(0.0f, 1.0f, 0.0f));
        ball2->speed = 3.0f;
        ball2->set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));
        ball2->height = 0.5f;
        ball2->width = 0.5f;
        state.vec_enemies.push_back(ball2);
    }

    if (passed_time > PHASE1LENGTH + PHASE2LENGTH + PHASE3LENGTH + 6.0f && passed_time <
        PHASE1LENGTH + PHASE2LENGTH + PHASE3LENGTH + PHASE4LENGTH + 6.0f && passed_time - prevSpawnTime > 0.4f) {
        prevSpawnTime = passed_time;
        Entity* ball1 = new Entity();
        ball1->set_entity_type(ENEMY);
        ball1->set_ai_type(STANDER);
        ball1->set_ai_state(IDLE);
        ball1->texture_id = fireball_large_texture_id;
        ball1->set_position(glm::vec3(0.0f, Utility::random(-10.0f, 0.0f), 0.0f));
        ball1->set_movement(glm::vec3(1.0f, 0.0f, 0.0f));
        ball1->speed = 2.5f;
        ball1->set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));
        ball1->height = 0.5f;
        ball1->width = 0.5f;
        state.vec_enemies.push_back(ball1);

        Entity* ball2 = new Entity();
        ball2->set_entity_type(ENEMY);
        ball2->set_ai_type(STANDER);
        ball2->set_ai_state(IDLE);
        ball2->texture_id = fireball_large_texture_id;
        ball2->set_position(glm::vec3(10.0f, Utility::random(-10.0f, 0.0f), 0.0f));
        ball2->set_movement(glm::vec3(-1.0f, 0.0f, 0.0f));
        ball2->speed = 3.0f;
        ball2->set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));
        ball2->height = 0.5f;
        ball2->width = 0.5f;
        state.vec_enemies.push_back(ball2);
    }

    if (passed_time > PHASE1LENGTH + PHASE2LENGTH + PHASE3LENGTH + PHASE4LENGTH + 6.0f && passed_time <
        PHASE1LENGTH + PHASE2LENGTH + PHASE3LENGTH + PHASE4LENGTH + PHASE5LENGTH + 6.0f && passed_time - prevSpawnTime > 0.2f) {
        prevSpawnTime = passed_time;
        Entity* ball1 = new Entity();
        ball1->set_entity_type(ENEMY);
        ball1->set_ai_type(STANDER);
        ball1->set_ai_state(IDLE);
        ball1->texture_id = fireball_large_texture_id;
        
        glm::vec3 spawnPos = glm::vec3(0.0f, 0.0f, 0.0f);

        if (Utility::random(0, 100) < 50) {
            spawnPos.x = Utility::random(-1.0f, 11.0f);
            if (Utility::random(0, 100) < 50) {
                spawnPos.y = -12.0f;
            }
            else {
                spawnPos.y = 2.0f;
            }
        } else{
            spawnPos.y = Utility::random(-12.0f, 2.0f);
            if (Utility::random(0, 100) < 50) {
                spawnPos.x = -1.0f;
            }
            else {
                spawnPos.x = 11.0f;
            }
        }

        ball1->set_position(spawnPos);
        
        float y_dist = state.player->get_position().y - ball1->get_position().y;
        float x_dist = state.player->get_position().x - ball1->get_position().x;

        ball1->set_movement(glm::vec3(cos(atan2f(y_dist, x_dist)), sin(atan2f(y_dist, x_dist)), 0.0f));
        ball1->speed = 3.0f;
        ball1->set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));
        ball1->height = 0.4f;
        ball1->width = 0.4f;
        state.vec_enemies.push_back(ball1);
    }

    if (state.player->is_active && passed_time > PHASE1LENGTH + PHASE2LENGTH + PHASE3LENGTH + PHASE4LENGTH + PHASE5LENGTH + 6.0f && !win) {
        Mix_PlayChannel(-1, state.win_sfx, 0);
        Mix_HaltMusic();
        win = true;
    }

    this->state.player->update(delta_time, state.player, state.vec_enemies, state.vec_enemies.size(), this->state.map);
    //LOG("Player: " << state.player->get_position().x << " " << state.player->get_position().y);

    for (size_t i = 0; i < state.vec_enemies.size(); i++) {
        auto enemy = state.vec_enemies.at(i);
        //LOG("Enemy " << i << ": " << enemy->get_position().x << " " << enemy->get_position().y);
        state.vec_enemies.at(i)->update(delta_time, state.player, state.vec_enemies, state.vec_enemies.size(), this->state.map);
    }
}

void EncounterA::render(ShaderProgram *program)
{
    this->state.map->render(program);
    this->state.player->render(program);

    for (int i = 0; i < state.vec_enemies.size(); i++) {
        state.vec_enemies.at(i)->render(program);
    }

    if (win) {
        Utility::draw_text(program, Utility::load_texture("assets/font1.png"), "You've won!", 0.5f, 0.001f, glm::vec3(3.0f, -3.0f, 0.0f));
    }
}
