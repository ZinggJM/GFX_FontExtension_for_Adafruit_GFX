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

#include "GFX_FontExtension_for_Adafruit_GFX.h"

#ifdef __AVR__
#include <avr/pgmspace.h>
#elif defined(ESP8266) || defined(ESP32)
#include <pgmspace.h>
#endif

#ifndef pgm_read_byte
#define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#endif
#ifndef pgm_read_word
#define pgm_read_word(addr) (*(const unsigned short *)(addr))
#endif
#ifndef pgm_read_dword
#define pgm_read_dword(addr) (*(const unsigned long *)(addr))
#endif

#if !defined(__INT_MAX__) || (__INT_MAX__ > 0xFFFF)
#define pgm_read_pointer(addr) ((void *)pgm_read_dword(addr))
#else
#define pgm_read_pointer(addr) ((void *)pgm_read_word(addr))
#endif

GFX_FontExtension_for_Adafruit_GFX::GFX_FontExtension_for_Adafruit_GFX()
{
  adafruit_gfx = 0;
  decoderState = 0;
  cursor_x = 0;
  cursor_y = 0;
  textsize_x = 1;
  textsize_y = 1;
  wrap = false;
}

void GFX_FontExtension_for_Adafruit_GFX::init(Adafruit_GFX &gfx)
{
  adafruit_gfx = &gfx;
}

void GFX_FontExtension_for_Adafruit_GFX::begin(Adafruit_GFX &gfx)
{
  adafruit_gfx = &gfx;
}

void GFX_FontExtension_for_Adafruit_GFX::drawChar(int16_t x, int16_t y, uint16_t c, uint16_t color, uint16_t bg, uint8_t size_x, uint8_t size_y, const GFXfont* gfxFont)
{
  if (!adafruit_gfx) return;
  //Serial.print("drawChar(0x"); Serial.print(c, HEX); Serial.println(")");
  if (!gfxFont) // 'Classic' built-in font
  {
    adafruit_gfx->drawChar(x, y, c, color, bg, size_x, size_y);
  }
  else // Custom font
  {
    //Serial.print(">drawChar(0x"); Serial.print(c, HEX); Serial.println(")");
    // Character is assumed previously filtered by write() to eliminate
    // newlines, returns, non-printable characters, etc.  Calling
    // drawChar() directly with 'bad' characters of font may cause mayhem!

    c -= pgm_read_word(&gfxFont->first);
    GFXglyph *glyph  = &(((GFXglyph *)pgm_read_pointer(&gfxFont->glyph))[c]);
    uint8_t  *bitmap = (uint8_t *)pgm_read_pointer(&gfxFont->bitmap);

    uint16_t bo = pgm_read_word(&glyph->bitmapOffset);
    uint8_t  w  = pgm_read_byte(&glyph->width),
             h  = pgm_read_byte(&glyph->height);
    int8_t   xo = pgm_read_byte(&glyph->xOffset),
             yo = pgm_read_byte(&glyph->yOffset);
    uint8_t  xx, yy, bits = 0, bit = 0;
    int16_t  xo16 = 0, yo16 = 0;

    if (size_x > 1 || size_y > 1)
    {
      xo16 = xo;
      yo16 = yo;
    }
    //Serial.print("("); Serial.print(bo); Serial.print(","); Serial.print(w); Serial.print(","); Serial.print(h); Serial.println(")");
    adafruit_gfx->startWrite();
    for (yy = 0; yy < h; yy++)
    {
      for (xx = 0; xx < w; xx++)
      {
        if (!(bit++ & 7))
        {
          bits = pgm_read_byte(&bitmap[bo++]);
        }
        if (bits & 0x80)
        {
          if (size_x == 1 && size_y == 1)
          {
            adafruit_gfx->writePixel(x + xo + xx, y + yo + yy, color);
          }
          else
          {
            adafruit_gfx->writeFillRect(x + (xo16 + xx) * size_x, y + (yo16 + yy) * size_y, size_x, size_y, color);
          }
        }
        bits <<= 1;
      }
    }
    adafruit_gfx->endWrite();

  } // End classic vs custom font
}

uint16_t GFX_FontExtension_for_Adafruit_GFX::decodeUTF8(uint8_t c)
{
  // 7 bit Unicode Code Point
  if ((c & 0x80) == 0x00)
  {
    decoderState = 0;
    return (uint16_t)c;
  }
  if (decoderState == 0)
  {
    // 11 bit Unicode Code Point
    if ((c & 0xE0) == 0xC0)
    {
      decoderBuffer = ((c & 0x1F) << 6); // Save first 5 bits
      decoderState = 1;
      return 0;
    }
    // 16 bit Unicode Code Point
    if ((c & 0xF0) == 0xE0)
    {
      decoderBuffer = ((c & 0x0F) << 12); // Save first 4 bits
      decoderState = 2;
      return 0;
    }
    // 21 bit Unicode  Code Point not supported so fall-back to extended ASCII
    if ((c & 0xF8) == 0xF0) return (uint16_t)c;
  }
  else
  {
    if (decoderState == 2)
    {
      decoderBuffer |= ((c & 0x3F) << 6); // Add next 6 bits of 16 bit code point
      decoderState--;
      return 0;
    }
    else // decoderState must be == 1
    {
      decoderBuffer |= (c & 0x3F); // Add last 6 bits of code point
      decoderState = 0;
      return decoderBuffer;
    }
  }
  decoderState = 0;
  return (uint16_t)c; // fall-back to extended ASCII
}

