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

// OGifImage - a gif graphics file class to use with xclass library

//   "The Graphics Interchange Format(c) is the Copyright property of
//    CompuServe Incorporated. GIF(sm) is a Service Mark property of
//    CompuServe Incorporated."

// Unimplemented functions:
// - Aspect ratio discarded
// - Most extension's read but discarded
// - char *GetPlainTextExt()

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include <X11/Xlib.h>

#include <xclass/OGifImage.h>


#define LM_to_uint(a,b)         (((b) << 8) | ((a) & 0xFF))   

// The defined ERRS for the decode_LZW routine

#define OUT_OF_MEMORY -10
#define BAD_CODE_SIZE -20
#define READ_ERROR    -1
#define WRITE_ERROR   -2
#define OPEN_ERROR    -3
#define CREATE_ERROR  -4

static long code_mask[13] = {
  0,
  0x0001, 0x0003,
  0x0007, 0x000F,
  0x001F, 0x003F,
  0x007F, 0x00FF,
  0x01FF, 0x03FF,
  0x07FF, 0x0FFF
};

#define PlainText_Ext     0x01  // Plain Text Extension
#define Image_Descriptor  0x2C  // Image Descriptor
#define Extension_Intro	  0x21  // Extension Introducer
#define Graph_Control_Ext 0xF9  // Graphic Control Extension 
#define Comment_Ext       0xFE  // Comment Extension
#define Applic_Ext        0xFF  // Application Extension
#define EndOfGif          0x3B  // Last character of valid gif file
#define BlockTerminator   0x00

#define NoDisposal        0
#define DoNotDispose      1
#define RestoreToBgnd     2
#define RestoreToPrev     3


//----------------------------------------------------------------------

