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

// Routines used to render HTML onto the screen for the OXHtml widget.

#include <string.h>
#include <stdlib.h>

#include <xclass/OXHtml.h>


//----------------------------------------------------------------------

OHtmlBlock::OHtmlBlock() : OHtmlElement(Html_Block) {
  z = NULL;
  top = bottom = 0;
  left = right = 0;
  n = 0;
  pPrev = pNext = 0;
}

OHtmlBlock::~OHtmlBlock() {
  if (z) delete[] z;
}


// Destroy the given Block after first unlinking it from the element list.
// Note that this unlinks the block from the element list only -- not from
// the block list.

void OXHtml::UnlinkAndFreeBlock(OHtmlBlock *pBlock) {
  if (pBlock->pNext) {
    pBlock->pNext->pPrev = pBlock->pPrev;
  } else {
    pLast = pBlock->pPrev;
  }
  if (pBlock->pPrev) {
    pBlock->pPrev->pNext = pBlock->pNext;
  } else {
    pFirst = pBlock->pNext;
  }
  pBlock->pPrev = pBlock->pNext = 0;
  delete pBlock;
}


// Append a block to the block list and insert the block into the
// element list immediately prior to the element given.
//
// pToken - The token that comes after pBlock
// pBlock - The block to be appended

void OXHtml::AppendBlock(OHtmlElement *pToken, OHtmlBlock *pBlock) {
  pBlock->pPrev = pToken->pPrev;
  pBlock->pNext = pToken;
  pBlock->bPrev = lastBlock;
  pBlock->bNext = 0;
  if (lastBlock) {
    lastBlock->bNext = pBlock;
  } else {
    firstBlock = pBlock;
  }
  lastBlock = pBlock;
  if (pToken->pPrev) {
    pToken->pPrev->pNext = (OHtmlElement *) pBlock;
  } else {
    pFirst = (OHtmlElement *) pBlock;
  }
  pToken->pPrev = (OHtmlElement *) pBlock;
}


//----------------------------------------------------------------------

// Print an ordered list index into the given buffer. Use numbering
// like this:
//
//     A  B  C ... Y Z AA BB CC ... ZZ
//
// Revert to decimal for indices greater than 52.

static void GetLetterIndex(char *zBuf, int index, int isUpper) {
  int seed;

  if (index < 1 || index > 52) {
    sprintf(zBuf, "%d", index);
    return;
  }

  if (isUpper) {
    seed = 'A';
  } else {
    seed = 'a';
  }

  index--;

  if (index < 26) {
    zBuf[0] = seed + index;
    zBuf[1] = 0;
  } else {
    index -= 26;
    zBuf[0] = seed + index;
    zBuf[1] = seed + index;
    zBuf[2] = 0;
  }

  strcat(zBuf, ".");
}

// Print an ordered list index into the given buffer.  Use roman
// numerals.  For indices greater than a few thousand, revert to
// decimal.

static void GetRomanIndex(char *zBuf, int index, int isUpper) {
  int i = 0;
  int j;

  static struct {
    int value;
    char *name;
  } values[] = {
    { 1000, "m"  },
    {  999, "im" },
    {  990, "xm" },
    {  900, "cm" },
    {  500, "d"  },
    {  499, "id" },
    {  490, "xd" },
    {  400, "cd" },
    {  100, "c"  },
    {   99, "ic" },
    {   90, "xc" },
    {   50, "l"  },
    {   49, "il" },
    {   40, "xl" },
    {   10, "x"  },
    {    9, "ix" },
    {    5, "v"  },
    {    4, "iv" },
    {    1, "i"  },
  };

  if (index < 1 || index >= 5000) {
    sprintf(zBuf, "%d", index);
    return;
  }
  for (j = 0; index > 0 && j < sizeof(values) / sizeof(values[0]); j++) {
    int k;
    while (index >= values[j].value) {
      for (k = 0; values[j].name[k]; k++) {
        zBuf[i++] = values[j].name[k];
      }
      index -= values[j].value;
    }
  }
  zBuf[i] = 0;
  if (isUpper) {
    for (i = 0; zBuf[i]; i++) {
      zBuf[i] += 'A' - 'a';
    }
  }

  strcat(zBuf, ".");
}

