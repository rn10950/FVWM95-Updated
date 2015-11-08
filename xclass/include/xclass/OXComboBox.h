/**************************************************************************

    This file is part of xclass.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

**************************************************************************/

#ifndef __OXCOMBOBOX_H
#define __OXCOMBOBOX_H

#include <X11/Xlib.h>

#include <xclass/OXCompositeFrame.h>
#include <xclass/OXWidget.h>
#include <xclass/OXTextEntry.h>
#include <xclass/OXListBox.h>
#include <xclass/OXDDListBox.h>
#include <xclass/OXButton.h>
#include <xclass/OMessage.h>


//----------------------------------------------------------------------

class OComboBoxMessage : public OListBoxMessage {
public:
  OComboBoxMessage(int typ, int act, int wid, int eid = -1,
                   int tot = 0, int sel = 0, int key = 0) : 
    OListBoxMessage(typ, act, wid, eid, tot, sel) {
      keysym = key;
    }
              
  int keysym;
};
                
//----------------------------------------------------------------------

class OXComboBox : public OXCompositeFrame, public OXWidget {
protected:
  static const OPicture *_bpic;

public:
  OXComboBox(const OXWindow *p, const char *text = NULL, int id = -1,
             unsigned int options = HORIZONTAL_FRAME | SUNKEN_FRAME |
                                    DOUBLE_BORDER | OWN_BKGND,
             unsigned long back = _defaultDocumentBackground);
  virtual ~OXComboBox();

  virtual ODimension GetDefaultSize() const;
  virtual int  HandleButton(XButtonEvent *event);
  virtual int  ProcessMessage(OMessage *msg);
  virtual void AddEntry(OString *s, int ID)
    { _lb->AddEntry(s, ID); }
  virtual void InsertEntry(OString *s, int ID, int afterID)
    { _lb->InsertEntry(s, ID, afterID); }
  virtual void RemoveEntry(int ID)
    { _lb->RemoveEntry(ID); }
  virtual void RemoveEntries(int from_ID, int to_ID)
    { _lb->RemoveEntries(from_ID, to_ID); }
  virtual void RemoveAllEntries()
    { _lb->RemoveAllEntries(); }
  virtual const OXListBox *GetListBox() const { return _lb; }
  virtual OXLBEntry *Select(int ID);
  virtual int  GetSelected() const { return _lb->GetSelected(); }
  virtual OXLBEntry *GetSelectedEntry() const
    { return _lb->GetSelectedEntry(); }

//  virtual void SetTopEntry(OXLBEntry *e, OLayoutHints *lh);
  virtual int HandleMotion(XMotionEvent *event)
    { return _te->HandleMotion(event); };
  virtual int HandleDoubleClick(XButtonEvent *event)
    { return _te->HandleDoubleClick(event); };

  const char *GetText() { return _te->GetString(); }
  void SetText(const char *text) { _te->Clear(); _te->AddText(0, text); }
  OXTextEntry *GetTextEntry() const { return _te; }

  virtual void Reconfig();

protected:
  void _PopDown();
  void _UpdateText(OXLBEntry *e);

  OXTextEntry *_te;
  OXScrollBarElt *_but;
  OXDDPopup *_dd;
  OXListBox *_lb;
  OLayoutHints *_l1, *_l2;
  OMessage *_message;
};


#endif  // __OXCOMBOBOX_H