OGifImage::OGifImage(OXClient *client, const char *filename) :
  OImage(client) {

    char type[4], ver[4];
    unsigned char buf[16];
    int go, next;
    XGCValues gcval;
    unsigned long gcmask;
    Window Root;
    Pixmap pix_dummy;

    FirstGif = NULL;
    GifScreen = NULL;
    _bufferPixmap = None;
    _bufferMask = None;

    type[3] = '\0'; // Make an end mark
    ver[3]  = '\0';
  
    InvalidGif = False;
    navail_bytes = 0;
    nbits_left = 0;
    _numFrames = 0;
    lineheight = 0;

    display = _client->GetDisplay();
    screen = DefaultScreen(display);
    XColorMap = DefaultColormap(display, screen);
    Root = _client->GetRoot()->GetId();
    pix_dummy = XCreatePixmap(display, Root, 1, 1, 1);

    gcmask = GCForeground | GCBackground | GCGraphicsExposures;
    gcval.foreground = 1; //XBlackPixel(display, screen);
    gcval.background = 0; //XWhitePixel(display, screen);
    gcval.graphics_exposures = False;
    transGC = XCreateGC(display, pix_dummy, gcmask, &gcval);

    gcmask = GCForeground | GCGraphicsExposures;
    gcval.foreground = 0; //XWhitePixel(display, screen);
    gcval.graphics_exposures = False;
    clearGC = XCreateGC(display, pix_dummy, gcmask, &gcval);

    // We don't need this any more, so delete it
    XFreePixmap(display, pix_dummy);

    gcmask = GCForeground | GCGraphicsExposures;
    gcval.foreground = 0;
    gcval.graphics_exposures = False;
    copyGC = XCreateGC(display, Root, gcmask, &gcval);

    GifFile = fopen(filename, "r+b");
    if (GifFile == NULL) {
      fprintf(stderr, "OGifImage: File Not Found (%s)\n", filename);
      InvalidGif = True;
      return;
    }
    fread(type, 3, 1, GifFile);
    if (strcmp(type, "GIF") != 0) {
      fprintf(stderr, "OGifImage: This is Not a Gif file (%s)\n", filename);
      fclose(GifFile);
      InvalidGif = True;
      return;
    }
    fread(ver, 3, 1, GifFile);
    if ((strcmp(ver, "87a") != 0) && (strcmp(ver, "89a") != 0)) {
      fprintf(stderr, "OGifImage: Unknown Gif Version %s (%s)\n", ver, filename);
      fclose(GifFile);
      InvalidGif = True;
      return;
    }

    FirstGif = new _GifImage;
    FirstGif->NextGif = NULL;
    CurrentGif = FirstGif;

    GifScreen = new _GifScreen;

    fread(buf, 7, 1, GifFile);
    GifScreen->_width = LM_to_uint(buf[0], buf[1]);
    GifScreen->_height = LM_to_uint(buf[2], buf[3]);
    GifScreen->_HasGlobalColorTable = ((buf[4] & 0x80) == 0x80);
    GifScreen->ColorResolution = (((buf[4] & 0x70) >> 4) + 1);
    GifScreen->SortedColors = ((buf[4] & 0x08) == 0x08);

    if (GifScreen->_HasGlobalColorTable)
      GifScreen->GlobalColorTableSize = 2 << (buf[4] & 0x07);
    else
      GifScreen->GlobalColorTableSize = 0;
  
    GifScreen->_BackgroundColorIndex = buf[5];
    if (buf[6] == 0)
      GifScreen->_PixelAspectRatio = 0;
    else
      GifScreen->_PixelAspectRatio = (buf[6] + 15) / 64;
  
    if (GifScreen->_HasGlobalColorTable)
      ReadColorTable(&GlobalColorTable[0],
                     GifScreen->GlobalColorTableSize);

    go = 1;
    while (go) {
      next = get_byte();
      if (next < 0) break;

      switch (next) {
        case Image_Descriptor:
          lineheight = 0;
          interlacepass = 1;
          interlacediff = 8;
          ReadImageDescriptor();
          ReadImage();
          ReadTerminator();
          break;

        case Extension_Intro:
          next = get_byte();
          switch (next) {
            case PlainText_Ext:
              ReadPlainTextExtension();
              break;

            case Graph_Control_Ext: 
              if (_numFrames != 0) {
                CurrentGif->NextGif = new _GifImage;
                CurrentGif = CurrentGif->NextGif;
                if (_numFrames == 1) FirstGif->NextGif = CurrentGif;
                CurrentGif->NextGif = NULL;
              }
              ReadGraphExtension();
              break;

            case Comment_Ext:   
              ReadCommentExtension();
              break;

            case Applic_Ext:    
              ReadApplicationExtension();
              break;

            default:
              fprintf(stderr, "OGifImage: Unknown Extension %d\n", next);
              break;
          }
          break;

        case EndOfGif:
          go = 0;
          break;
 
        default:
          fprintf(stderr, "OGifImage: Unknown code: %02X at %lu\n", next, ftell(GifFile));
          if (_numFrames == 0) {
            fclose(GifFile);
            InvalidGif = True;
            return;
          } else {
            go = 0;
          }
          break;
      }
    }

    fclose(GifFile);

    CurrentGif = FirstGif;

    if (_bufferPixmap != None) XFreePixmap(display, _bufferPixmap);
    _bufferPixmap = XCreatePixmap(display, Root,
                                  GifScreen->_width,
                                  GifScreen->_height,
                                  _client->GetDisplayDepth());

    _bufferMask = XCreatePixmap(display, _client->GetRoot()->GetId(),
                                GifScreen->_width, GifScreen->_height, 1);
    XFillRectangle(display, _bufferMask,
                   clearGC, 0, 0, GifScreen->_width, GifScreen->_height); 

    DrawFrame(_bufferPixmap, copyGC,
              0, 0, GifScreen->_width, GifScreen->_height, 0, 0);
}


OGifImage::~OGifImage() {
  _GifImage *ptr, *next;

  ptr = FirstGif;
  while (ptr) {
    next = ptr->NextGif;
    if (ptr->Image)
      XDestroyImage(ptr->Image); // this frees both the image and image->data
    if (ptr->UseTransparentColor)
      XFreePixmap(display, ptr->TransparentMask);
    delete ptr;
    ptr = next;
  }

  if (GifScreen) delete GifScreen;

  if (_bufferPixmap != None) XFreePixmap(display, _bufferPixmap);
  if (_bufferMask != None) XFreePixmap(display, _bufferMask);

  XFreeGC(display, transGC);
  XFreeGC(display, clearGC);
  XFreeGC(display, copyGC);
}


