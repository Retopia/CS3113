#include "Scene.h"

class Menu : public Scene {
public:
    int ENEMY_COUNT = 1;
    
    ~Menu();

    bool played = false;
    
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
};
