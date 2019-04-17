//
// Created by glaeqen on 07/04/19.
//

#include "WindowToComplexPlaneMapper.h"

double WindowToComplexPlaneMapper::getLeftTopX() {
    return applicationState.leftTopX;
}

double WindowToComplexPlaneMapper::getRightBottomX() {
    return applicationState.rightBottomX;
}

double WindowToComplexPlaneMapper::getLeftTopY() {
    return applicationState.leftTopY;
}

double WindowToComplexPlaneMapper::getRightBottomY() {
    return applicationState.rightBottomY;
}

WindowToComplexPlaneMapper::WindowToComplexPlaneMapper(const ApplicationState &state)
        : applicationState(state) {}
