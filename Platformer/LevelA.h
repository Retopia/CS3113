#include "Scene.h"

class LevelA : public Scene {
public:
    int ENEMY_COUNT = 6;

    bool played = false;
    
    ~LevelA();
    
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
};
