#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <vector>
#include <stack>
#include "ApplicationState.h"
#include "DataRequestNamedPipe.h"
#include "DataResponseNamedPipe.h"

class Application {

public:
    Application(const std::string &applicationName,
                const std::string &requestImagePipePath,
                const std::string &retrieveImagePipePath);

    virtual ~Application() noexcept;

    void start();

    void handleEvents();

    void remapCoordinates();

    void handlePipes();

    void render();

private:
    DataRequestNamedPipe requestImagePipe;
    DataResponseNamedPipe responseImagePipe;
    ApplicationState applicationState;
    std::vector<Pixel> currentImage;
    std::stack<ApplicationState> lifoCoordinates;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
};

