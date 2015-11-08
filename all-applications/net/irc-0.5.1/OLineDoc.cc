#include <unistd.h>
#include <ctype.h>

#include <xclass/OGC.h>

#include "OLineDoc.h"
#include "OXViewDoc.h"
#include "OXPreferences.h"

extern OSettings *foxircSettings;


//----------------------------------------------------------------------

OLineDoc::OLineDoc() {
  prev = next = this;
  _w = _h = 0;
  lnlen = 0;
}

OLineDoc::~OLineDoc() {
  Clear();
}

OLineDoc *OLineDoc::InsertBefore(OLineDoc *l) {
  l->prev = prev;
  l->next = this;
  prev->next = l;
  prev = l;
  return l;
}

OLineDoc *OLineDoc::InsertAfter(OLineDoc *l) {
  l->prev = this;
  l->next = next;
  next->prev = l;
  next = l;
  return l;
}

void OLineDoc::SetCanvas(OXViewDocFrame *c) {
  _textFrame = c;
  _canvas = c;
  _defaultFg = foxircSettings->GetPixelID(P_COLOR_TEXT);  // white
}

void OLineDoc::SetDefaultColor(unsigned long color) {
  _defaultFg = color;
}

void OLineDoc::Clear() {
  delete[] chars;
  delete[] color;
  delete[] attrib;
  lnlen = 0;
}