// Draw the selection background for the given block
//
// x, y - Virtual coords of top-left of drawable

void OXHtml::DrawSelectionBackground(OHtmlBlock *pBlock, Drawable drawable,
                                     int x, int y) {
  int xLeft, xRight;        // Left and right bounds of box to draw
  int yTop, yBottom;        // Top and bottom of box
  OHtmlElement *p = 0;      // First element of the block
  OXFont *font;             // Font
  GC gc;                    // GC for drawing
  XRectangle xrec;          // Size of a filled rectangle to be drawn

  if (pBlock == 0 || (pBlock->flags & HTML_Selected) == 0) return;

  xLeft = pBlock->left - x;
  if (pBlock == pSelStartBlock && selStartIndex > 0) {
    if (selStartIndex >= pBlock->n) return;
    p = pBlock->pNext;
    font = GetFont(p->style.font);
    if (font == 0) return;
    if (p->type == Html_Text) {
      OHtmlTextElement *tp = (OHtmlTextElement *) p;
      xLeft = tp->x - x + font->TextWidth(pBlock->z, selStartIndex);
    }
  }
  xRight = pBlock->right - x;
  if (pBlock == pSelEndBlock && selEndIndex < pBlock->n) {
    if (p == 0) {
      p = pBlock->pNext;
      font = GetFont(p->style.font);
      if (font == 0) return;
    }
    if (p->type == Html_Text) {
      OHtmlTextElement *tp = (OHtmlTextElement *) p;
      xRight = tp->x - x + font->TextWidth(pBlock->z, selEndIndex);
    }
  }
  yTop = pBlock->top - y;
  yBottom = pBlock->bottom - y;
  gc = GetGC(COLOR_Selection, FONT_Any);
  xrec.x = xLeft;
  xrec.y = yTop;
  xrec.width = xRight - xLeft;
  xrec.height = yBottom - yTop;
  XFillRectangles(GetDisplay(), drawable, gc, &xrec, 1);
}


// Draw a rectangle. The rectangle will have a 3-D appearance if
// flat is 0 and a flat appearance if flat is 1.
//
// depth - width of the relief or the flat line

void OXHtml::DrawRect(Drawable drawable, OHtmlElement *src,
                      int x, int y, int w, int h, int depth, int relief) {
  if (depth > 0) {
    int i;
    GC gcLight, gcDark;
    XRectangle xrec[1];

    if (relief != HTML_RELIEF_FLAT) {
      int iLight, iDark;
      iLight = GetLightShadowColor(src->style.bgcolor);
      gcLight = GetGC(iLight, FONT_Any);
      iDark = GetDarkShadowColor(src->style.bgcolor);
      gcDark = GetGC(iDark, FONT_Any);
      if (relief == HTML_RELIEF_SUNKEN) {
        GC gcTemp = gcLight;
        gcLight = gcDark;
        gcDark = gcTemp;
      }
    } else {
      gcLight = GetGC(src->style.color, FONT_Any);
      gcDark = gcLight;
    }
    xrec[0].x = x;
    xrec[0].y = y;
    xrec[0].width = depth;
    xrec[0].height = h;
    XFillRectangles(GetDisplay(), drawable, gcLight, xrec, 1);
    xrec[0].x = x + w - depth;
    XFillRectangles(GetDisplay(), drawable, gcDark, xrec, 1);
    for (i = 0; i < depth && i < h/2; i++) {
      XDrawLine(GetDisplay(), drawable, gcLight, x+i, y+i, x+w-i-1, y+i);
      XDrawLine(GetDisplay(), drawable, gcDark, x+i, y+h-i-1, 
                x+w-i-1, y+h-i-1);
    }
  }
  if (h > depth*2 && w > depth*2) {
    GC gcBg;
    XRectangle xrec[1];
    gcBg = GetGC(src->style.bgcolor, FONT_Any);
    xrec[0].x = x + depth;
    xrec[0].y = y + depth;
    xrec[0].width = w - depth*2;
    xrec[0].height = h - depth*2;
    XFillRectangles(GetDisplay(), drawable, gcBg, xrec, 1);
  }
}


