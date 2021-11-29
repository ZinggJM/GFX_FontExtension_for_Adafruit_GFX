// GFX_FontExtension_for_Adafruit_GFX : A Font Extension companion class for Adafruit_GFX for UTF-8 Fonts.
//
// based on code extracted from https://github.com/adafruit/Adafruit-GFX-Library and https://github.com/Bodmer/Adafruit-GFX-Library#enhanced-fork
//
// Author : J-M Zingg
//
// Version : see library.properties
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE
//
// Library: https://github.com/ZinggJM/GFX_FontExtension_for_Adafruit_GFX
//
// Font files can be created using the fontconverter from https://github.com/Bodmer/GFX_Font_Converter
// see tutorial https://www.youtube.com/watch?v=L8MmTISmwZ8

#ifndef _GFX_FontExtension_for_Adafruit_GFX_H_
#define _GFX_FontExtension_for_Adafruit_GFX_H_

#include <Adafruit_GFX.h>

class GFX_FontExtension_for_Adafruit_GFX : public Print
{
  public:
    GFX_FontExtension_for_Adafruit_GFX();
    void init(Adafruit_GFX &gfx);
    void begin(Adafruit_GFX &gfx);
    void drawChar(int16_t x, int16_t y, uint16_t c, uint16_t color, uint16_t bg, uint8_t size_x, uint8_t size_y, const GFXfont* gfxFont);
    virtual size_t write(uint8_t);
    void setTextSize(uint8_t s);
    void setTextSize(uint8_t sx, uint8_t sy);
    void setFont(const GFXfont* f = NULL); // use this to set a font (to its page)
    void setFont(const GFXfont* f, uint8_t page); // use setFont(0, page); to remove a font, e.g. setFont(0, FreeMonoBold12pt7b.first / 256);
    void setCursor(int16_t x, int16_t y);
    void setTextColor(uint16_t c);
    void setTextColor(uint16_t c, uint16_t bg);
    void setTextWrap(bool w);
    int16_t getCursorX(void) const;
    int16_t getCursorY(void) const;
  private:
    uint16_t decodeUTF8(uint8_t c);
  private:
    Adafruit_GFX* adafruit_gfx;
    const GFXfont* gfxFonts[256];
    uint8_t  decoderState;   // UTF-8 decoder state
    uint16_t decoderBuffer;  // Unicode code-point buffer
    int16_t cursor_x, cursor_y;
    uint16_t textcolor;
    uint16_t textbgcolor;
    uint8_t textsize_x;
    uint8_t textsize_y;
    bool wrap;
};

#endif
