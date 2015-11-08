/**************************************************************************

    HTML widget for xclass. Based on tkhtml 1.28
    Copyright (C) 1997-2000 D. Richard Hipp <drh@acm.org>
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

// Routines used for processing <IMG> markup

#include <string.h>
#include <stdlib.h>

#include <xclass/OXHtml.h>
#include <xclass/OHtmlUri.h>
#include <xclass/OGifImage.h>


//----------------------------------------------------------------------

OHtmlImage::OHtmlImage(OXHtml *h, const char *url, const char *width,
                       const char *height) {
  _html = h;
  zUrl = StrDup(url);
  zWidth = StrDup(width);
  zHeight = StrDup(height);
  image = NULL;
  pNext = NULL;
  pList = NULL;
  w = 0;
  h = 0;
  timer = NULL;
}

OHtmlImage::~OHtmlImage() {
  delete[] zUrl;
  delete[] zWidth;
  delete[] zHeight;
  if (image) delete image;
  if (timer) delete timer;
}


//----------------------------------------------------------------------

// Find the alignment for an image

int OXHtml::GetImageAlignment(OHtmlElement *p) {
  char *z;
  int i;
  int result;

  static struct {
    char *zName;
    int iValue;
  } aligns[] = {
    { "bottom",    IMAGE_ALIGN_Bottom    },
    { "baseline",  IMAGE_ALIGN_Bottom    },
    { "middle",    IMAGE_ALIGN_Middle    },
    { "top",       IMAGE_ALIGN_Top       },
    { "absbottom", IMAGE_ALIGN_AbsBottom },
    { "absmiddle", IMAGE_ALIGN_AbsMiddle },
    { "texttop",   IMAGE_ALIGN_TextTop   },
    { "left",      IMAGE_ALIGN_Left      },
    { "right",     IMAGE_ALIGN_Right     },
  };

  z = p->MarkupArg("align", 0);
  result = IMAGE_ALIGN_Bottom;
  if (z) {
    for (i = 0; i < sizeof(aligns) / sizeof(aligns[0]); i++) {
      if (strcasecmp(aligns[i].zName, z) == 0) {
        result = aligns[i].iValue;
        break;
      }
    }
  }
  return result;
}

// This routine is called when an image changes. If the size of the
// images changes, then we need to completely redo the layout. If
// only the appearance changes, then this works like an expose event.

// pImage    - Pointer to an OHtmlImage object
// newWidth  - New width of the image
// newHeight - New height of the image

void OXHtml::ImageChanged(OHtmlImage *pImage, int newWidth, int newHeight) {
  OHtmlImageMarkup *pElem;

  if (pImage->w != newWidth || pImage->h != newHeight) {
    // We have to completely redo the layout after adjusting the size
    // of the images
    for (pElem = pImage->pList; pElem; pElem = pElem->iNext) {
      pElem->w = newWidth;
      pElem->h = newHeight;
    }
    flags |= RELAYOUT;
    pImage->w = newWidth;
    pImage->h = newHeight;
    RedrawEverything();
  } else {
#if 0
    for (pElem = pImage->pList; pElem; pElem = pElem->iNext) {
      pElem->redrawNeeded = 1;
    }
    flags |= REDRAW_IMAGES;
    ScheduleRedraw();
#else
    for (pElem = pImage->pList; pElem; pElem = pElem->iNext) {
      pElem->redrawNeeded = 1;
      DrawRegion(OPosition(pElem->x, pElem->y - pElem->ascent),
                 ODimension(pElem->w, pElem->h), False);
    }
#endif
  }
}


// Given an <IMG> markup, find or create an appropriate OHtmlImage
// object and return a pointer to that object. NULL might be returned.

OHtmlImage *OXHtml::GetImage(OHtmlImageMarkup *p) {
  char *zWidth;
  char *zHeight;
  char *zSrc;
  OHtmlImage *pImage;

  if (p->type != Html_IMG) { CANT_HAPPEN; return 0; }

  zSrc = p->MarkupArg("src", 0);
  if (zSrc == 0) return 0;

  zSrc = ResolveUri(zSrc);
  if (zSrc == 0) return 0;

  zWidth = p->MarkupArg("width", "");
  zHeight = p->MarkupArg("height", "");

  //p->w = atoi(zWidth);
  //p->h = atoi(zHeight);

  for (pImage = imageList; pImage; pImage = pImage->pNext) {
    if (strcmp(pImage->zUrl, zSrc) == 0 
    &&  strcmp(pImage->zWidth, zWidth) == 0
    &&  strcmp(pImage->zHeight, zHeight) == 0) {
      delete[] zSrc;
      return pImage;
    }
  }

  OImage *img = LoadImage(zSrc, atoi(zWidth), atoi(zHeight));

  if (img) {
    pImage = new OHtmlImage(this, zSrc, zWidth, zHeight);
    pImage->image = img;
    if (img->IsAnimated()) {
      pImage->timer = new OTimer(this, img->GetAnimDelay());
    }
    ImageChanged(pImage, img->GetWidth(), img->GetHeight());
    pImage->pNext = imageList;
    imageList = pImage;
  } else {
    pImage = 0;
  }

  delete[] zSrc;

  return pImage;
}


// This is the default LoadImage() procedure. It just tries to load the
// image from a file in the local filesystem.

OImage *OXHtml::LoadImage(const char *url, int w, int h) {
  OGifImage *image;

  OHtmlUri uri(url);

  image = new OGifImage(_client, uri.zPath);
  if (image->isInvalidGif()) {
    delete image;
    image = 0;   
  }
   
  return image;
}


// Return the height and width, converting to percent if required

char *OXHtml::GetPctWidth(OHtmlElement *p, char *opt, char *ret) {
  int n, m, val;
  char *tz, *z;
  OHtmlElement *pElem = p;

  z = pElem->MarkupArg(opt, "");
  if (!strchr(z, '%')) return z;
  if (!sscanf(z, "%d", &n)) return z;
  if (n < 0 || n > 100) return z;
  if (opt[0] == 'h')
    val = _canvas->GetHeight() * 100;
  else
    val = _canvas->GetWidth() * 100;
  if (!inTd) {
    sprintf(ret, "%d", val / n);
  } else {
    while (pElem && pElem->type != Html_TD) pElem = pElem->pPrev;
    if (!pElem) return z;
    tz = pElem->MarkupArg(opt, 0);
    if (tz && !strchr(tz, '%') && sscanf(tz, "%d", &m)) {
      sprintf(ret, "%d", m * 100 / n);
      return ret;
    }
    pElem = ((OHtmlCell *)pElem)->pTable;
    if (!pElem) return z;
    tz = pElem->MarkupArg(opt, 0);
    if (tz && !strchr(tz, '%') && sscanf(tz, "%d", &m)) {
      sprintf(ret, "%d", m * 100 / n);
      return ret;
    }
    return z;
  }
  return ret;
}


// This routine searchs for an image beneath the coordinates x,y
// and returns the token number of the the image, or -1 if no
// image found.

int OXHtml::GetImageAt(int x, int y) {
  OHtmlBlock *pBlock;
  OHtmlElement *pElem;
  int n;

  for (pBlock = firstBlock; pBlock; pBlock = pBlock->bNext) {
    if (pBlock->top > y || pBlock->bottom < y ||
        pBlock->left > x || pBlock->right < x) {
      continue;
    }
    for (pElem = pBlock->pNext; pElem; pElem = pElem->pNext) {
      if (pBlock->bNext && pElem == pBlock->bNext->pNext) break;
      if (pElem->type == Html_IMG) {
        return TokenNumber(pElem);
      }
    }
  }

  return -1;
}