// Display a single HtmlBlock. This is where all the drawing happens.

void OXHtml::BlockDraw(OHtmlBlock *pBlock, Drawable drawable,
                       int drawableLeft, int drawableTop,
                       int drawableWidth, int drawableHeight,
                       Pixmap pixmap) {
  OXFont *font;           // Font to use to render text
  GC gc;                  // A graphics context
  OHtmlElement *src;      // OHtmlElement holding style information
  OHtmlTable *pTable;     // The table (when drawing part of a table)
  int x, y;               // Where to draw

  if (pBlock == 0) return;

  src = pBlock->pNext;
  while (src && (src->flags & HTML_Visible) == 0) src = src->pNext;

  if (src == 0) return;

  if (pBlock->n > 0) {
    // We must be dealing with plain old text
    if (src->type == Html_Text) {
      OHtmlTextElement *tsrc = (OHtmlTextElement *) src;
      x = tsrc->x;
      y = tsrc->y;
    } else {
      CANT_HAPPEN;
      return;
    }
    if (pBlock->flags & HTML_Selected) {
      DrawSelectionBackground(pBlock, drawable, drawableLeft, drawableTop);
    }
    gc = GetGC(src->style.color, src->style.font);
    font = GetFont(src->style.font);
    if (font == 0) return;
    font->DrawChars(GetDisplay(), drawable, gc, pBlock->z, pBlock->n,
                    x - drawableLeft, y - drawableTop);
    if (src->style.flags & STY_Underline) {
      font->UnderlineChars(GetDisplay(), drawable, gc, pBlock->z,
                        x - drawableLeft, y-drawableTop, 0, pBlock->n);
    }
    if (src->style.flags & STY_StrikeThru) {
      XRectangle xrec;
      xrec.x = pBlock->left - drawableLeft;
      xrec.y = (pBlock->top + pBlock->bottom) / 2 - drawableTop;
      xrec.width = pBlock->right - pBlock->left;
      xrec.height = 1 + (pBlock->bottom - pBlock->top > 15);
      XFillRectangles(GetDisplay(), drawable, gc, &xrec, 1);
    }
    if (pBlock == pInsBlock && insStatus > 0) {
      int x;
      XRectangle xrec;
      if (insIndex < pBlock->n) {
        OHtmlTextElement *tsrc = (OHtmlTextElement *) src;
        x = tsrc->x - drawableLeft;
        x += font->TextWidth(pBlock->z, insIndex);
      } else {
        x = pBlock->right - drawableLeft;
      }
      if (x > 0) --x;
      xrec.x = x;
      xrec.y = pBlock->top - drawableTop;
      xrec.width =  2;
      xrec.height = pBlock->bottom - pBlock->top;
      XFillRectangles(GetDisplay(), drawable, gc, &xrec, 1);
    }
  } else {
    // We are dealing with a single OHtmlElement which contains something
    // other than plain text.
    int cnt, w;
    char zBuf[30];
    OHtmlLi *li;
    OHtmlImageMarkup *image;
    switch (src->type) {
      case Html_LI:
        li = (OHtmlLi *) src;
        x = li->x;
        y = li->y;
        switch (li->ltype) {
          case LI_TYPE_Enum_1:
            sprintf(zBuf, "%d.", li->cnt);
            break;
          case LI_TYPE_Enum_A:
            GetLetterIndex(zBuf, li->cnt, 1);
            break;
          case LI_TYPE_Enum_a:
            GetLetterIndex(zBuf, li->cnt, 0);
            break;
          case LI_TYPE_Enum_I:
            GetRomanIndex(zBuf, li->cnt, 1);
            break;
          case LI_TYPE_Enum_i:
            GetRomanIndex(zBuf, li->cnt, 0);
            break;
          default:
            zBuf[0] = 0;
            break;
        }
        gc = GetGC(src->style.color, src->style.font);
        switch (li->ltype) {
          case LI_TYPE_Undefined:
          case LI_TYPE_Bullet1:
            XFillArc(GetDisplay(), drawable, gc,
                     x - 7 - drawableLeft, y - 8 - drawableTop, 7, 7,
                     0, 360*64);
            break;

          case LI_TYPE_Bullet2:
            XDrawArc(GetDisplay(), drawable, gc,
                     x - 7 - drawableLeft, y - 8 - drawableTop, 7, 7,
                     0, 360*64);
            break;

          case LI_TYPE_Bullet3:
            XDrawRectangle(GetDisplay(), drawable, gc,
                     x - 7 - drawableLeft, y - 8 - drawableTop, 7, 7);
            break;
          
          case LI_TYPE_Enum_1:
          case LI_TYPE_Enum_A:
          case LI_TYPE_Enum_a:
          case LI_TYPE_Enum_I:
          case LI_TYPE_Enum_i:
            cnt = strlen(zBuf);
            font = GetFont(src->style.font);
            if (font == 0) return;
            w = font->TextWidth(zBuf, cnt);
            font->DrawChars(GetDisplay(), drawable, gc, zBuf, cnt, 
                            x - w - drawableLeft, y - drawableTop);
            break;
        }
        break;

      case Html_HR: {
        OHtmlHr *hr = (OHtmlHr *) src;
        int relief = ruleRelief;
        switch (relief) {
          case HTML_RELIEF_RAISED: 
          case HTML_RELIEF_SUNKEN:
            break;
          default:
            relief = HTML_RELIEF_FLAT;
            break;
        }
        DrawRect(drawable, src, hr->x - drawableLeft, hr->y - drawableTop,
                 hr->w, hr->h, 1, relief);
        break;
      }

      case Html_TABLE: {
        OHtmlTable *table = (OHtmlTable *) src;
        int relief = tableRelief;
        if ((!bgImage || src->style.expbg) && !table->hasbg) {
          switch (relief) {
            case HTML_RELIEF_RAISED: 
            case HTML_RELIEF_SUNKEN:
              break;
            default:
              relief = HTML_RELIEF_FLAT;
              break;
          }
          DrawRect(drawable, src,
                   table->x - drawableLeft, table->y - drawableTop,
                   table->w, table->h, table->borderWidth, relief);
        }
        if (table->bgImage)
          DrawTableBgnd(table->x, table->y, table->w, table->h, pixmap,
                        table->bgImage);
        break;
      }

      case Html_TH:
      case Html_TD: {
        OHtmlCell *cell = (OHtmlCell *) src;
        int depth, relief;
        OImage *bgImg;
        pTable = cell->pTable;
        if ((!bgImage || src->style.expbg) && !(pTable && pTable->hasbg)) {
          depth = pTable && (pTable->borderWidth > 0);
          switch (tableRelief) {
            case HTML_RELIEF_RAISED:  relief = HTML_RELIEF_SUNKEN; break;
            case HTML_RELIEF_SUNKEN:  relief = HTML_RELIEF_RAISED; break;
            default:                  relief = HTML_RELIEF_FLAT;   break;
          }
          DrawRect(drawable, src,
                   cell->x - drawableLeft, cell->y - drawableTop,
                   cell->w, cell->h, depth, relief);
        }
        // See if row has an image
        if (cell->bgImage)
          DrawTableBgnd(cell->x, cell->y, cell->w, cell->h, pixmap,
                        cell->bgImage);
        else if (cell->pRow && (bgImg = ((OHtmlRef *)cell->pRow)->bgImage))
          DrawTableBgnd(cell->x, cell->y, cell->w, cell->h, pixmap,
                        bgImg);
        break;
      }

      case Html_IMG:
        image = (OHtmlImageMarkup *) src;
        if (image->pImage) {
          DrawImage(image, drawable, drawableLeft, drawableTop,
                    drawableLeft + drawableWidth,
                    drawableTop + drawableHeight);
        } else if (image->zAlt) {
          gc = GetGC(src->style.color, src->style.font);
          font = GetFont(src->style.font);
          if (font == 0) return;
          font->DrawChars(GetDisplay(), drawable, gc,
                          image->zAlt, strlen(image->zAlt),
                          image->x - drawableLeft, 
                          image->y - drawableTop);
        }    
        break;

      default:
        break;
    }
  }
}


