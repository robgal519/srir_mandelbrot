//
// Created by glaeqen on 07/04/19.
//

#include <iostream>
#include "WindowToComplexPlaneMapper.h"

WindowToComplexPlaneMapper::WindowToComplexPlaneMapper(ApplicationState &state)
        : applicationState(state) {}

void WindowToComplexPlaneMapper::remapCoordinates() {
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

double WindowToComplexPlaneMapper::calculateNewPosition(int mouseCoord, double smallerCoordEdge, double biggerCoordEdge,
                                                        int screenSize) {
    return (1.0 * mouseCoord / screenSize) * (biggerCoordEdge - smallerCoordEdge) + smallerCoordEdge;
}
