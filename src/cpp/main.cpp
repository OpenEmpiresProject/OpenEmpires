#include "Game.h"
#include "Studio.h"
#include "internal/DRSTester.h"

int main()
{
    game::Game game;
    return game.run(game::Game::Params());

    // drs::DRSTester tester;
    // tester.load();
    // return 0;

    // game::Studio studio;
    // studio.init();
    // studio.run();
    // return 0;
}
