/*
 * Created on Wed Apr 17 2019 by Robert Gałat
 *
 * Copyright (c) 2019 AGH FiIS
 */

#ifndef COLORS_GUARD
#define COLORS_GUARD

namespace colors{
struct RGB{
    unsigned char R,G,B;
};

RGB HSV_to_RGB(unsigned char H, unsigned char S, unsigned char V){

    unsigned char region, remainder, p, q, t,r,g,b;

    if (S == 0)
    {
        return {V,V,V};
    }

    region = H/ 43;
    remainder = (H - (region * 43)) * 6; 

    p = (V * (255 - S)) >> 8;
    q = (V * (255 - ((S * remainder) >> 8))) >> 8;
    t = (V * (255 - ((S * (255 - remainder)) >> 8))) >> 8;

    switch (region)
    {
        case 0:
            r = V; g = t; b = p;
            break;
        case 1:
            r = q; g = V; b = p;
            break;
        case 2:
            r = p; g = V; b = t;
            break;
        case 3:
            r = p; g = q; b = V;
            break;
        case 4:
            r = t; g = p; b = V;
            break;
        default:
            r = V; g = p; b = q;
            break;
    }

    return {r,g,b};
}

RGB RGBColor(int counter, int maxIterations){
    unsigned char H,S,V;
    H = static_cast<unsigned char>(255*counter/maxIterations);
    S = 255;
    if(counter<maxIterations)
        V = 255;
    else
        V = 0;
    
    return HSV_to_RGB(H,S,V);
}

} // namespace colors

#endif // COLORS_GUARD