// Draw all or part of an image.

void OXHtml::DrawImage(OHtmlImageMarkup *image, Drawable drawable,
                       int drawableLeft, int drawableTop,
                       int drawableRight, int drawableBottom) {
  int imageTop;          // virtual canvas coordinate for top of image
  int x, y;              // where to place image on the drawable
  int imageX, imageY;    // \__  Subset of image that fits
  int imageW, imageH;    // /    on the drawable

  imageTop = image->y - image->ascent;
  y = imageTop - drawableTop;
  if (imageTop + image->h > drawableBottom) {
    imageH = drawableBottom - imageTop;
  } else {
    imageH = image->h;
  }
  if (y < 0) {
    imageY = -y;
    imageH += y;
    y = 0;
  } else {
    imageY = 0;
  }
  x = image->x - drawableLeft;
  if (image->x + image->w > drawableRight) {
    imageW = drawableRight - image->x;
  } else {
    imageW = image->w;
  }
  if (x < 0) {
    imageX = -x;
    imageW += x;
    x = 0;
  } else {
    imageX = 0;
  }

  OImage *img = image->pImage->image;

  img->Draw(drawable, GetAnyGC(), imageX, imageY, imageW, imageH, x, y);
  XSync(GetDisplay(), False);

  image->redrawNeeded = 0;
}

