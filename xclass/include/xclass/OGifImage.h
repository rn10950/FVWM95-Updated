/**************************************************************************

    This file is part of xclass.
    Copyright (C) 2002-2003 Hector Peraza.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

**************************************************************************/

//   "The Graphics Interchange Format(c) is the Copyright property of
//    CompuServe Incorporated. GIF(sm) is a Service Mark property of
//    CompuServe Incorporated."

#ifndef __OGIFIMAGE_H
#define __OGIFIMAGE_H

#include <X11/Xlib.h>

#include <xclass/OXClient.h>
#include <xclass/OXWindow.h>
#include <xclass/OImage.h>
#include <xclass/utils.h>


//----------------------------------------------------------------------

#define MAX_CODES   4095

struct _GifScreen {
  unsigned int  _width, _height;
  unsigned char _BackgroundColorIndex;
  unsigned char _PixelAspectRatio;
  bool _HasGlobalColorTable, SortedColors;
  unsigned int ColorResolution;
  unsigned int GlobalColorTableSize;
};

struct _color {
  unsigned char Red, Green, Blue;
};

struct _GifImage {
  _GifImage() { UseTransparentColor = 0; TransparentColorIndex = 0;
                DisposalMethod = 0; Image = 0; }
  unsigned int _leftpos, _toppos, _width, _height;
  bool _HasLocalColorTable, _isInterlaced,
       _SortedColors, UseTransparentColor;
  int ColorTableSize;
  unsigned int DelayTime;
  unsigned char TransparentColorIndex;
  unsigned char DisposalMethod;
  XImage *Image;
  Pixmap TransparentMask;
  _GifImage *NextGif;
};

struct _ApplicationExtension {
  unsigned char BlockSize;
  unsigned char Identifier[8];
  unsigned char AuthenticationCode[3];
};


//--------------------------------------------------------------------
// OGifImage - a gif graphics file class. Animated and transparent
//             images are supported.

class OGifImage : public OImage {
public:
  OGifImage(OXClient *client, const char *filename);
  virtual ~OGifImage();

  virtual void NextFrame();     // advance to next frame
  
  virtual unsigned long GetAnimDelay() { return CurrentGif->DelayTime * 10L; }

  virtual void Draw(Drawable d, GC gc, int src_x, int src_y,
                    int src_w, int src_h, int dst_x, int dst_y);

  virtual int GetWidth() { return GifScreen ? GifScreen->_width : 0; }
  virtual int GetHeight() { return GifScreen ? GifScreen->_height : 0; }

  long GetBackgroundPixel() {
               return XPixelTable[GifScreen->_BackgroundColorIndex]; }

  virtual bool isTransparent() { return FirstGif->UseTransparentColor; }

  bool isInvalidGif() { return InvalidGif; }
  
private:
  virtual void DrawFrame(Drawable d, GC gc, int src_x, int src_y,
                         int src_w, int src_h, int dst_x, int dst_y);

  bool InvalidGif;

  void ReadColorTable(_color *Table, unsigned int TableSize);
  void ReadImage();
  void ReadImageDescriptor();
  void ReadApplicationExtension();
  void ReadGraphExtension();
  void ReadCommentExtension();
  void ReadPlainTextExtension();
  int  ReadSubBlock();
  int  ReadTerminator();

  int  get_byte();
  short init_LZW(short size);
  short get_next_code();
  short decode_LZW(short linewidth);
  int  out_line(unsigned char pixels[], int linelen);
  void AllocColor(_color *Table, int ColorMapSize);
  void AllocClosestColor(int ColorMapSize);
  
  void DisposeCurrent(Drawable d);

  unsigned int interlacepass;
  unsigned int interlacediff;
  
  _GifScreen *GifScreen;

  FILE *GifFile;
  _color GlobalColorTable[256];
  _color LocalColorTable[256];

  // LZW decoder variables
  int bad_code_count;

  short curr_size;               // The current code size
  short clear;                   // Value for a clear code
  short ending;                  // Value for an ending code
  short newcodes;                // First available code
  short top_slot;                // Highest code for current size
  short slot;                    // Last read code

  // The following static variables are used for separating out codes
  short navail_bytes;            // # bytes left in block
  short nbits_left;              // # bits left in current byte
  unsigned char b1;              // Current byte
  unsigned char byte_buff[257];  // Current block
  unsigned char *pbytes;         // Pointer to next byte in block

  unsigned char stack[MAX_CODES + 1];   // Stack for storing pixels
  unsigned char suffix[MAX_CODES + 1];  // Suffix table
  unsigned short prefix[MAX_CODES + 1]; // Prefix linked list
  unsigned int lineheight;
	
  // XWindows variables
  Display *display;
  int screen; 

  _GifImage *FirstGif, *CurrentGif;

  XColor XColorTable[256];
  long XPixelTable[256];
  Colormap XColorMap;
  GC transGC, clearGC;  // GC's with depth 1
  GC copyGC;
};


#endif  // __OGIFIMAGE_H
