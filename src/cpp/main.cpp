#include <iostream>
#include "game/GameEngine.h"

int main() {
    GameEngine gameEngine;

    gameEngine.start();

    while (true) {
        gameEngine.update();
        gameEngine.render();
    }

    return 0;
}