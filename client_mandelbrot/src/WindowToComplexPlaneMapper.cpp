//
// Created by glaeqen on 07/04/19.
//

#include <iostream>
#include "WindowToComplexPlaneMapper.h"

WindowToComplexPlaneMapper::WindowToComplexPlaneMapper(ApplicationState &state)
        : applicationState(state) {}

void WindowToComplexPlaneMapper::remapCoordinates() {
    double oldLeftTopX = applicationState.leftTopX;
    double oldLeftTopY = applicationState.leftTopY;
    double oldRightBottomX = applicationState.rightBottomX;
    double oldRightBottomY = applicationState.rightBottomY;

    int mostTopLeftX, mostRightBottomX, mostRightBottomY, mostTopLeftY;
    if(applicationState.firstMouseClick.first < applicationState.secondMouseClick.first){
        mostTopLeftX = applicationState.firstMouseClick.first;
        mostRightBottomX = applicationState.secondMouseClick.first;
    }
    else {
        mostTopLeftX = applicationState.secondMouseClick.first;
        mostRightBottomX = applicationState.firstMouseClick.first;
    }

    if(applicationState.firstMouseClick.second < applicationState.secondMouseClick.second){
        mostRightBottomY = applicationState.firstMouseClick.second;
        mostTopLeftY = applicationState.secondMouseClick.second;
    }
    else {
        mostRightBottomY = applicationState.secondMouseClick.second;
        mostTopLeftY = applicationState.firstMouseClick.second;
    }

    applicationState.leftTopX = calculateNewPosition(mostTopLeftX,
                                                     oldLeftTopX,
                                                     oldRightBottomX,
                                                     applicationState.windowWidth);

    applicationState.leftTopY = calculateNewPosition(mostTopLeftY,
                                                     oldRightBottomY,
                                                     oldLeftTopY,
                                                     applicationState.windowHeight);

    applicationState.rightBottomX = calculateNewPosition(mostRightBottomX,
                                                         oldLeftTopX,
                                                         oldRightBottomX,
                                                         applicationState.windowWidth);

    applicationState.rightBottomY = calculateNewPosition(mostRightBottomY,
                                                         oldRightBottomY,
                                                         oldLeftTopY,
                                                         applicationState.windowHeight);

}

double WindowToComplexPlaneMapper::calculateNewPosition(int mouseCoord, double smallerCoordEdge, double biggerCoordEdge,
                                                        int screenSize) {
    return (1.0 * mouseCoord / screenSize) * (biggerCoordEdge - smallerCoordEdge) + smallerCoordEdge;
}
