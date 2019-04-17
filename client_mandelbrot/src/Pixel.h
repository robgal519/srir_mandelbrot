//
// Created by Glaeqen on 2019-04-10.
//

#ifndef MANDELBROTCLIENT_PIXEL_H
#define MANDELBROTCLIENT_PIXEL_H

#include <cstdint>

struct Pixel{
    const static Pixel RED;
    const static Pixel GREEN;
    const static Pixel BLUE;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

inline const Pixel Pixel::RED = {0xff, 0, 0};
inline const Pixel Pixel::GREEN = {0, 0xff, 0};
inline const Pixel Pixel::BLUE = {0, 0, 0xff};

#endif //MANDELBROTCLIENT_PIXEL_H
