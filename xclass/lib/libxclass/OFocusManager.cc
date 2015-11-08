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

#include <X11/X.h>
#include <X11/Xlib.h>

#include <xclass/utils.h>
#include <xclass/OXClient.h>
#include <xclass/OFocusManager.h>
#include <xclass/OXCompositeFrame.h>


//----------------------------------------------------------------------

OFocusManager::OFocusManager(OXClient *client, OXFrame *focusRoot) {
  _client = client;
  _focusRoot = focusRoot;                                      
  _focusOwner = NULL;                                          
}

int OFocusManager::FocusNext(OXFrame *f) {
  OXFrame *target = f;

  if (!target || !target->GetParent()) return False;
         
  if (target->IsComposite()) {
    if (FocusForward((OXCompositeFrame *) target)) return True;
  }

  while (target != _focusRoot) {
    int found = False;
     
    OXFrame *p = (OXFrame *) target->GetParent();
    if (!p->IsComposite()) return False;
    SListFrameElt *ptr = *((OXCompositeFrame *) p)->GetListAddr();
  
    for ( ; ptr != NULL; ptr = ptr->next) {
      OXFrame *c = ptr->frame;
      if (found) {
        if (c->IsComposite()) {
          if (c->TakesFocus()) {
            if (AssignFocus(c)) return True;
          }
          if (FocusForward((OXCompositeFrame *) c)) return True;
        } else {
          if (AssignFocus(c)) return True;
        }
      } else if (c == target) {
        found = True;
      } 
    }

    target = p;
  }
    
  return False;
} 

int OFocusManager::FocusPrevious(OXFrame *f) {
  OXFrame *target = f;

  if (!target || !target->GetParent()) return False;

//  if (target->IsComposite()) {
//    if (FocusBackward((OXCompositeFrame *) target)) return True;
//  }

  while (target != _focusRoot) {
    int found = False;
    OXFrame *p = (OXFrame *) target->GetParent();
    if (!p->IsComposite()) return False;
    SListFrameElt *ptr = *((OXCompositeFrame *) p)->GetListTail();

    for ( ; ptr != NULL; ptr = ptr->prev) {
      OXFrame *c = ptr->frame;
      if (found) {
        if (c->IsComposite()) {
          if (FocusBackward((OXCompositeFrame *) c)) return True;
        } else {
          if (AssignFocus(c)) return True;
        }
      } else if (c == target) {
        found = True;
      }
    } 

    if (p->TakesFocus()) {
      if (AssignFocus(p)) return True;
    }

    target = p;
  }
    
  return False;
}     

int OFocusManager::AssignFocus(OXFrame *f) {
  if (f && f->TakesFocus() && f->IsVisible() && f->IsMapped() &&
           f->IsEnabled()) {
    f->RequestFocus();
    return True;
  }
  return False;
}
      
int OFocusManager::FocusForward(OXCompositeFrame *f) {
  SListFrameElt *ptr;

  if (!f) return False;

  for (ptr = *f->GetListAddr(); ptr != NULL; ptr = ptr->next) {
    OXFrame *c = ptr->frame;
    if (c->IsComposite()) {
      if (c->TakesFocus()) {
        if (AssignFocus(c)) return True;
      }
      if (FocusForward((OXCompositeFrame *) c)) return True;
    } else {   
      if (AssignFocus(c)) return True;
    }
  }
    
  return False; 
}  

int OFocusManager::FocusBackward(OXCompositeFrame *f) {
  SListFrameElt *ptr;

  if (!f) return False;

  for (ptr = *f->GetListTail(); ptr != NULL; ptr = ptr->prev) {
    OXFrame *c = ptr->frame;
    if (c->IsComposite()) {
      if (FocusBackward((OXCompositeFrame *) c)) return True;
    } else {
      if (AssignFocus(c)) return True;
    }
  }
  if (f->TakesFocus()) {
    if (AssignFocus(f)) return True;
  }
   
  return False;
}

OXFrame *OFocusManager::GetFocusOwner() {
  if (_focusOwner) {
    if (_client->GetWindowById(_focusOwnerId) != _focusOwner)
      _focusOwner = NULL;
  }
  return _focusOwner;
}

void OFocusManager::SetFocusOwner(OXFrame *f) {
  _focusOwner = f;
  if (f) _focusOwnerId = f->GetId();
}
