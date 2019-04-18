//
// Created by glaeqen on 07/04/19.
//

#ifndef MANDELBROTCLIENT_WINDOWTOCOMPLEXPLANEMAPPER_H
#define MANDELBROTCLIENT_WINDOWTOCOMPLEXPLANEMAPPER_H


#include "ApplicationState.h"

class WindowToComplexPlaneMapper {

public:
    WindowToComplexPlaneMapper(ApplicationState &state);

    void remapCoordinates();
private:
    ApplicationState& applicationState;

    static double calculateNewPosition(int mouseCoord, double smallerCoordEdge, double biggerCoordEdge, int screenSize);
};


#endif //MANDELBROTCLIENT_WINDOWTOCOMPLEXPLANEMAPPER_H
