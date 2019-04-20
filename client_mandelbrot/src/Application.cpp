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
        remapCoordinates();
        handlePipes();
        render();
    }

}

void Application::remapCoordinates() {
    if(!applicationState.remapCoordinates) return;
    applicationState.remapCoordinates = false;

    double oldLeftTopX = applicationState.leftTopX;
    double oldLeftTopY = applicationState.leftTopY;
    double oldRightBottomX = applicationState.rightBottomX;
    double oldRightBottomY = applicationState.rightBottomY;


    applicationState.leftTopX = calculateNewPosition(applicationState.firstMouseClick.first,
                                                     oldLeftTopX,
                                                     oldRightBottomX,
                                                     applicationState.windowWidth);

    applicationState.leftTopY = calculateNewPosition(applicationState.firstMouseClick.second,
                                                     oldRightBottomY,
                                                     oldLeftTopY,
                                                     applicationState.windowHeight);

    applicationState.rightBottomX = calculateNewPosition(applicationState.secondMouseClick.first,
                                                         oldLeftTopX,
                                                         oldRightBottomX,
                                                         applicationState.windowWidth);

    applicationState.rightBottomY = calculateNewPosition(applicationState.secondMouseClick.second,
                                                         oldRightBottomY,
                                                         oldLeftTopY,
                                                         applicationState.windowHeight);

}

double Application::calculateNewPosition(int mouseCoord, double smallerCoordEdge, double biggerCoordEdge,
                                                        int screenSize) {
    return (1.0 * mouseCoord / screenSize) * (biggerCoordEdge - smallerCoordEdge) + smallerCoordEdge;
}

void Application::handlePipes() {
    if (!applicationState.requestImage) return;
    applicationState.requestImage = false;

    Request imageRequest{
            applicationState.running,
            applicationState.windowWidth,
            applicationState.windowHeight,
            applicationState.leftTopX,
            applicationState.leftTopY,
            applicationState.rightBottomX,
            applicationState.rightBottomY
    };

    requestImagePipe.sendRequest(imageRequest);
    if (!applicationState.running) return;
    responseImagePipe.readResponse(currentImage, applicationState.windowWidth, applicationState.windowHeight);
}

void Application::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                applicationState.running = false;
                applicationState.requestImage = true;
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event != SDL_WINDOWEVENT_SIZE_CHANGED) break;
                applicationState.windowWidth = event.window.data1;
                applicationState.windowHeight = event.window.data2;
                applicationState.requestImage = true;
                break;
            case SDL_MOUSEMOTION:
                applicationState.secondMouseClick = std::make_pair(
                        event.button.x,
                        (event.button.x * 1.0 - applicationState.firstMouseClick.first) *
                        applicationState.windowHeight / applicationState.windowWidth +
                        applicationState.firstMouseClick.second
                );
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button != SDL_BUTTON_LEFT) break;
                applicationState.firstMouseClick = std::make_pair(event.button.x, event.button.y);
                applicationState.keyDown = true;
                break;
            case SDL_MOUSEBUTTONUP:
                if (event.button.button != SDL_BUTTON_LEFT) break;
                applicationState.keyDown = false;
                applicationState.remapCoordinates = true;
                applicationState.requestImage = true;
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
    if (applicationState.keyDown) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_Rect outline{applicationState.firstMouseClick.first,
                         applicationState.secondMouseClick.second,
                         applicationState.secondMouseClick.first - applicationState.firstMouseClick.first,
                         applicationState.firstMouseClick.second - applicationState.secondMouseClick.second};
        SDL_RenderDrawRect(renderer, &outline);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    }
    SDL_RenderPresent(renderer);
}

