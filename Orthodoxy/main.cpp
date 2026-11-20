#include <cstdlib>
#include <ctime>
#include "Classes/Engine.h"

int main()
{
    // Different random numbers every time the game starts (the boss picks his attacks with them)
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    Engine game;
    game.run();
    return 0;
}