int OLineDoc::Fill(const char *buf) {
  int cnt;
  const char *src;
  char line[MAXLINESIZE], colr[MAXLINESIZE], atrb[MAXLINESIZE];
  char ca = ATTRIB_NORMAL, cc = 0xFF;
  unsigned char c;

  OXGC *gc = _textFrame->GetGC();

  OFontMetrics fm;
  _textFrame->GetFont()->GetFontMetrics(&fm);

  int lh = fm.ascent + fm.descent;

  src = buf;
  cnt = 0;
  while ((c = (unsigned char) *src++)) {
    if (c == 0x09) {
      do {
        line[cnt] = ' ';
        colr[cnt] = cc;
        atrb[cnt] = ca;
        ++cnt;
      } while ((cnt & 0x7) && (cnt < MAXLINESIZE-1));
    } else if (c == 0x02) {  // toggle bold attribute
      if (ca & ATTRIB_BOLD)
        ca &= ~ATTRIB_BOLD;
      else
        ca |= ATTRIB_BOLD;
    } else if (c == 0x16) {  // toggle reverse attribute
      if (ca & ATTRIB_REVERSE)
        ca &= ~ATTRIB_REVERSE;
      else
        ca |= ATTRIB_REVERSE;
    } else if (c == 0x1F) {  // toggle underline attribute
      if (ca & ATTRIB_UNDERLINE)
        ca &= ~ATTRIB_UNDERLINE;
      else
        ca |= ATTRIB_UNDERLINE;
    } else if (c == 0x0F) {  // clear all attributes
      ca = ATTRIB_NORMAL;
    } else if (c == 0x03) {  // set mirc color or clear color attribute
      if (!isdigit(*src)) {
        ca &= ~(ATTRIB_FGCOLOR | ATTRIB_BGCOLOR);
      } else {
        int fg, bg;
        fg = *src++ - '0';
        if (isdigit(*src)) fg = fg*10 + (*src++ - '0');
        ca |= ATTRIB_FGCOLOR;
        cc &= 0x0F;
        cc |= (char) ((fg << 4) & 0xF0);
        if (*src == ',') {
          ++src;
          if (isdigit(*src)) {
            bg = *src++ - '0';
            if (isdigit(*src)) bg = bg*10 + (*src++ - '0');
            ca |= ATTRIB_BGCOLOR;
            cc &= 0xF0;
            cc |= (char) bg & 0x0F;
          }
        }
      }
    } else if (c == 0x1B) {  // ANSI color attribute
      if (*src) {
        if (*src++ == '[') {
          int i, j, attr[10];
          for (i = 0; i < 10; ++i) attr[i] = 0;
          i = 0;
          while (1) {
            while (isdigit(*src)) attr[i] = attr[i]*10 + (*src++ - '0');
            if (i < 10) ++i;
            if (*src != ';') break;
            ++src;
          }
          if (*src == 'm') {
            for (j = 0; j < i; ++j) {
              switch (attr[j]) {
                case 0: ca = ATTRIB_NORMAL; break;
                case 1: ca |= ATTRIB_BOLD; break;
                case 2: ca |= ATTRIB_UNDERLINE; break;
                case 7: ca |= ATTRIB_REVERSE; break;
                case 22: ca &= ~ATTRIB_BOLD; break;
                case 24: ca &= ~ATTRIB_UNDERLINE; break;
                case 27: ca &= ~ATTRIB_REVERSE; break;
                case 30: ca |= ATTRIB_FGCOLOR; cc = (cc & 0x0F) | 0x10; break;
                case 31: ca |= ATTRIB_FGCOLOR; cc = (cc & 0x0F) | 0x40; break;
                case 32: ca |= ATTRIB_FGCOLOR; cc = (cc & 0x0F) | 0x90; break;
                case 33: ca |= ATTRIB_FGCOLOR; cc = (cc & 0x0F) | 0x80; break;
                case 34: ca |= ATTRIB_FGCOLOR; cc = (cc & 0x0F) | 0xc0; break;
                case 35: ca |= ATTRIB_FGCOLOR; cc = (cc & 0x0F) | 0xd0; break;
                case 36: ca |= ATTRIB_FGCOLOR; cc = (cc & 0x0F) | 0xb0; break;
                case 37: ca |= ATTRIB_FGCOLOR; cc = (cc & 0x0F) | 0x00; break;
                case 40: ca |= ATTRIB_BGCOLOR; cc = (cc & 0xF0) | 0x01; break;
                case 41: ca |= ATTRIB_BGCOLOR; cc = (cc & 0xF0) | 0x04; break;
                case 42: ca |= ATTRIB_BGCOLOR; cc = (cc & 0xF0) | 0x09; break;
                case 43: ca |= ATTRIB_BGCOLOR; cc = (cc & 0xF0) | 0x08; break;
                case 44: ca |= ATTRIB_BGCOLOR; cc = (cc & 0xF0) | 0x0c; break;
                case 45: ca |= ATTRIB_BGCOLOR; cc = (cc & 0xF0) | 0x0d; break;
                case 46: ca |= ATTRIB_BGCOLOR; cc = (cc & 0xF0) | 0x0b; break;
                case 47: ca |= ATTRIB_BGCOLOR; cc = (cc & 0xF0) | 0x00; break;
              }
            }
          }
          if (*src) ++src;
        }
      }
    } else if (c > 0x1F) {
      line[cnt] = c;
      colr[cnt] = cc;
      atrb[cnt] = ca;
      ++cnt;
    }
    if (cnt >= MAXLINESIZE-1) break;
  }
  line[cnt] = '\0';
  lnlen = strlen(line);
  chars  = new char[lnlen + 1];
  color  = new char[lnlen + 1];
  attrib = new char[lnlen + 1];
  strcpy(chars, line);
  memcpy(color,  colr, lnlen);
  memcpy(attrib, atrb, lnlen);
  _h = lh * (int)(lnlen / 80) + lh;
  _w = 80;
  return True;
}

int OLineDoc::LoadFile(FILE *file) {
  char buf[MAXLINESIZE];

  if (fgets(buf, MAXLINESIZE, file) != NULL) {
    Fill(buf);
    return True;
  } else
    return False;
}

void OLineDoc::Layout() {
  char *p = chars, *prev = chars, *chunk = chars;
  int tw, nlines;
  const OXFont *font = _textFrame->GetFont();

  OXGC *gc = _textFrame->GetGC();

  OFontMetrics fm;
  font->GetFontMetrics(&fm);

  _h = fm.ascent + fm.descent;

  nlines = 1;

  int w = (_w = _textFrame->GetDocWidth()) - (2 * MARGIN);

  tw = _textFrame->GetFont()->XTextWidth(chars, lnlen);
  if (tw <= w) return;

  while (1) {
    p = strchr(p, ' ');
    if (p == NULL) {
      tw = font->XTextWidth(chunk);
      if (tw > w) ++nlines;
      break;
    }
    tw = font->XTextWidth(chunk, p-chunk);
    if (tw > w) {
      if (prev == chunk)
        chunk = prev = ++p;
      else
        p = chunk = prev;
      ++nlines;
    } else {
      prev = ++p;
    }
  }

  _h *= nlines;
}

