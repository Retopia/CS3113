#include "Scene.h"

class LevelC : public Scene {
public:
    int ENEMY_COUNT = 6;
    
    ~LevelC();

    bool played = false;
    
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
};