void OGifImage::Draw(Drawable d, GC gc, int src_x, int src_y,
                     int src_w, int src_h, int dst_x, int dst_y) {
  XGCValues gcv;
  unsigned long gcm;

  if (_bufferPixmap != None) {
    if (CurrentGif->UseTransparentColor) {
      gcm = GCClipMask | GCClipXOrigin | GCClipYOrigin;
      gcv.clip_mask = _bufferMask;
      gcv.clip_x_origin = dst_x - src_x;
      gcv.clip_y_origin = dst_y - src_y;
      XChangeGC(display, gc, gcm, &gcv);
    }
    XCopyArea(display, _bufferPixmap, d, gc,
              src_x, src_y, src_w, src_h, dst_x, dst_y);
    gcm = GCClipMask;
    gcv.clip_mask = None;
    XChangeGC(display, gc, gcm, &gcv);
  } else {
    DrawFrame(d, gc, src_x, src_y, src_w, src_h, dst_x, dst_y);
  }
}

void OGifImage::DrawFrame(Drawable d, GC gc, int src_x, int src_y,
                          int src_w, int src_h, int dst_x, int dst_y) {
  XGCValues gcv;
  unsigned long gcm;

  if (CurrentGif->UseTransparentColor) {
    gcm = GCClipMask | GCClipXOrigin | GCClipYOrigin;
    gcv.clip_mask = CurrentGif->TransparentMask;
    gcv.clip_x_origin = CurrentGif->_leftpos;
    gcv.clip_y_origin = CurrentGif->_toppos;
    XChangeGC(display, transGC, gcm, &gcv);
  }

  XFillRectangle(display, _bufferMask, transGC,
                 CurrentGif->_leftpos, CurrentGif->_toppos,
                 CurrentGif->_width, CurrentGif->_height);

  gcm = GCClipMask;
  gcv.clip_mask = None;
  XChangeGC(display, transGC, gcm, &gcv);

  if (CurrentGif->UseTransparentColor) {
    gcm = GCClipMask | GCClipXOrigin | GCClipYOrigin;
    gcv.clip_mask = CurrentGif->TransparentMask;
    gcv.clip_x_origin = -src_x + CurrentGif->_leftpos;
    gcv.clip_y_origin = -src_y + CurrentGif->_toppos;
    XChangeGC(display, gc, gcm, &gcv);
  }

  XPutImage(display, d, gc,
            CurrentGif->Image,
            src_x, src_y,
            dst_x + CurrentGif->_leftpos, dst_y + CurrentGif->_toppos,
            src_w, src_h);

  gcm = GCClipMask;
  gcv.clip_mask = None;
  XChangeGC(display, gc, gcm, &gcv);
}

