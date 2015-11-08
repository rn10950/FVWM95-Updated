/**************************************************************************

    This file is part of xclass, a Win95-looking GUI toolkit.
    Copyright (C) 1996, 1997 David Barth, Hector Peraza.

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

#ifndef __OXTEXTENTRY_H
#define __OXTEXTENTRY_H

#include <X11/Xlib.h>

#include <xclass/OXFrame.h>
#include <xclass/OXWidget.h>
#include <xclass/OMessage.h>
#include <xclass/OTextBuffer.h>


//----------------------------------------------------------------------

class OTextEntryMessage : public OWidgetMessage {
public:
  OTextEntryMessage(int typ, int act, int wid, int key = 0) :
    OWidgetMessage(typ, act, wid) {
      keysym = key;
  }

  int keysym;
};

//----------------------------------------------------------------------

class OXTextEntry : public OXFrame, public OXWidget {
protected:
  static const OXFont *_defaultFont;
  static const OXGC *_defaultGC, *_defaultSelGC, *_defaultSelBgndGC;
  static int _init;

protected:

public:
  OXTextEntry(const OXWindow *p, OTextBuffer *text = NULL, int ID = -1,
              unsigned int option = SUNKEN_FRAME | DOUBLE_BORDER,
              unsigned long back = _defaultDocumentBackground);
  virtual ~OXTextEntry();

  virtual int  HandleButton(XButtonEvent *event);
  virtual int  HandleDoubleClick(XButtonEvent *event);
  virtual int  HandleMotion(XMotionEvent *event);
  virtual int  HandleKey(XKeyEvent *event);
  virtual int  HandleCrossing(XCrossingEvent *event);
  virtual int  HandleSelection(XSelectionEvent *event);
  virtual int  HandleTimer(OTimer *t);
  virtual ODimension GetDefaultSize() const;
  virtual ODimension GetTextSize();

  unsigned GetTextLength() const { return _text->GetTextLength(); }
  unsigned GetBufferLength() const { return _text->GetBufferLength(); }
  const char *GetString() const { return _text->GetString(); }

  void AddText(int pos, const char *text, int sendmsg = False);
  void AddText(int pos, const char *text, int length, int sendmsg = False);
  void RemoveText(int pos, int length, int sendmsg = False);
  void Clear(int sendmsg = False);
  void SetCursor(int newpos);
  void SelectAll();

  void SetFont(const OXFont *font);
  void SetTextColor(unsigned int color);

  OTextBuffer *GetTextBuffer() const { return _text; }

protected:
  virtual void _GotFocus();
  virtual void _LostFocus();
  virtual void _Enable(int onoff);
  virtual int  _TextWidth(int begin, int length);
  virtual void _DrawString(GC gc, int x, int y, int begin, int length);
  virtual void _DrawImageString(GC gc, int x, int y, int begin, int length);

  void DrawCursor(bool mode);
  virtual void _DoRedraw();
  int  _GetCharacterIndex(int xcoord);
  int  _scrollDirection;
  void _PastePrimary(Window wid, Atom property, int Delete);

  OTextBuffer *_text;
  OTimer *_curblink, *_scrollselect;
  // selection
  int _start_ix, _start_x, _end_ix, _end_x, _selection_on;
  // cursor
  int _cursor_x, _cursor_ix, _cursor_on;  // ix stands for index (char string index)
  int _blinkTime;
  int _vstart_x, _dragging, _th, _ascent;
  int _select_on_focus;
  unsigned long _back;

  OXGC *_normGC, *_selGC, *_selbackGC;
  const OXFont *_font;
};


#endif  // __OXTEXTENTRY_H
