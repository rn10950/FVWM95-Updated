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

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
   
#include <xclass/utils.h>
#include <xclass/OXClient.h>
#include <xclass/OXCompositeFrame.h>
#include <xclass/OLayout.h>
#include <xclass/OResourcePool.h>
#include <xclass/OFocusManager.h>
#include <xclass/OGC.h>


//----------------------------------------------------------------

OXCompositeFrame::OXCompositeFrame(const OXWindow *p, int w, int h,
				   unsigned int options,
				   unsigned long back) : 
  OXFrame(p, w, h, options, back) {

    // by default
    _layoutManager = NULL;

    _flist = _ftail = NULL;

    if (_options & HORIZONTAL_FRAME)
      SetLayoutManager(new OHorizontalLayout(this));
    else
      SetLayoutManager(new OVerticalLayout(this));
}

// Delete this frame and all its children recursively...

OXCompositeFrame::~OXCompositeFrame() {
  SListFrameElt *ptr = _flist, *next;

  while (ptr) {
    next = ptr->next;
    delete ptr->frame;
    delete ptr;
    ptr = next;
  }
  if (_layoutManager) delete _layoutManager;
//  DestroyWindow();      // is it safe to do this here?
}

void OXCompositeFrame::SetLayoutManager(OLayoutManager *l) {
  if (_layoutManager) delete _layoutManager;
  _layoutManager = l;
}

void OXCompositeFrame::ChangeOptions(unsigned int options) {

  OXFrame::ChangeOptions(options);

  if (options & HORIZONTAL_FRAME)
    SetLayoutManager(new OHorizontalLayout(this));
  else
    SetLayoutManager(new OVerticalLayout(this));

}

void OXCompositeFrame::AddFrame(OXFrame *f, OLayoutHints *l) {
  SListFrameElt *nw/*, *ptr*/;  // ==!==

  // See remarks in OXClient.cc
  nw = new SListFrameElt;
  nw->frame = f;
  nw->layout = l;
  nw->next = NULL;
  nw->prev = NULL;

  if (_flist == NULL) {
    _flist = _ftail = nw;
  } else {
    _ftail->next = nw;
    nw->prev = _ftail;
    _ftail = nw;
  }

//  if (_flist != NULL) {
//    for (ptr=_flist; ptr->next != NULL; ptr=ptr->next);
//    ptr->next = nw;
//  } else _flist = nw;
}

// Remove a given frame from the list of children, and delete the
// list element, but do not delete the frame itself

void OXCompositeFrame::RemoveFrame(OXFrame *f) {
  SListFrameElt *ptr, *ptr2;  // ==!==

  for (ptr = _flist;
      (ptr != NULL) && (ptr->frame != f);
       ptr = ptr->next);
  if (ptr == NULL) return; // not found!

  if (ptr->prev == NULL) {
    _flist = ptr->next;
    if (_flist == NULL)
      _ftail = NULL;
    else
      _flist->prev = NULL;
  } else if (ptr->next == NULL) {
    ptr2 = ptr->prev;
    ptr2->next = NULL;
    _ftail = ptr2;
  } else {
    ptr2 = ptr->prev;
    ptr2->next = ptr->next;
    ptr2 = ptr->next;
    ptr2->prev = ptr->prev;
  }
  delete ptr;

//  if (_flist == NULL) return;
//
//  if (_flist->frame == f) {
//    ptr = _flist;
//    _flist = _flist->next;
//    delete ptr;
//  } else {
//    for (ptr = _flist;
//        (ptr->next != NULL) && (ptr->next->frame != f);
//         ptr = ptr->next);
//    if (ptr->next == NULL) return; // not found!
//    ptr2 = ptr->next;
//    ptr->next = ptr2->next;
//    delete ptr2;
//  }
}

void OXCompositeFrame::MapSubwindows() {
  SListFrameElt *ptr;

  OXWindow::MapSubwindows();
  for (ptr = _flist; ptr != NULL; ptr = ptr->next)
    ptr->frame->MapSubwindows();
}

void OXCompositeFrame::HideFrame(OXFrame *f) {
  if (f && (f->GetParent() == this)) {
    f->UnmapWindow();
    f->SetVisible(False);
    Layout();
  }
}

void OXCompositeFrame::ShowFrame(OXFrame *f) {
  if (f && (f->GetParent() == this)) {
    f->MapWindow();
    f->SetVisible(True);
    Layout();
  }
}

void OXCompositeFrame::Layout() {
  if (_layoutManager) _layoutManager->Layout();
}

OXFrame *OXCompositeFrame::GetFrameFromPoint(int x, int y) {
  SListFrameElt *ptr;
  OXFrame *f;

  if (!Contains(x, y)) return NULL;

  for (ptr = _flist; ptr != NULL; ptr = ptr->next) {
    if (ptr->frame->IsVisible()) {
      f = ptr->frame->GetFrameFromPoint(x - ptr->frame->GetX(),
                                        y - ptr->frame->GetY());
      if (f) return f;
    }
  }

  return this;
}

int OXCompositeFrame::TranslateCoordinates(OXFrame *child, int x, int y,
                                           int *fx, int *fy) {
  SListFrameElt *ptr;

  if (child == this) {
    if (fx) *fx = x;
    if (fy) *fy = y;
    return True;
  }

  if (!Contains(x, y)) return False;

  for (ptr = _flist; ptr != NULL; ptr = ptr->next) {
    if (child == ptr->frame) {
      if (fx) *fx = x - ptr->frame->GetX();
      if (fy) *fy = y - ptr->frame->GetY();
      return True;
    } else if (ptr->frame->IsComposite()) {
      if (((OXCompositeFrame *)ptr->frame)->TranslateCoordinates(child, 
              x - ptr->frame->GetX(), y - ptr->frame->GetY(), fx, fy))
        return True;
    }
  }

  return False;
}

void OXCompositeFrame::Reconfig() {
  SListFrameElt *ptr;

  OXFrame::Reconfig();

  for (ptr = _flist; ptr != NULL; ptr = ptr->next)
    ptr->frame->Reconfig();

//  Layout();  OXMainFrame would do a Resize(GetDefaultSize());
}
