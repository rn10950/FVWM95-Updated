/* OXGifFrame - a gif graphics file class to use with xclass library    */

/*   "The Graphics Interchange Format(c) is the Copyright property of	*
 *    CompuServe Incorporated. GIF(sm) is a Service Mark property of	*
 *    CompuServe Incorporated."						*/

/* Unimplemented functions (so far, e.g ToDo list):
 * - Aspect ratio discarded
 * - Most extension's read but discarded - implement char* GetPlainTextExt()
 * - ... Alot of bugs ?? 
 */


#ifndef __OXGIFFRAME_H
#define __OXGIFFRAME_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>
#include <X11/Xlib.h>

#include <xclass/OXClient.h>
#include <xclass/OXFrame.h>
#include <xclass/OTimer.h>
#include <xclass/utils.h>

#define MAX_CODES   4095

struct _GifScreen {
  unsigned int  _width, _height;
  unsigned char _BackgroundColorIndex;
  unsigned char _PixelAspectRatio;
  Bool _HasGlobalColorTable, SortedColors;
  unsigned int ColorResolution;
  unsigned int GlobalColorTableSize;
};

struct _color {
  unsigned char Red, Green, Blue;
};

struct _GifImage {
  unsigned int _leftpos, _toppos, _width, _height;
  Bool _HasLocalColorTable, _isInterlaced,
       _SortedColors, UseTransparentColor;
  int ColorTableSize;
  unsigned int DelayTime;
  unsigned char TransparentColorIndex;
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

class OXGifFrame : public OXFrame {
public:
  OXGifFrame(const OXWindow *p, char *filename,
             int w = 10, int h = 10,
             unsigned int options = CHILD_FRAME);
  virtual ~OXGifFrame();

  virtual ODimension GetDefaultSize() const;
  virtual int HandleExpose(XExposeEvent *event);
  virtual int HandleTimer(OTimer *t);

  int  GetWidth() { return GifScreen->_width; }
  int  GetHeight() { return GifScreen->_height; }
  int  GetLeftPosition() { return FirstGif->_leftpos; }
  int  GetTopPosition() { return FirstGif->_toppos; }
  long GetBackgroundPixel() {
               return XPixelTable[GifScreen->_BackgroundColorIndex]; }
  void StopAnimation(); 
  void StartAnimation(); 
  void StepAnimation();
  Bool isTransparent() { return FirstGif->UseTransparentColor; }
  Bool isInvalidGif() { return InvalidGif; }
  Bool AnimationRunning() { return _AnimationRunning; }
  void DisplayFrame();

private:
  Bool InvalidGif;
  Bool _AnimationRunning;

  void ReadColorTable(_color *Table, unsigned int TableSize);
  void ReadImage();
  void ReadImageDescriptor();
  void ReadApplicationExtension();
  void ReadGraphExtension();
  void ReadCommentExtension();
  int  ReadSubBlock();
  int  ReadTerminator();
  int  get_byte();
  short init_LZW(short size);
  short get_next_code();
  short decode_LZW(short linewidth);
  int  out_line(unsigned char pixels[], int linelen);
  void AllocColor(_color *Table, int ColorMapSize);
  void AllocClosestColor(int ColorMapSize);
  unsigned int interlacepass;
  unsigned int interlacediff;

  _GifScreen *GifScreen;
  OTimer *Delay;

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
  int lineheight;
	
  // XWindows variables
  Display *display;
  int screen; 

  _GifImage *FirstGif, *CurrentGif;
  Pixmap BufferPixmap;
  int ImageCount;
  XColor XColorTable[256];
  long XPixelTable[256];
  Colormap XColorMap;
  GC transGC, clearGC;  // GC's with depth 1
  GC copyGC;
  struct timeval present_time;
  unsigned int waittime;
};


#endif  // __OXGIFFRAME_H