void OGifImage::DisposeCurrent(Drawable d) {
  XGCValues gcv;
  unsigned long gcm;

  if (CurrentGif->DisposalMethod == RestoreToBgnd) {
    unsigned long bgpixel;

    if (CurrentGif->UseTransparentColor) {
#if 1
      XFillRectangle(display, _bufferMask, clearGC,
                     CurrentGif->_leftpos, CurrentGif->_toppos,
                     CurrentGif->_width, CurrentGif->_height);
#else
      XCopyArea(display, CurrentGif->TransparentMask, _bufferMask, transGC,
                0, 0, CurrentGif->_width, CurrentGif->_height,
                CurrentGif->_leftpos, CurrentGif->_toppos);
#endif
    } else {
      XFillRectangle(display, _bufferMask, transGC,
                     CurrentGif->_leftpos, CurrentGif->_toppos,
                     CurrentGif->_width, CurrentGif->_height);
    }

    bgpixel = XPixelTable[GifScreen->_BackgroundColorIndex];

    XSetForeground(display, copyGC, bgpixel);

    //if (CurrentGif->UseTransparentColor) {
    //  gcm = GCClipMask | GCClipXOrigin | GCClipYOrigin;
    //  gcv.clip_mask = CurrentGif->TransparentMask;
    //  gcv.clip_x_origin = CurrentGif->_leftpos;
    //  gcv.clip_y_origin = CurrentGif->_toppos;
    //  XChangeGC(display, copyGC, gcm, &gcv);
    //}

    XFillRectangle(display, d, copyGC,
                   CurrentGif->_leftpos, CurrentGif->_toppos,
                   CurrentGif->_width, CurrentGif->_height);

    gcm = GCClipMask;
    gcv.clip_mask = None;
    XChangeGC(display, copyGC, gcm, &gcv);

  } else if (CurrentGif->DisposalMethod == RestoreToPrev) {

    // not yet implemented
    fprintf(stderr, "OGifImage: RestoreToPrev not implemented!\n");

  } else if (CurrentGif->DisposalMethod == DoNotDispose) {

    // nothing to do, just leave the buffer pixmap and mask as is.

  } else if (CurrentGif->DisposalMethod == NoDisposal) {

  }

}

void OGifImage::NextFrame() {
  if (_numFrames > 1) {
    if (_bufferPixmap != None)
      DisposeCurrent(_bufferPixmap);

    if (CurrentGif->NextGif == NULL)
      CurrentGif = FirstGif;
    else 
      CurrentGif = CurrentGif->NextGif;

#if 1
    if (CurrentGif->Image == NULL)  // some broken animated gifs contain
                                    // spurious graph ctrl ext blocks @ end
      CurrentGif = FirstGif;
#endif

    if (_bufferPixmap != None)
      DrawFrame(_bufferPixmap, copyGC,
                0, 0, CurrentGif->_width, CurrentGif->_height, 0, 0);
  }
}


//--- Read and allocate the colortable

void OGifImage::ReadColorTable(_color *Table, unsigned int TableSize) {
  fread(Table, 3, TableSize, GifFile);
  AllocColor(Table, TableSize);
}


//--- Reads and decodes the image

void OGifImage::ReadImage() {
  decode_LZW(CurrentGif->_width);
}


void OGifImage::ReadImageDescriptor() {
  unsigned char buf[16];
  int depth = DefaultDepth(display, screen), bitmap_pad;

  _numFrames++;
 
  if (depth <= 8)
    bitmap_pad = 8;
  else if (depth <= 16)
    bitmap_pad = 16;
  else
    bitmap_pad = 32;

  fread(buf, 9, 1, GifFile);
  // Set width height ....
  CurrentGif->_leftpos = LM_to_uint(buf[0], buf[1]);
  CurrentGif->_toppos  = LM_to_uint(buf[2], buf[3]);
  CurrentGif->_width   = LM_to_uint(buf[4], buf[5]);
  CurrentGif->_height  = LM_to_uint(buf[6], buf[7]);
  CurrentGif->_HasLocalColorTable = ((buf[8] & 0x80) == 0x80);
  CurrentGif->_isInterlaced = ((buf[8] & 0x40) == 0x40);
  CurrentGif->_SortedColors = ((buf[8] & 0x20) == 0x20);

  if (CurrentGif->_HasLocalColorTable) {
    CurrentGif->ColorTableSize = 2 << (buf[8] & 0x07);
    ReadColorTable(&LocalColorTable[0], CurrentGif->ColorTableSize);
  } else {
    CurrentGif->ColorTableSize = GifScreen->GlobalColorTableSize;
  }

  if (CurrentGif->UseTransparentColor) {
    CurrentGif->TransparentMask = XCreatePixmap(display,
                                                _client->GetRoot()->GetId(),
                                                CurrentGif->_width,
                                                CurrentGif->_height, 1);
    XFillRectangle(display, CurrentGif->TransparentMask,
                   clearGC, 0, 0, CurrentGif->_width, CurrentGif->_height); 
  }

  CurrentGif->Image = XCreateImage(display, DefaultVisual(display, screen),
                                   depth, ZPixmap, 0, NULL,
                                   CurrentGif->_width, CurrentGif->_height,
                                   bitmap_pad, 0);

  CurrentGif->Image->data =
         (char *) malloc(CurrentGif->Image->bytes_per_line *
                         CurrentGif->_height);

  if (CurrentGif->Image->data == NULL) {
    fprintf(stderr, "OGifImage: Failed to allocate memory, aborted.");
    InvalidGif = True;
    return;
  }

}