void OXHtml::AnimateImage(OHtmlImage *image) {
  OImage *img = image->image;

  if (!img->IsAnimated()) return;
  img->NextFrame();
  delete image->timer;
  image->timer = new OTimer(this, img->GetAnimDelay());
  ImageChanged(image, image->w, image->h);
}


//----------------------------------------------------------------------

// Recompute the following fields of the given block structure:
//
//    base.count         The number of elements described by this
//                       block structure.
//
//    n                  The number of characters of text output
//                       associated with this block.  If the block
//                       renders something other than text (ex: <IMG>)
//                       then set n to 0.
//
//    z                  Pointer to malloced memory containing the
//                       text associated with this block.  NULL if
//                       n is 0.
//
// Return a pointer to the first OHtmlElement not covered by the block.

OHtmlElement *OXHtml::FillOutBlock(OHtmlBlock *p) {
  OHtmlElement *pElem;
  int go, i, n, x, y;
  SHtmlStyle style;
  int firstSelected;      // First selected character in this block
  int lastSelected;       // Last selected character in this block
  char zBuf[2000];

  // Reset n and z

  if (p->n) p->n = 0;

  if (p->z) delete[] p->z;
  p->z = 0;

  firstSelected = 1000000;
  lastSelected = -1;

  // Skip over OHtmlElements that aren't directly displayed.

  pElem = p->pNext;
  p->count = 0;
  while (pElem && (pElem->flags & HTML_Visible) == 0) {
    OHtmlElement *pNext = pElem->pNext;
    if (pElem->type == Html_Block) {
      UnlinkAndFreeBlock((OHtmlBlock *) pElem);
    } else {
      p->count++;
    }
    pElem = pNext;
  }
  if (pElem == 0) return 0;

  // Handle "special" elements.

  if (pElem->type != Html_Text) {
    switch (pElem->type) {
      case Html_HR: {
        OHtmlHr *hr = (OHtmlHr *) pElem;
        p->top    = hr->y - hr->h;
        p->bottom = hr->y;
        p->left   = hr->x;
        p->right  = hr->x + hr->w;
        break;
      }

      case Html_LI: {
        OHtmlLi *li = (OHtmlLi *) pElem;
        p->top    = li->y - li->ascent;
        p->bottom = li->y + li->descent;
        p->left   = li->x - 10;
        p->right  = li->x + 10;
        break;
      }

      case Html_TD:
      case Html_TH: {
        OHtmlCell *cell = (OHtmlCell *) pElem;
        p->top    = cell->y;
        p->bottom = cell->y + cell->h;
        p->left   = cell->x;
        p->right  = cell->x + cell->w;
        break;
      }

      case Html_TABLE: {
        OHtmlTable *table = (OHtmlTable *) pElem;
        p->top    = table->y;
        p->bottom = table->y + table->h;
        p->left   = table->x;
        p->right  = table->x + table->w;
        break;
      }

      case Html_IMG: {
        OHtmlImageMarkup *image = (OHtmlImageMarkup *) pElem;
        p->top    = image->y - image->ascent;
        p->bottom = image->y + image->descent;
        p->left   = image->x;
        p->right  = image->x + image->w;
        break;
      }
    }
    p->count++;

    return pElem->pNext;
  }

  // If we get this far, we must be dealing with text.

  OHtmlTextElement *text = (OHtmlTextElement *) pElem;
  n = 0;
  x = text->x;
  y = text->y;
  p->top = y - text->ascent;
  p->bottom = y + text->descent;
  p->left = x;
  style = pElem->style;
  go = 1;
  while (pElem) {
    OHtmlElement *pNext = pElem->pNext;
    switch (pElem->type) {
      case Html_Text: {
        OHtmlTextElement *txt = (OHtmlTextElement *) pElem;
        if (pElem->flags & STY_Invisible) {
          break;
        }
        if (txt->spaceWidth <= 0) {
          CANT_HAPPEN;
          break;
        }
        if (y != txt->y 
        ||  style.font != pElem->style.font
        ||  style.color != pElem->style.color
        ||  (style.flags & STY_FontMask) 
              != (pElem->style.flags & STY_FontMask)) {
          go = 0;
        } else {
          int sw = txt->spaceWidth;
          int nSpace = (txt->x - x) / sw;
          if (nSpace * sw + x != txt->x) {
            go = 0;
          } else if ((n + nSpace + pElem->count) >= sizeof(zBuf)) {
            // go = 0; - this caused a hang, instead lets do what we can
            for (i = 0; i < nSpace && (n+1) < sizeof(zBuf); ++i) {
              zBuf[n++] = ' ';
            }
            strncpy(&zBuf[n], txt->zText, sizeof(zBuf) - n - 1);
            zBuf[sizeof(zBuf)-1] = 0;
            n += i;
            x = txt->x + txt->w;
          } else {
            for (i = 0; i < nSpace && (n+1) < sizeof(zBuf); ++i) {
              zBuf[n++] = ' ';
            }
            strncpy(&zBuf[n], txt->zText, sizeof(zBuf) - n - 1);
            zBuf[sizeof(zBuf)-1] = 0;
            n += pElem->count;
            x = txt->x + txt->w;
          }
        }
        break;
      }

      case Html_Space:
        if (pElem->style.font != style.font) {
          pElem = pElem->pNext;
          go = 0;
        } else if ((style.flags & STY_Preformatted) != 0 &&
                   (pElem->flags & HTML_NewLine) != 0) {
          pElem = pElem->pNext;
          go = 0;
        }
        break;

      case Html_Block:
        UnlinkAndFreeBlock((OHtmlBlock *) pElem);
        break;

      case Html_A:
      case Html_EndA:
        go = 0;
        break;

      default:
        if (pElem->flags & HTML_Visible) go = 0;
        break;
    }
    if (go == 0) break;
    p->count++;
    pElem = pNext;
  }
  p->right = x;

  while (n > 0 && zBuf[n-1] == ' ') n--;
  p->z = new char[n+1];
  strncpy(p->z, zBuf, n);
  p->z[n] = 0;
  p->n = n;

  return pElem;
}


