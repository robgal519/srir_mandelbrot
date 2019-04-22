#pragma once

#include <utility>

struct ApplicationState {
    bool running = true;
    bool requestImage = true;
    bool remapCoordinates = false;
    bool keyDown = false;
    int windowHeight = 720;
    int windowWidth = 600;
    double leftTopX = -2;
    double leftTopY = 1.5;
    double rightBottomX = 0.5;
    double rightBottomY = -1.5;
    std::pair<int, int> firstMouseClick = {0, 0};
    std::pair<int, int> secondMouseClick = {windowWidth, windowHeight};
};

