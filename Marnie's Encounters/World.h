#include "Scene.h"

class World : public Scene {
public:
    int ENEMY_COUNT = 3;

    bool played = false;
    
    ~World();
    
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
};