// Scan ahead looking for a place to put a block.  Return a pointer
// to the element which should come immediately after the block.
//
// if pCnt != 0, then put the number of elements skipped in *pCnt.
//
// p    - First candidate for the start of a block
// pCnt - Write number of elements skipped here

OHtmlElement *OXHtml::FindStartOfNextBlock(OHtmlElement *p, int *pCnt) {
  int cnt = 0;

  while (p && (p->flags & HTML_Visible) == 0) {
    OHtmlElement *pNext = p->pNext;
    if (p->type == Html_Block) {
      UnlinkAndFreeBlock((OHtmlBlock *) p);
    } else {
      cnt++;
    }
    p = pNext;
  }
  if (pCnt) *pCnt = cnt;

  return p;
}


// Add additional blocks to the block list in order to cover
// all elements on the element list.
//
// If any old blocks are found on the element list, they must
// be left over from a prior rendering.  Unlink and delete them.

void OXHtml::FormBlocks() {
  OHtmlElement *pElem;

  if (lastBlock) {
    pElem = FillOutBlock(lastBlock);
  } else {
    pElem = pFirst;
  }
  while (pElem) {
    int cnt;
    pElem = FindStartOfNextBlock(pElem, &cnt);
    if (pElem) {
      OHtmlBlock *pNew = new OHtmlBlock();
      if (lastBlock) {
        lastBlock->count += cnt;
      }
      AppendBlock(pElem, pNew);
      pElem = FillOutBlock(pNew);
    }
  }
}


