#include "Simulation.cpp"

// for some reason don't expect this to be correct
#define FRAME_RATE 300

int main() {
    auto sim = Simulation();

    // this code is to lock the game to a certain cycle rate or framerate hopefully works idk
    unsigned int a = SDL_GetTicks();
    unsigned int b = SDL_GetTicks();

    double delta = 0;
    while (true)
    {
        a = SDL_GetTicks();
        delta = a - b;

        if (delta > 1000/FRAME_RATE)
        {
            b = a;
            sim.Update(delta);
        }
    }

    return 0;
}
