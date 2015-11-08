/**************************************************************************

    This file is part of xclass, a Win95-looking GUI toolkit.
    Copyright (C) 1996, 1997 David Barth, Hector Peraza.

    This code is partially based on Robert W. McMullen's ListTree-3.0
    widget. Copyright (C) 1995.

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
 
#ifndef __OXLISTTREE_H
#define __OXLISTTREE_H

#include <xclass/OXClient.h>
#include <xclass/OXWindow.h>
#include <xclass/OXView.h>
#include <xclass/OPicture.h>
#include <xclass/OMessage.h>

class OXFont;
class OTimer;


//----------------------------------------------------------------------

class OListTreeMessage : public OWidgetMessage {
public:
  OListTreeMessage(int typ, int act, int wid,
                   int b = 0, int xr = 0, int yr = 0) :
    OWidgetMessage(typ, act, wid) {
      button   = b;
      xroot    = xr;
      yroot    = yr;
  }

  int button, xroot, yroot;
};


//----------------------------------------------------------------------

class OListTreeItem : public OBaseObject {
public:
  OListTreeItem(OXClient *_client, const char *name,
                const OPicture *opened, const OPicture *closed);
  ~OListTreeItem();

  void Rename(const char *new_name);
  
  OListTreeItem	*parent, *firstchild, *prevsibling, *nextsibling;
  int  open, active;
  char *text;
  int  length, xnode, y, xpic, ypic, xtext, ytext, height, picWidth;
  const OPicture *open_pic, *closed_pic;

protected:
  OXClient *_client;
};


class OXListTree : public OXView {
public:
  OXListTree(const OXWindow *p, int w, int h, int id = -1,
             unsigned int options = SUNKEN_FRAME | DOUBLE_BORDER);
  virtual ~OXListTree();

  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleDoubleClick(XButtonEvent *event);
  virtual int HandleKey(XKeyEvent *event);
  virtual int HandleTimer(OTimer *t);

  void AddItem(OListTreeItem *parent, OListTreeItem *item);
  OListTreeItem *AddItem(OListTreeItem *parent, const char *string,
                         const OPicture *open = NULL,
                         const OPicture *closed = NULL);
  void RenameItem(OListTreeItem *item, const char *string);
  int  DeleteItem(OListTreeItem *item);
  int  DeleteChildren(OListTreeItem *item);
  int  Reparent(OListTreeItem *item, OListTreeItem *newparent);
  int  ReparentChildren(OListTreeItem *item, OListTreeItem *newparent);

  void OpenNode(OListTreeItem *node);
  void CloseNode(OListTreeItem *node);

  int  Sort(OListTreeItem *item);
  int  SortSiblings(OListTreeItem *item);
  int  SortChildren(OListTreeItem *item);
  void HighlightItem(OListTreeItem *item, int ensure_visible = False);
  void ClearHighlighted();
  void GetPathnameFromItem(OListTreeItem *item, char *path);
  void EnsureVisible(OListTreeItem *item);

  //static int _compare(const void *item1, const void *item2);

  OListTreeItem *GetFirstItem() const { return _first; }
  OListTreeItem *GetSelected() const { return _selected; }
  OListTreeItem *FindSiblingByName(OListTreeItem *item, const char *name);
  OListTreeItem *FindChildByName(OListTreeItem *item, const char *name);

  virtual int DrawRegion(OPosition coord, ODimension size, int clear = True);

protected:
  virtual void _GotFocus();
  virtual void _LostFocus();
  
  virtual void UpdateBackgroundStart();

  void _Draw(int yevent, int hevent);
  int  _DrawChildren(OListTreeItem *item, int x, int y, int xroot);
  void _DrawItem(OListTreeItem *item, int x, int y, int *xroot,
                 int *retwidth, int *retheight);
  void _DrawItemPic(OListTreeItem *item);
  void _DrawItemName(OListTreeItem *item);
  void _DrawNode(OListTreeItem *item, int x, int y);

  void _HighlightItem(OListTreeItem *item, int state, int draw);
  void _HighlightChildren(OListTreeItem *item, int state, int draw);
  void _UnselectAll(int draw);

  void ShowFocusHilite(int onoff);
  void _EnsureVisible(OListTreeItem *item);

  void _RemoveReference(OListTreeItem *item);
  void _DeleteChildren(OListTreeItem *item);
  void _InsertChild(OListTreeItem *parent, OListTreeItem *item);
  void _InsertChildren(OListTreeItem *parent, OListTreeItem *item);
  int  _SearchChildren(OListTreeItem *item, int y, int findy,
                       OListTreeItem **finditem);
  OListTreeItem *_FindItem(int findy);

  OListTreeItem *_first, *_selected;
  int _hspacing, _vspacing, _indent, _margin, _last_y;
  unsigned int _grayPixel;
  GC _drawGC, _lineGC, _highlightGC;
  const OXFont *_font;
  int _th, _ascent, _focused;
  int _exposeTop, _exposeBottom;
  OTimer *_timer;
};


#endif  // __OXLISTTREE_H
