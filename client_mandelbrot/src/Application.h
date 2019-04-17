//
// Created by glaeqen on 06/04/19.
//

#ifndef MANDELBROTCLIENT_APPLICATION_H
#define MANDELBROTCLIENT_APPLICATION_H


#include <SDL2/SDL.h>
#include <string>
#include <vector>
#include "ApplicationState.h"
#include "DataRequestNamedPipe.h"
#include "DataResponseNamedPipe.h"
#include "Pixel.h"

class Application {

public:
    Application(const std::string &applicationName,
                const std::string &requestImagePipePath,
                const std::string &retrieveImagePipePath);

    virtual ~Application() noexcept;

    void start();

    void handleEvents();

    void handlePipes();

    void render();

private:
    DataRequestNamedPipe requestImagePipe;
    DataResponseNamedPipe responseImagePipe;
    ApplicationState applicationState;
    std::vector<Pixel> currentImage;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
};


#endif //MANDELBROTCLIENT_APPLICATION_H
