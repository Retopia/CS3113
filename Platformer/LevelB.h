#include "Scene.h"

class LevelB : public Scene {
public:
    int ENEMY_COUNT = 6;
    
    ~LevelB();

    bool played = false;
    
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
};
