//
// Created by glaeqen on 06/04/19.
//

#ifndef MANDELBROTCLIENT_APPLICATIONSTATE_H
#define MANDELBROTCLIENT_APPLICATIONSTATE_H


#include "Pixel.h"

struct ApplicationState {
    int running = true;
    int windowHeight = 480;
    int windowWidth = 640;
    double leftTopX = -2;
    double leftTopY = 1.5;
    double rightBottomX = 0.5;
    double rightBottomY = -1.5;
    bool requestImage = true;
    Pixel chosenColor = {0, 0xff, 0};
};


#endif //MANDELBROTCLIENT_APPLICATIONSTATE_H