// Draw table background

void OXHtml::DrawTableBgnd(int l, int t, int w, int h, 
                           Drawable pixmap, OImage *image) {
  int iw, ih, mx, my, dl, dt, dr, db, sh, sw, sx, sy, left, top,
      right, bottom, hd;

  left = l - _visibleStart.x;
  top = t - _visibleStart.y;

  dl = dirtyLeft;
  dt = dirtyTop;
  dr = dirtyRight;
  db = dirtyBottom;

  right = left + w - 1;
  bottom = top + h - 1;
  if (dr == 0 && db == 0) { dr = right; db = bottom; }
  if (left > dr || right < dl || top > db || bottom < dt) return;

  iw = image->GetWidth();
  ih = image->GetHeight();

#if 0
  if (iw < 4 && ih < 4) return;  // CPU burners we ignore.
  sx = (left + _visibleStart.x) % iw;   // X offset within image to start from
  sw = iw - sx;                         // Width of section of image to draw.
  for (mx = left - dl; w > 0; mx += sw, sw = iw, sx = 0) {
    if (sw > w) sw = w;
    sy = (top + _visibleStart.y) % ih;  // Y offset within image to start from
    sh = ih - sy;                       // Height of section of image to draw.
    for (my = top - dt, hd = h; hd > 0; my += sh, sh = ih, sy = 0) {
      if (sh > hd) sh = hd;
      // printf("image: %d %d %d %d %d %d\n", sx, sy, sw, sh, mx,my);
      image->Draw(pixmap, GetAnyGC(), sx, sy, sw, sh, mx, my);
      hd -= sh;
    }
    w -= sw;
  }
#else
  if (image->GetPicture() == None) return;
  GC gc = GetAnyGC();
  XGCValues gcv;
  unsigned int mask = GCTile | GCFillStyle |
                      GCTileStipXOrigin | GCTileStipYOrigin;
  gcv.tile = image->GetPicture();
  gcv.fill_style = FillTiled;
  gcv.ts_x_origin = -_visibleStart.x - dirtyLeft;
  gcv.ts_y_origin = -_visibleStart.y - dirtyTop;
  XChangeGC(GetDisplay(), gc, mask, &gcv);

  XFillRectangle(GetDisplay(), pixmap, gc, left - dl, top - dt, w, h);

  mask = GCFillStyle;
  gcv.fill_style = FillSolid;
  XChangeGC(GetDisplay(), gc, mask, &gcv);
#endif
}
