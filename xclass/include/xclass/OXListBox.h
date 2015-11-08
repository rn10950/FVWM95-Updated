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

#ifndef __OXLISTBOX_H
#define __OXLISTBOX_H

#include <X11/Xlib.h>

#include <xclass/OXCompositeFrame.h>
#include <xclass/OXCanvas.h>
#include <xclass/ODimension.h>
#include <xclass/OString.h>
#include <xclass/OTextBuffer.h>
#include <xclass/OPicture.h>
#include <xclass/OMessage.h>


class OXFont;
class OXGC;


//----------------------------------------------------------------------

class OListBoxMessage : public OWidgetMessage {
public:
  OListBoxMessage(int typ, int act, int wid, int eid = -1,
                  int tot = 0, int sel = 0) : 
    OWidgetMessage(typ, act, wid) {
      entryID  = eid;
      total    = tot;
      selected = sel;
  }

  int entryID, total, selected;
};


//----------------------------------------------------------------------

//--- Listbox (text strings only)

//--- Base class for listbox entries:

class OXLBEntry : public OXFrame {
public:
  OXLBEntry(const OXWindow *p, int ID,
            unsigned int options = HORIZONTAL_FRAME,
            unsigned long back = _defaultDocumentBackground) :
    OXFrame(p, 10, 10, options | OWN_BKGND, back) 
      { _active = _focused = False; _ID = ID; _bkcolor = back; }

  virtual void Activate(int a);
  virtual void UpdateEntry(OXLBEntry *e) {}  // this is needed on OXDDListBoxes :(
  int ID() const { return _ID; }
  int IsActive() const { return _active; }
  void ShowFocusHilite(int onoff) 
       { if (onoff != _focused) { _focused = onoff; NeedRedraw(); } }

protected:
  virtual void _DoRedraw() {}

  int _active, _ID, _bkcolor, _focused;
};

//--- Text string entries

class OXTextLBEntry : public OXLBEntry {
protected:
  static unsigned long _selPixel;
  static OXGC *_defaultGC;
  static const OXFont *_defaultFont;
  static int _init;

public:
  OXTextLBEntry(const OXWindow *p, OString *s, int ID,
                unsigned int options = HORIZONTAL_FRAME,
                unsigned long back = _defaultDocumentBackground);
  virtual ~OXTextLBEntry();

  virtual ODimension GetDefaultSize() const { return ODimension(_tw, _th+1); }
  const OString *GetText() const { return _text; }
  void SetText(OString *new_text);
  virtual void UpdateEntry(OXLBEntry *e)
    { SetText(new OString(((OXTextLBEntry *)e)->GetText())); }

protected:
  virtual void _DoRedraw();

  OXGC *_normGC;
  const OXFont *_font;
  OString *_text;
  int _tw, _th, _ta;
};

class OXLBContainer : public OXCompositeFrame {
public:
  OXLBContainer(const OXWindow *p, int w, int h,
                unsigned int options = SUNKEN_FRAME,
                unsigned long back = _defaultFrameBackground);
  virtual ~OXLBContainer();

  virtual void SetBackgroundColor(unsigned long color);

  virtual void AddEntry(OXLBEntry *lbe, OLayoutHints *lhints);
  virtual void InsertEntry(OXLBEntry *lbe, OLayoutHints *lhints, int afterID);
  virtual void RemoveEntry(int ID);
  virtual void RemoveEntries(int from_ID, int to_ID);
  virtual void RemoveAllEntries();

  virtual int  HandleKey(XKeyEvent *event);
  virtual int  HandleButton(XButtonEvent *event);
  virtual int  HandleMotion(XMotionEvent *event);
  virtual int  GetSelected() const;
  virtual OXLBEntry *GetSelectedEntry() const { return _last_active; }
  void MoveSelectDown(OXLBEntry *lbe);
  void MoveSelectUp(OXLBEntry *lbe);
  virtual void SelectAll();
  virtual void UnSelectAll();
  virtual OXLBEntry *Select(int ID);
  virtual OXLBEntry *FindEntry(int ID);
  virtual OXLBEntry *GetFirstEntry();

  void SetMultipleSelect(int mode) { _multipleSelect = mode; }

  void ShowFocusHilite(int onoff);

protected:
  OXLBEntry *_last_active;  // also anchor during multiple selects
  int _focused, _dragging, _multipleSelect;
};


//----------------------------------------------------------------------

class OXListBox : public OXCompositeFrame, public OXWidget {
public:
  OXListBox(const OXWindow *p, int ID,
            unsigned int options = SUNKEN_FRAME | DOUBLE_BORDER,
            unsigned long back = _defaultDocumentBackground);
  virtual ~OXListBox();

  virtual void SetBackgroundColor(unsigned long color);

  virtual void AddEntry(OString *s, int ID);
  virtual void AddEntry(OXLBEntry *lbe, OLayoutHints *lhints);
  virtual void InsertEntry(OString *s, int ID, int afterID);
  virtual void InsertEntry(OXLBEntry *lbe, OLayoutHints *lhints, int afterID);
  virtual void RemoveEntry(int ID);
  virtual void RemoveEntries(int from_ID, int to_ID);
  virtual void RemoveAllEntries();

  OXWindow *GetContainer() const
    { return ((OXWindow *) _vport->GetContainer()); }
  OXWindow *GetViewPort() const
    { return ((OXWindow *) _vport); }
  virtual void Resize(int w, int h);
  virtual void MoveResize(int x, int y, int w, int h);
  virtual void Layout();
  virtual void IntegralHeight(int mode) { _integralHeight = mode; }
  virtual ODimension GetDefaultSize() const;

  virtual int  ProcessMessage(OMessage *msg);
  virtual int  HandleKey(XKeyEvent *event);

  virtual OXLBEntry *Select(int ID);
  virtual OXLBEntry *FindEntry(int ID) { return _lbc->FindEntry(ID); }
  virtual int  GetSelected() const;
  virtual OXLBEntry *GetSelectedEntry() const
    { return _lbc->GetSelectedEntry(); }
  virtual OXLBEntry *GetFirstEntry() const
    { return _lbc->GetFirstEntry(); }

  virtual void SetMultipleSelect(int mode)
    { _lbc->SetMultipleSelect(mode); }

  virtual void MoveSelectDown(OXLBEntry *lbe)
    { _lbc->MoveSelectDown(lbe); }
  virtual void MoveSelectUp(OXLBEntry *lbe)
    { _lbc->MoveSelectUp(lbe); }


  void AutoUpdate(int mode) { _autoUpdate = mode; }
  virtual void Update();

protected:
  void _SetContainer(OXFrame *f) { _vport->SetContainer(f); }
  virtual void _GotFocus();
  virtual void _LostFocus();
  void _EnsureVisible(OXLBEntry *e);

  OXLBContainer *_lbc;
  OXViewPort *_vport;
  OXVScrollBar *_vscrollbar;
  int _itemVsize, _integralHeight, _autoUpdate;
};


#endif  // __OXLISTBOX_H
