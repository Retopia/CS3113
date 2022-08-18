#include "Scene.h"

class EncounterA : public Scene {
public:    
    ~EncounterA();

    bool played = false;
    
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;

    GLuint map_texture_id;
    GLuint fireball_small_texture_id;
    GLuint fireball_large_texture_id;

    float passed_time = 0.0f;
    float prevSpawnTime = 0.0f;
};
