#ifndef GAMEENGINE_H
#define GAMEENGINE_H

class GameEngine {
public:
    GameEngine();
    void start();
    void update();
    void render();
    ~GameEngine();
};

#endif // GAMEENGINE_H