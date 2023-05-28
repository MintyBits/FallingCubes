#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <iostream>
#include <vector>

// remember to use resolutions proportional to your aspect ratio or else the objects will fall into the void
#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 600

#define RECT_WIDTH 100
#define RECT_HEIGHT 100
#define RECT_GRAVITY 2
#define RECT_ACCELERATION_CONSTANT 0.04
#define RECT_MAX_ACCELERATION 2

struct RectangleCollisionObject {
    // Used to see if we should stop falling or not
    enum CollisionDirection {
        LEFT,
        RIGHT,
        UP,
        DOWN,
        NONE,
    };

    RectangleCollisionObject(uint64_t id, int x, int y, int h, int w) {
        this->id = id;
        this->rect = SDL_Rect {x, y, h, w};
    }

    // Checks where an object is possibly collding with another object
    CollisionDirection CheckCollision(RectangleCollisionObject rect2) {
        float w = 0.5 * (rect.w + rect2.rect.w);
        float h = 0.5 * (rect.h + rect2.rect.h);

        float dx = rect.x - rect2.rect.x;
        float dy = rect.y - rect2.rect.y;

        // ngl I just copy pasted this entire function, I can't math

        if (abs(dx) <= w && abs(dy) <= h)
        {
            float wy = w * dy;
            float hx = h * dx;

            if (wy > hx) {
                if (wy > -hx)
                    return CollisionDirection::UP;
                else
                    return CollisionDirection::LEFT;
            }
            else {
                if (wy > -hx)
                    return CollisionDirection::RIGHT;
                else
                    return CollisionDirection::DOWN;
            }

        }
        return CollisionDirection::NONE;
    }


    uint64_t id;
    float acceleration = 0.1f;
    SDL_Rect rect;
};

// draws text automatically on the screne
void DrawText(SDL_Renderer* renderer, std::string str, int x, int y) {
    // open font as file descriptor
    TTF_Font* sans = TTF_OpenFont("FreeSans.ttf", 24);

    SDL_Color white = {255 ,255 ,255};

    // get the message to a surface so we can adjust width and height automatically
    SDL_Surface* surface_message =
            TTF_RenderText_Solid(sans, str.c_str(), white);

    SDL_Texture* message = SDL_CreateTextureFromSurface(renderer, surface_message);

    SDL_Rect msg_rect;
    msg_rect.h = surface_message->h; // auto adjust sizing
    msg_rect.w = surface_message->w;
    msg_rect.x = x;
    msg_rect.y = y;

    // render
    SDL_RenderCopy(renderer, message, NULL, &msg_rect);

    // close the font descriptor and free our objects
    TTF_CloseFont(sans);
    SDL_FreeSurface(surface_message);
    SDL_DestroyTexture(message);
}

class Simulation {
public:
    Simulation() {
        if(SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cout << "SDL Failed Init: " << SDL_GetError() << std::endl;
            exit(1);
        }

        // required to draw text on screen
        if(TTF_Init() < 0) {
            std::cout << "TTF Failed Init" << SDL_GetError() << std::endl;
        }

        window = SDL_CreateWindow("RigidBodies",
                                                  SDL_WINDOWPOS_UNDEFINED,
                                                  SDL_WINDOWPOS_UNDEFINED,
                                                  WINDOW_WIDTH, WINDOW_HEIGHT,
                                                  SDL_WINDOW_SHOWN);
        if(window == NULL) {
            std::cout << "Error: " << SDL_GetError() << std::endl;
            exit(1);
        }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if(renderer == NULL) {
            std::cout << "Error: " << SDL_GetError() << std::endl;
            exit(1);
        }

    }

    ~Simulation() {
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();
    }

    // takes delta to display cycle rate
    void Update(int delta) {
        // Create the block that follows your cursor
        this->CreateMouseOutline();
        this->HandleInput();

        // Set the background color
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // black
        SDL_RenderClear(renderer);

        // Draw the loop rate and the object count
        DrawText(renderer, "Cycles Per Second : " + std::to_string(1000 / delta), 0, 0);
        DrawText(renderer, "Total Objects : " + std::to_string(physicsObjects.size()), 0, 24);

        // apply gravity and draw our physics cubes
        for(int i = 0; i < physicsObjects.size(); i++) {
            // apply gravity TODO
            ApplyGravity(&physicsObjects[i]);

            // draw cubes
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // red
            SDL_RenderFillRect(renderer, &physicsObjects[i].rect);

            // draw outline
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // white
            SDL_RenderDrawRect(renderer, &physicsObjects[i].rect);
        }

        // draw the block that follows your cursor
        SDL_SetRenderDrawColor(renderer, 169, 169, 169, 255); // grey
        SDL_RenderFillRect(renderer, &this->objectOutline);

        // present to screen
        SDL_RenderPresent(renderer);
    }
private:

    // applys a force that makes an object fall downwards
    void ApplyGravity(RectangleCollisionObject* target) {
        // since the rects are centered we just subtract one hundred from
        // resolution for a floor, it's lazy but it works.
        if(target->rect.y >= WINDOW_HEIGHT - 100)
            return; // don't fall off the screen

        // check if we can move down
        for(auto obj : physicsObjects) {
            if(obj.id == target->id)
                continue; // don't include yourself in collision or else you freeze

            if(target->CheckCollision(obj) == RectangleCollisionObject::CollisionDirection::DOWN)
                return; // can't move down anymore
        }

        // add acceleration to the cube to increase the fall speed
        if(target->acceleration < RECT_MAX_ACCELERATION)
            target->acceleration += RECT_ACCELERATION_CONSTANT;

        target->rect.y += RECT_GRAVITY * target->acceleration;
    }

    void HandleInput() {
        SDL_Event e;

        // wait for mouse click
        while(SDL_PollEvent(&e)) {
            switch(e.type) {
            case SDL_MOUSEBUTTONDOWN:
                // get position of our mouse
                int x, y;
                SDL_GetMouseState(&x, &y);

                // make sure there are no blocks in the way of our mouse
                for(auto obj : physicsObjects) {
                    if(obj.CheckCollision(RectangleCollisionObject(physicsObjects.size() + 1, x - (RECT_WIDTH / 2), y - (RECT_HEIGHT / 2), RECT_WIDTH, RECT_HEIGHT)) != RectangleCollisionObject::CollisionDirection::NONE)
                        return;
                }

                // safe to spawn in our new rectangle
                physicsObjects.push_back(RectangleCollisionObject(physicsObjects.size() + 1, x - (RECT_WIDTH / 2), y - (RECT_HEIGHT / 2), RECT_WIDTH, RECT_HEIGHT));
                break;
            case SDL_QUIT:
                exit(0);
                break;
            }
        }
    }

    // method for creating the grey cube surrounding the cursor
    void CreateMouseOutline() {
        int x, y;
        SDL_GetMouseState(&x, &y);

        // we divide by two to center the block on the cursor
        objectOutline = SDL_Rect {x - (RECT_WIDTH / 2), y - (RECT_HEIGHT / 2), RECT_WIDTH, RECT_HEIGHT};
    }

    // follows the mouse and shows where an object will spawn
    SDL_Rect objectOutline;

    // these are objects you spawn by clicking
    std::vector<RectangleCollisionObject> physicsObjects;

    SDL_Renderer* renderer;
    SDL_Window* window;
};

