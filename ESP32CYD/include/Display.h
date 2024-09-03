#ifndef DISPLAY_H
#define DISPLAY_H

#include <TFT_eSPI.h>
#include "Config.h"

void initDisplay(TFT_eSPI &tft){
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_WHITE);
    tft.setTextColor(TFT_BLACK, TFT_WHITE);
}

void drawTextCentered(TFT_eSPI &tft, const String &text, int x, int y, int fontSize) {
    tft.drawCentreString(text, x, y, fontSize);
}

void drawPause(const uint16_t CENTERX, const uint16_t CENTERY, TFT_eSPI &tft, bool positive){
  uint16_t colourP = positive ? TFT_WHITE : TFT_BLACK;
  uint16_t colourN = positive ? TFT_BLACK : TFT_WHITE;

  tft.fillCircle(CENTERX, CENTERY + 40, 25, colourN);
  tft.drawCircle(CENTERX, CENTERY + 40, 25, colourP);
  tft.fillRect(CENTERX - 8, CENTERY + 32, 6, 18, colourP);
  tft.fillRect(CENTERX + 4, CENTERY + 32, 6, 18, colourP);
}

void drawPlay(const uint16_t CENTERX, const uint16_t CENTERY, TFT_eSPI &tft, bool positive){
  uint16_t colourP = positive ? TFT_WHITE : TFT_BLACK;
  uint16_t colourN = positive ? TFT_BLACK : TFT_WHITE;

  tft.fillCircle(CENTERX, CENTERY + 40, 25, colourN);
  tft.drawCircle(CENTERX, CENTERY + 40, 25, colourP);
  tft.fillTriangle(CENTERX + 8, CENTERY + 40, 
          CENTERX - 6, CENTERY + 40 + 7, 
          CENTERX - 6, CENTERY + 40 - 7, colourP);
}

void drawPrev(const uint16_t CENTERX, const uint16_t CENTERY, TFT_eSPI &tft, bool positive){
  uint16_t colourP = positive ? TFT_WHITE : TFT_BLACK;
  uint16_t colourN = positive ? TFT_BLACK : TFT_WHITE;

  tft.fillCircle(CENTERX - 90, CENTERY + 40, 20, colourN);
  tft.drawCircle(CENTERX - 90, CENTERY + 40, 20, colourP);
  tft.fillTriangle(CENTERX - 96, CENTERY + 40, 
          CENTERX - 86, CENTERY + 45, 
          CENTERX - 86, CENTERY + 35, colourP);
}

void drawNext(const uint16_t CENTERX, const uint16_t CENTERY, TFT_eSPI &tft, bool positive){
  uint16_t colourP = positive ? TFT_WHITE : TFT_BLACK;
  uint16_t colourN = positive ? TFT_BLACK : TFT_WHITE;

  tft.fillCircle(CENTERX + 90, CENTERY + 40, 20, colourN);
  tft.drawCircle(CENTERX + 90, CENTERY + 40, 20, colourP);
  tft.fillTriangle(CENTERX + 96, CENTERY + 40, 
          CENTERX + 86, CENTERY + 45, 
          CENTERX + 86, CENTERY + 35, colourP);
}

void drawProgress(const uint16_t CENTERX, const uint16_t CENTERY, TFT_eSPI &tft, int progressMs, int durationMs){\
  tft.fillRect(CENTERX - 144, CENTERY + 90, 290, 12, TFT_BLACK);
  tft.drawRoundRect(CENTERX - 144, CENTERY + 90, 290, 12, 6, TFT_WHITE);
  tft.fillCircle(CENTERX - 144 + 3, CENTERY + 90 + 6, 5, TFT_WHITE);
  int length = map(progressMs, 0, durationMs, 0, 280);
  tft.fillRect(20, CENTERY + 90, length, 12, TFT_WHITE);
}

void updateProgress(const uint16_t CENTERX, const uint16_t CENTERY, TFT_eSPI &tft, int progressMs, int durationMs){
  int length = map(progressMs, 0, durationMs, 0, 280);
  tft.fillRect(20, CENTERY + 90, length, 12, TFT_WHITE);
}

void drawWait(const uint16_t CENTERX, const uint16_t CENTERY, TFT_eSPI &tft, bool positive){
  uint16_t colourP = positive ? TFT_WHITE : TFT_BLACK;
  uint16_t colourN = positive ? TFT_BLACK : TFT_WHITE;

  tft.fillCircle(CENTERX, CENTERY, 2, colourP);
  tft.fillCircle(CENTERX - 8, CENTERY, 2, colourP);
  tft.fillCircle(CENTERX + 8, CENTERY, 2, colourP);
}

void drawButtons(const uint16_t CENTERX, const uint16_t CENTERY, TFT_eSPI &tft){
  drawPrev(CENTERX, CENTERY, tft, true);
  drawPause(CENTERX, CENTERY, tft, true);
  drawNext(CENTERX, CENTERY, tft, true);
  drawProgress(CENTERX, CENTERY, tft, 0, 100);
}


#endif // DISPLAY_H