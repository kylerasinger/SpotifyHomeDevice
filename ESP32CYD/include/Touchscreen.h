// touchscreen.h
#ifndef TOUCHSCREEN_H
#define TOUCHSCREEN_H

#include <XPT2046_Touchscreen.h>
#include "Config.h"

void initTouchscreen(XPT2046_Touchscreen &touchscreen, SPIClass &spi){
    spi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
    touchscreen.begin(spi);
    touchscreen.setRotation(1); //change to 3 if upside down
}
void getTouchCoordinates(XPT2046_Touchscreen &touchscreen, uint16_t &x, uint16_t &y, uint16_t &z){
    TS_Point p = touchscreen.getPoint();
    x = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
    y = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);
    z = p.z;
}

#endif // TOUCHSCREEN_H
