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

#ifndef __OXDDLISTBOX_H
#define __OXDDLISTBOX_H

#include <X11/Xlib.h>

#include <xclass/OXCompositeFrame.h>
#include <xclass/OXWidget.h>
#include <xclass/OXListBox.h>
#include <xclass/ODimension.h>
#include <xclass/OMessage.h>
#include <xclass/OString.h>


//-------------------------------------------------- Drop-down listbox

class OXDDPopup : public OXCompositeFrame {
public:
  OXDDPopup(const OXWindow *p, int w, int h,
            unsigned int options = VERTICAL_FRAME,
            unsigned long back = _defaultDocumentBackground);

  virtual int HandleButton(XButtonEvent *);
  virtual int HandleMotion(XMotionEvent *);
  virtual int HandleKey(XKeyEvent *);

  virtual int ProcessMessage(OMessage *msg);

  void PlacePopup(int x, int y, int w, int h);
  void EndPopup();
  
  int IsDown() const { return _isDown; }

protected:
  int _isDown, _kev;
};


class OXDDListBox : public OXCompositeFrame, public OXWidget {
protected:
  static const OPicture *_bpic;

public:
  OXDDListBox(const OXWindow *p, int ID,
              unsigned int options = HORIZONTAL_FRAME | SUNKEN_FRAME | DOUBLE_BORDER,
              unsigned long back = _defaultDocumentBackground);
  virtual ~OXDDListBox();

  virtual ODimension GetDefaultSize() const;

  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleKey(XKeyEvent *event);
  virtual int ProcessMessage(OMessage *msg);

  virtual void AddEntry(OString *s, int ID)
    { _lb->AddEntry(s, ID); }
  virtual void AddEntry(OXLBEntry *lbe, OLayoutHints *lhints)
    { _lb->AddEntry(lbe, lhints); }
  virtual void InsertEntry(OString *s, int ID, int afterID)
    { _lb->InsertEntry(s, ID, afterID); }
  virtual void InsertEntry(OXLBEntry *lbe, OLayoutHints *lhints, int afterID)
    { _lb->InsertEntry(lbe, lhints, afterID); }
  virtual void RemoveEntry(int ID)
    { _lb->RemoveEntry(ID); }
  virtual void RemoveEntries(int from_ID, int to_ID)
    { _lb->RemoveEntries(from_ID, to_ID); }
  virtual void RemoveAllEntries()
    { _lb->RemoveAllEntries(); }

  virtual const OXListBox *GetListBox() const { return _lb; }
  virtual void Select(int ID);
  virtual int  GetSelected() const { return _lb->GetSelected(); }
  virtual OXLBEntry *GetSelectedEntry() const
    { return _lb->GetSelectedEntry(); }

  virtual void SetTopEntry(OXLBEntry *e, OLayoutHints *lh);

  virtual void Reconfig();

protected:
  virtual void _GotFocus();
  virtual void _LostFocus();
  
  void _PopDown();

  OXLBEntry *_selentry;
  OXScrollBarElt *_ddbutton;
  OXDDPopup *_ddframe;
  OXListBox *_lb;
  OLayoutHints *_lhs, *_lhb, *_lhdd;
  OMessage *_message;
};


#endif  // __OXDDLISTBOX_H