//--- Reads the next byte of the GifFile. Used by the decode_LZW routine

int OGifImage::get_byte() {
  return fgetc(GifFile);
}


//--- This function initializes the decoder for reading a new image.

short OGifImage::init_LZW(short size) {
  curr_size = size + 1;
  top_slot = 1 << curr_size;
  clear = 1 << size;
  ending = clear + 1;
  slot = newcodes = ending + 1;
  navail_bytes = nbits_left = 0;
  return 0;
}


//--- Get the next code from the GIF file. Return the code, or
//    a negative number in case of file errors.

short OGifImage::get_next_code() {
  short i, x;
  unsigned long ret;

  if (nbits_left == 0) {
    if (navail_bytes <= 0) {
      // Out of bytes in current block, so read next block
      pbytes = byte_buff;
      if ((navail_bytes = get_byte()) < 0)
        return navail_bytes;
      else if (navail_bytes) {
        for (i = 0; i < navail_bytes; ++i) {
          if ((x = get_byte()) < 0) return x;
          byte_buff[i] = x;
        }
      }
    }
    b1 = *pbytes++;
    nbits_left = 8;
    --navail_bytes;
  }

  ret = b1 >> (8 - nbits_left);

  while (curr_size > nbits_left) {
    if (navail_bytes <= 0) {
      // Out of bytes in current block, so read next block
      pbytes = byte_buff;
      if ((navail_bytes = get_byte()) < 0)
        return navail_bytes;
      else if (navail_bytes) {
        for (i = 0; i < navail_bytes; ++i) {
          if ((x = get_byte()) < 0) return x;
          byte_buff[i] = x;
        }
      }
    }
    b1 = *pbytes++;
    ret |= b1 << nbits_left;
    nbits_left += 8;
    --navail_bytes;
  }
  nbits_left -= curr_size;
  ret &= code_mask[curr_size];

  return ((short) ret);
}


//----------------------------------------------------------------------

// DECODE.C - An LZW decoder for GIF
//
// Copyright (C) 1987, by Steven A. Bennett
// Permission is given by the author to freely redistribute and include
// this code in any program as long as this credit is given where due.
// In accordance with the above, I want to credit Steve Wilhite who wrote
// the code which this is heavily inspired by...
//
// GIF and 'Graphics Interchange Format' are trademarks (tm) of
// Compuserve, Incorporated, an H&R Block Company.
//
// Release Notes: This file contains a decoder routine for GIF images
// which is similar, structurally, to the original routine by Steve Wilhite.
// It is, however, somewhat noticably faster in most cases.


// short decode_LZW(linewidth)
//    short linewidth - Pixels per line of image
//
// This function decodes an LZW image, according to the method used
// in the GIF spec. Every *linewidth* "characters" (ie. pixels) decoded
// will generate a call to out_line(), which is a user specific function
// to display a line of pixels. The function gets it's codes from
// get_next_code() which is responsible for reading blocks of data and
// separating them into the proper size codes. Finally, get_byte() is
// the global routine to read the next byte from the GIF file.
//
// It is generally a good idea to have linewidth correspond to the actual
// width of a line (as specified in the Image header) to make your own
// code a bit simpler, but it isn't absolutely necessary.
//
// Returns: 0 if successful, else negative. (See ERRS defined)

