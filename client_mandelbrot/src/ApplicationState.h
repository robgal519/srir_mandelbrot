//
// Created by glaeqen on 06/04/19.
//

#ifndef MANDELBROTCLIENT_APPLICATIONSTATE_H
#define MANDELBROTCLIENT_APPLICATIONSTATE_H


#include "Pixel.h"
#include <utility>

struct ApplicationState {
    int running = true;
    int windowHeight = 480;
    int windowWidth = 640;
    double leftTopX = -2;
    double leftTopY = 1.5;
    double rightBottomX = 0.5;
    double rightBottomY = -1.5;
    bool requestImage = true;
    bool keyDown = false;
    std::pair<int, int> firstMouseClick = {0, 0};
    std::pair<int, int> secondMouseClick = {windowWidth, windowHeight};
};


#endif //MANDELBROTCLIENT_APPLICATIONSTATE_H
