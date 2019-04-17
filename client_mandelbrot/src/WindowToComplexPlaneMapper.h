//
// Created by glaeqen on 07/04/19.
//

#ifndef MANDELBROTCLIENT_WINDOWTOCOMPLEXPLANEMAPPER_H
#define MANDELBROTCLIENT_WINDOWTOCOMPLEXPLANEMAPPER_H


#include "ApplicationState.h"

class WindowToComplexPlaneMapper {

public:
    WindowToComplexPlaneMapper(const ApplicationState &state);

    double getLeftTopX();

    double getLeftTopY();

    double getRightBottomX();

    double getRightBottomY();
private:
    ApplicationState applicationState;
};


#endif //MANDELBROTCLIENT_WINDOWTOCOMPLEXPLANEMAPPER_H