short OGifImage::decode_LZW(short linewidth) {
  register unsigned char *sp, *bufptr;
  unsigned char *buf;
  register short code, fc, oc, bufcnt;
  short c, size, ret;

  // Initialize for decoding a new image...
  if ((size = get_byte()) < 0) return(size);

  if (size < 2 || 9 < size) return(BAD_CODE_SIZE);

  init_LZW(size);

  // Initialize in case they forgot to put in a clear code.
  // (This shouldn't happen, but we'll try and decode it anyway...)
  oc = fc = 0;

  // Allocate space for the decode buffer
  if ((buf = (unsigned char *) malloc(linewidth + 1)) == NULL)
    return(OUT_OF_MEMORY);

  // Set up the stack pointer and decode buffer pointer
  sp = stack;
  bufptr = buf;
  bufcnt = linewidth;

  // This is the main loop.  For each code we get we pass through the
  // linked list of prefix codes, pushing the corresponding "character" for
  // each code onto the stack.  When the list reaches a single "character"
  // we push that on the stack too, and then start unstacking each character
  // for output in the correct order.  Special handling is included for the
  // clear code, and the whole thing ends when we get an ending code.

#if 0
  while ((c = get_next_code()) != ending) {
#else
  while (1) {
    if (lineheight >= CurrentGif->_height) break;
    c = get_next_code();
    if (c == ending) break;
#endif
 
    // If we had a file error, return without completing the decode
    if (c < 0) {
      free(buf);
      return(0);
    }

    // If the code is a clear code, reinitialize all necessary items.
    if (c == clear) {
      curr_size = size + 1;
      slot = newcodes;
      top_slot = 1 << curr_size;

      // Continue reading codes until we get a non-clear code
      // (Another unlikely, but possible case...)
      while ((c = get_next_code()) == clear) ;

      // If we get an ending code immediately after a clear code
      // (Yet another unlikely case), then break out of the loop.
      if (c == ending) break;

      // Finally, if the code is beyond the range of already set codes, 
      // (this had better NOT happen. I have no idea what will result from
      // this, but I doubt it will look good) then set it to color zero.
      if (c >= slot) c = 0;
      oc = fc = c;

      // And let us not forget to put the char into the buffer. And if, on
      // the off chance, we were exactly one pixel from the end of the line,
      // we have to send the buffer to the out_line() routine...
      *bufptr++ = c;
      if (--bufcnt == 0) {
        if ((ret = out_line(buf, linewidth)) < 0) {
          free(buf);
          return(ret);
        }
        bufptr = buf;
        bufcnt = linewidth;
      }

    } else {

      // In this case, it's not a clear code or an ending code, so
      // it must be a code code...  So we can now decode the code into
      // a stack of character codes. (Clear as mud, right?)
      code = c;

      // Here we go again with one of those off chances...  If, on the
      // off chance, the code we got is beyond the range of those already
      // set up (Another thing which had better NOT happen...) we trick
      // the decoder into thinking it actually got the last code read.
      // (Hmmn... I'm not sure why this works...  But it does...)
      if (code >= slot) {
        if (code > slot) ++bad_code_count;
        code = oc;
        *sp++ = fc;
      }

      // Here we scan back along the linked list of prefixes, pushing
      // helpless characters (ie. suffixes) onto the stack as we do so.
      while (code >= newcodes) {
        *sp++ = suffix[code];
        code = prefix[code];
      }

      // Push the last character on the stack, and set up the new
      // prefix and suffix, and if the required slot number is greater
      // than that allowed by the current bit size, increase the bit
      // size.  (NOTE - If we are all full, we *don't* save the new
      // suffix and prefix...  I'm not certain if this is correct...
      // it might be more proper to overwrite the last code...
      *sp++ = code;
      if (slot < top_slot) {
        suffix[slot] = fc = code;
        prefix[slot++] = oc;
        oc = c;
      }
      if ((slot >= top_slot) && (curr_size < 12)) {
        top_slot <<= 1;
        ++curr_size;
      } 

      // Now that we've pushed the decoded string (in reverse order) onto
      // the stack, lets pop it off and put it into our decode buffer.
      // And when the decode buffer is full, write another line...
      while (sp > stack) {
        *bufptr++ = *(--sp);
        if (--bufcnt == 0) {
          if ((ret = out_line(buf, linewidth)) < 0) {
            free(buf);
            return(ret);
          }
          bufptr = buf;
          bufcnt = linewidth;
        }
      }
    }
  }

  ret = 0;
  if (bufcnt != linewidth)
    ret = out_line(buf, (linewidth - bufcnt));

  free(buf);
  return(ret);
}


//--- Reads and skips the Application Extension

void OGifImage::ReadApplicationExtension() {
  char c, buf[20];

  // some broken gifs have a non-standard application extension block size
  // (e.g. some Adobe created gifs have app ext bs = 10, and not 11)

#if 0
  _ApplicationExtension AppExt;
  fread(&AppExt, /* sizeof(AppExt) */12, 1, GifFile);
#else
  c = get_byte();
  if (c < 0) return;
  fread(buf, c, 1, GifFile);
#endif
  ReadSubBlock();
}


//--- Reads the Graphic Extension and sets the approriate values in the
//    _Image struct

void OGifImage::ReadGraphExtension() {
  unsigned char buf[6];

  fread(buf, 5, 1, GifFile);

  CurrentGif->UseTransparentColor = ((buf[1] & 0x01) == 1);
  CurrentGif->DelayTime = LM_to_uint(buf[2], buf[3]);
  CurrentGif->TransparentColorIndex = buf[4];
  CurrentGif->DisposalMethod = (buf[1] >> 2) & 0x7;

  ReadTerminator();
}


//--- Reads and skips the Comment Extension

void OGifImage::ReadCommentExtension() {
  ReadSubBlock();
}


//--- Reads and skips the Plain Text Extension

void OGifImage::ReadPlainTextExtension() {
  ReadSubBlock();
}


//--- Reads the subblocks until a "stop mark" appear

int OGifImage::ReadSubBlock() {
  unsigned char BlockSize;
  unsigned char data[256];
  int c, totsize;

  c = get_byte();
  if (c < 0) return 0;
  BlockSize = (unsigned char) c;
  totsize = BlockSize;
  while (BlockSize != 0) {
    fread(&data, BlockSize, 1, GifFile);   // read subblock of size BlockSize
    c = get_byte();
    if (c < 0) return 0;
    BlockSize = (unsigned char) c;
    totsize += BlockSize;
  }

  return totsize;
}


int OGifImage::ReadTerminator() {
  int termbyte;

  termbyte = get_byte();
  if (termbyte <= 0) return True;  // Treat EOF as a valid terminator?

  fprintf(stderr, "OGifImage: Wrong Terminator: %X\n", termbyte);
  return False;
}


int OGifImage::out_line(unsigned char pixels[], int linelen) {
  XImage *image = CurrentGif->Image;

  if (lineheight >= CurrentGif->_height) {
    fprintf(stderr, "OGifImage: GIF line %d out of range (%d)\n",
                    lineheight, CurrentGif->_height);
    return 0; //-1;
  }

  if (CurrentGif->UseTransparentColor) {
    for (int i = 0; i < linelen; i++) {
      if (pixels[i] != CurrentGif->TransparentColorIndex) {
        XDrawPoint(display, CurrentGif->TransparentMask, transGC,
                   i, lineheight);
        XPutPixel(image, i, lineheight, XPixelTable[pixels[i]]);
      }
    }
  } else {
    for (int i = 0; i < linelen; i++)
      XPutPixel(image, i, lineheight, XPixelTable[pixels[i]]);
  }

  if (CurrentGif->_isInterlaced) {
    if ((lineheight + interlacediff + 1) > CurrentGif->_height) {
      interlacediff = 8 >> (interlacepass - 1);
      interlacepass++;
      lineheight = interlacediff / 2; 
    } else {
      lineheight += interlacediff;
    }
  } else {  // NonInterlaced gif
    lineheight++;
  }

  return 0;
}


//----------------------------------------------------------------------

//--- Allocate colors: based on the xgif program by John Bradley,
//    bradley@cis.ipenn.edu

void OGifImage::AllocColor(_color *Table, int ColorMapSize) {
  int Strip, Msk, i;

  for (i = 0; i < 256; i++)
    XPixelTable[i] = 0;  // Put reasonable color for out of range.

  for (Strip = 0, Msk = 0xff; Strip < 8; Strip++, Msk <<= 1) {
    for (i = 0; i < ColorMapSize; i++) {
      // Prepare color entry in X format.
      XColorTable[i].red = (Table[i].Red & Msk) << 8;
      XColorTable[i].green = (Table[i].Green & Msk) << 8;
      XColorTable[i].blue = (Table[i].Blue & Msk) << 8;
      XColorTable[i].flags = DoRed | DoGreen | DoBlue;
      if (XAllocColor(display, XColorMap, &XColorTable[i]))
        XPixelTable[i] = XColorTable[i].pixel;
      else
        break;
    }

    if (i < ColorMapSize)
      XFreeColors(display, XColorMap, (long unsigned int*) XPixelTable, i, 0L);
    else
      break;
  }

  if (Strip == 8)
    fprintf(stderr, "OGifImage: Error - not enough colors available.\n");

  if (Strip != 0)
    fprintf(stderr, "OGifImage: %d bits were stripped off the color map.\n",
            Strip);
}


// Routine to allocate the requested colors from the X server.
//
// Two stages are performed:
// 1. Colors are requested directly.
// 2. If not enough colors can be allocated, the closest current color
//    in current table is selected instead.
//
// This allocation is not optimal as when fail to allocate all colors one
// should pick the right colors to do allocate in order to minimize the
// closest distance from the unallocated ones under some norm (what is a good
// norm for the RGB space?). Improve it if you are bored.

void OGifImage::AllocClosestColor(int ColorMapSize) {
  int i, j, Index = 0, Count = 0, XNumOfColors;
  unsigned long D, Distance, AvgDistance = 0, Red, Green, Blue;
  bool Failed = false;
  XColor *XOldColorTable;

  for (i = 0; i < 256; i++) {
    if (i < ColorMapSize) {          // Prepere color entry in X format.
      XColorTable[i].red = GlobalColorTable[i].Red << 8;
      XColorTable[i].green = GlobalColorTable[i].Green << 8;
      XColorTable[i].blue = GlobalColorTable[i].Blue << 8;
      XColorTable[i].flags = DoRed | DoGreen | DoBlue;
      XPixelTable[i] = -1;           // Not allocated yet.
    } else {
      XPixelTable[i] = 0;            // Put reasonable color for out of range.
    }
  }

  for (i = 0; i < ColorMapSize; i++)   // Allocate the colors from X
    if (XAllocColor(display, XColorMap, &XColorTable[i]))
      XPixelTable[i] = XColorTable[i].pixel;
    else
      Failed = true;

  if (Failed) {
    XNumOfColors = DisplayCells(display, screen);
    XOldColorTable = (XColor *) malloc(sizeof(XColor) * XNumOfColors);
    for (i = 0; i < XNumOfColors; i++) XOldColorTable[i].pixel = i;
    XQueryColors(display, XColorMap, XOldColorTable, XNumOfColors);
	
    for (i = 0; i < ColorMapSize; i++) {
      // Allocate closest colors from X
      if (XPixelTable[i] == -1) {      // Failed to allocate this one.
        Distance = 0xffffffff;

        Red = XColorTable[i].red;
        Green = XColorTable[i].green;
        Blue = XColorTable[i].blue;

        for (j = 0; j < XNumOfColors; j++) {
          // Find the closest color in 3D RGB space using L1 norm.
          if ((D = abs(Red - XOldColorTable[j].red) +
                   abs(Green - XOldColorTable[j].green) +
                   abs(Blue - XOldColorTable[j].blue)) < Distance) {
            Distance = D;
            Index = j;
          }
        }
        XPixelTable[i] = Index;

        AvgDistance += Distance;
        Count++;
      }
    }
    free(XOldColorTable);

    fprintf(stderr, "OGifImage: Colors will be approximated (average error = %d).\n",
                    (int) AvgDistance / Count);
  }
}