size_t GFX_FontExtension_for_Adafruit_GFX::write(uint8_t data)
{
  if (!adafruit_gfx) return 0;
  uint16_t c = (uint16_t)data;
  //Serial.print("write(0x"); Serial.print(c, HEX); Serial.println(")");
  c = decodeUTF8(data);
  if (c == 0) return 1;
  const GFXfont* gfxFont = gfxFonts[c / 256];
  if (!gfxFont) // 'Classic' built-in font
  {
    //Serial.print(">write(0x"); Serial.print(c, HEX); Serial.println(")");
    if (c > 255) return 1; // Stop 16 bit characters
    if (c == '\n') // Newline?
    {
      cursor_x  = 0;              // Reset x to zero,
      cursor_y += textsize_y * 8; // advance y one line
    }
    else if (c != '\r') // Ignore carriage returns
    {
      if (wrap && ((cursor_x + textsize_x * 6) > adafruit_gfx->width())) // Off right?
      {
        cursor_x  = 0;              // Reset x to zero,
        cursor_y += textsize_y * 8; // advance y one line
      }
      adafruit_gfx->drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize_x, textsize_y);
      cursor_x += textsize_x * 6;   // Advance x one char
    }
  }
  else // Custom font
  {
    //Serial.print(">>write(0x"); Serial.print(c, HEX); Serial.println(")");
    if (c == '\n')
    {
      cursor_x  = 0;
      cursor_y += (int16_t)textsize_y * (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
    }
    else if (c != '\r')
    {
      uint16_t first = pgm_read_word(&gfxFont->first);
      //Serial.print("0x"); Serial.print(first, HEX); Serial.print("..0x"); Serial.println(pgm_read_word(&gfxFont->last), HEX);
      if ((c >= first) && (c <= pgm_read_word(&gfxFont->last)))
      {
        GFXglyph *glyph = &(((GFXglyph *)pgm_read_pointer( &gfxFont->glyph))[c - first]);
        uint8_t   w     = pgm_read_byte(&glyph->width),
                  h     = pgm_read_byte(&glyph->height);
        if ((w > 0) && (h > 0)) // Is there an associated bitmap?
        {
          int16_t xo = (int8_t)pgm_read_byte(&glyph->xOffset); // sic
          if (wrap && ((cursor_x + textsize_x * (xo + w)) > adafruit_gfx->width()))
          {
            cursor_x  = 0;
            cursor_y += (int16_t)textsize_y * (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
          }
          drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize_x, textsize_y, gfxFont);
        }
        cursor_x += (uint8_t)pgm_read_byte(&glyph->xAdvance) * (int16_t)textsize_x;
      }
    }
  }
  return 1;
}

void GFX_FontExtension_for_Adafruit_GFX::setTextSize(uint8_t s)
{
  textsize_x = textsize_y = s;
}

void GFX_FontExtension_for_Adafruit_GFX::setTextSize(uint8_t sx, uint8_t sy)
{
  textsize_x = sx;
  textsize_y = sy;
}

void GFX_FontExtension_for_Adafruit_GFX::setFont(const GFXfont* f)
{
  if (f)
  {
    gfxFonts[pgm_read_word(&f->first) / 256] = f;
    gfxFonts[0] = f; // use also for default, e.g. space and newline
  }
  else gfxFonts[0] = 0;
}

void GFX_FontExtension_for_Adafruit_GFX::setFont(const GFXfont* f, uint8_t page)
{
  if (f && (page == pgm_read_word(&f->first) / 256)) gfxFonts[page] = f;
  else if (!f) gfxFonts[page] = 0;
}

void GFX_FontExtension_for_Adafruit_GFX::setCursor(int16_t x, int16_t y)
{
  cursor_x = x;
  cursor_y = y;
}

void GFX_FontExtension_for_Adafruit_GFX::setTextColor(uint16_t c)
{
  textcolor = c;
}

void GFX_FontExtension_for_Adafruit_GFX::setTextColor(uint16_t c, uint16_t bg)
{
  textcolor = c;
  textbgcolor = bg;
}

void GFX_FontExtension_for_Adafruit_GFX::setTextWrap(bool w) 
{
  wrap = w;
}

int16_t GFX_FontExtension_for_Adafruit_GFX::getCursorX(void) const 
{
  return cursor_x;
}

int16_t GFX_FontExtension_for_Adafruit_GFX::getCursorY(void) const 
{
  return cursor_y;
}
