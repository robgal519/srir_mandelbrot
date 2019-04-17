//
// Created by glaeqen on 06/04/19.
//

#include <SDL2/SDL.h>
#include "Application.h"
#include "Exception.h"
#include "WindowToComplexPlaneMapper.h"
#include <exception>

struct WindowOpeningFailedException : Exception {
    WindowOpeningFailedException(const std::string &message) : Exception(message) {}
};

Application::Application(const std::string &applicationName,
                         const std::string &requestImagePipePath,
                         const std::string &retrieveImagePipePath)
        : requestImagePipe(requestImagePipePath),
          responseImagePipe(retrieveImagePipePath) {
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow(
            applicationName.c_str(),
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            applicationState.windowWidth,
            applicationState.windowHeight,
            SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL
    );

    renderer = SDL_CreateRenderer(
            window,
            -1,
            SDL_RENDERER_ACCELERATED
    );

    texture = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_RGB24,
            SDL_TEXTUREACCESS_STREAMING,
            applicationState.windowWidth,
            applicationState.windowHeight
    );

    if (!window) {
        throw WindowOpeningFailedException(
                std::string("Error occured when creating a window") + SDL_GetError()
        );
    }
}

Application::~Application() noexcept {
    if (texture) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    SDL_Quit();
}

void Application::start() {
    while (applicationState.running) {
        handleEvents();
        handlePipes();
        render();
    }

}

void Application::handlePipes() {
    if (!applicationState.requestImage) return;

    WindowToComplexPlaneMapper mapper(applicationState);

    Request imageRequest{
            applicationState.windowWidth,
            applicationState.windowHeight,
            mapper.getLeftTopX(),
            mapper.getLeftTopY(),
            mapper.getRightBottomX(),
            mapper.getRightBottomY(),
            applicationState.chosenColor
    };

    requestImagePipe.sendRequest(imageRequest);
    responseImagePipe.readResponse(currentImage, applicationState.windowWidth, applicationState.windowHeight);
}

void Application::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                applicationState.running = false;
                break;
            case SDL_KEYUP:
                if (event.key.keysym.sym == SDLK_q) {
                    applicationState.requestImage = !applicationState.requestImage;
                }
                if (event.key.keysym.sym == SDLK_r) {
                    applicationState.chosenColor = Pixel::RED;
                }
                if (event.key.keysym.sym == SDLK_g) {
                    applicationState.chosenColor = Pixel::GREEN;
                }
                if (event.key.keysym.sym == SDLK_b) {
                    applicationState.chosenColor = Pixel::BLUE;
                }
                break;
        }
    }
}

void Application::render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
    SDL_UpdateTexture
            (
                    texture,
                    nullptr,
                    &*currentImage.begin(),
                    applicationState.windowWidth * 3
            );

    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
}