void OLineDoc::DrawRegion(Display *dpy, Drawable d,
                          int x, int y, XRectangle *rect) {
  char *p = chars, *prev = chars, *chunk = chars;
  int tw, th;

  OXGC *gc = _textFrame->GetGC();
  const OXFont *font = _textFrame->GetFont();

#if 1
  gc->SetForeground(_defaultFg);
#endif

  OFontMetrics fm;
  font->GetFontMetrics(&fm);

  int w = (_w = _textFrame->GetDocWidth()) - (2 * MARGIN);

  y += fm.ascent;

  tw = font->XTextWidth(chars, lnlen);
  if (tw <= w) {
    DrawLine(dpy, d, x+MARGIN, y, 0, lnlen);
    return;
  }

  th = fm.ascent + fm.descent;

  while (1) {
    p = strchr(p, ' ');
    if (p == NULL) {
      tw = font->XTextWidth(chunk);
      if (tw > w) {
        DrawLine(dpy, d, x+MARGIN, y, chunk-chars, prev-chunk-1);
        y += th;
        DrawLine(dpy, d, x+MARGIN, y, prev-chars, strlen(prev));
      } else {
        DrawLine(dpy, d, x+MARGIN, y, chunk-chars, strlen(chunk));
      }
      break;
    }
    tw = font->XTextWidth(chunk, p-chunk);
    if (tw > w) {
      if (prev == chunk)
        prev = ++p;
      else
        p = prev;
      DrawLine(dpy, d, x+MARGIN, y, chunk-chars, prev-chunk-1);
      chunk = prev;
      y += th;
    } else {
      prev = ++p;
    }
  }
}

void OLineDoc::DrawLine(Display *dpy, Drawable d, int x, int y,
                        int from, int len) {
  int i;
  int xl = x;

  len += from;
  for (i = from; i < len; ++i) {
    if ((attrib[i] != attrib[from]) || (color[i] != color[from])) {
      xl += DrawLineSegment(dpy, d, xl, y,
                            chars+from, i-from, attrib[from], color[from]);
      from = i;
    }
  }

  if (from != i)
    DrawLineSegment(dpy, d, xl, y,
                    chars+from, i-from, attrib[from], color[from]);
}

int OLineDoc::DrawLineSegment(Display *dpy, Drawable d,
                              int x, int y,
                              char *str, int len,
                              char attrib, char color) {
  int  tw;
  OXGC *gc = _textFrame->GetGC();
  unsigned long oldfg = gc->GetForeground(),
                oldbg = gc->GetBackground(),
                fg, bg, tmp;

  tw = _textFrame->GetFont()->XTextWidth(str, len);
  fg = oldfg;
  bg = oldbg;

#if 1
  fg = _defaultFg;
  gc->SetForeground(fg);
#endif

  if (attrib & ATTRIB_FGCOLOR)
    fg = foxircSettings->GetIrcPixel((color >> 4) & 0x0F);

  if (attrib & ATTRIB_BGCOLOR)
    bg = foxircSettings->GetIrcPixel(color & 0x0F);

  if (attrib & ATTRIB_REVERSE) {
    tmp = fg;
    fg = bg;
    bg = tmp;
  }

  if (oldfg != fg) gc->SetForeground(fg);
  if (oldbg != bg) gc->SetBackground(bg);

  if (attrib & (ATTRIB_BGCOLOR | ATTRIB_REVERSE))
    XDrawImageString(dpy, d, gc->GetGC(), x, y, str, len);
  else
    XDrawString(dpy, d, gc->GetGC(), x, y, str, len);

  if (attrib & ATTRIB_BOLD)
    XDrawString(dpy, d, gc->GetGC(), x+1, y, str, len);

  if (attrib & ATTRIB_UNDERLINE)
    XDrawLine(dpy, d, gc->GetGC(), x, y, x+tw-1, y);

  if (oldfg != fg) gc->SetForeground(oldfg);
  if (oldbg != bg) gc->SetBackground(oldbg);

  return tw;
}
