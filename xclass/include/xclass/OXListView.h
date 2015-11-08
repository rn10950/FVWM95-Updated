/**************************************************************************

    This file is part of xclass.
    Copyright (C) 1996-2000 Harald Radke, Hector Peraza.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

**************************************************************************/

#ifndef __OXLISTVIEW_H
#define __OXLISTVIEW_H

#include <xclass/utils.h>
#include <xclass/OResourcePool.h>
#include <xclass/OXFont.h>
#include <xclass/OXTextButton.h>
#include <xclass/OXResizer.h>
#include <xclass/OPicture.h>
#include <xclass/OSelectedPicture.h>
#include <xclass/OXItemView.h>


#define LV_LARGE_ICONS  0
#define LV_SMALL_ICONS  1
#define LV_LIST         2
#define LV_DETAILS      3

#define LV_UNSORTED     0
#define LV_ASCENDING    1
#define LV_DESCENDING   2


struct SColumnData {
  OXTextButton *button;
  OXVerticalResizer *resizer;
  int index, width, user_width, alignment, sort_mode;
};


//----------------------------------------------------------------------

class OXListView;

class OListViewItem : public OItem {
public:
  OListViewItem(const OXListView *p, int id, const OPicture *bigpic,
                const OPicture *smallpic, std::vector <OString *>names,
                int viewMode, int fullLine = False);
  virtual ~OListViewItem();

  virtual void SetViewMode(int viewMode, int fullLine = False);

  void SetFont(const OXFont *font);

  int GetViewMode() const { return _viewMode; }
  OString *GetName(int column = 0) const
           { return (column >= (int) _names.size()) ? NULL : _names[column]; }
  const OPicture *GetCurrentPicture() const { return _currentPic; }
  const OPicture *GetBigPicture() const { return _bigPic; }
  const OPicture *GetSmallPicture() const { return _smallPic; }

  std::vector<OString *>& GetNames() /*const*/ { return _names; }
  
  virtual ODimension GetDefaultSize() const;
  
  virtual int GetDefaultColumnWidth(int column) const;

  virtual void Draw(OXWindow *w, OPosition pos);
  virtual void DrawTruncated(OXWindow *w, GC gc, OPosition pos,
                             OString *s, int width);
  virtual void Layout();
  
  virtual int Compare(const OListViewItem *item, int column) const;

  friend class OXListView;

protected:
  OPosition _iconPos, _textPos;  // relative positions
  int _viewMode, _fullRowSelect, _tw, _th, _ta;

  std::vector<OString *> _names;

  static const OXFont *_defaultFont;
  const OXFont *_font;
  const OPicture *_bigPic, *_smallPic, *_currentPic;
  const OSelectedPicture *_selectedPic;

  static const OXGC *_dfselGC, *_dfhilightGC, *_dfnormGC;
  OXGC *_selGC, *_hilightGC, *_normGC;
};


//----------------------------------------------------------------------

class OXListView : public OXItemView {
public:
  OXListView(const OXWindow *p, int w, int h, int id,
             unsigned int options = SUNKEN_FRAME | DOUBLE_BORDER | OWN_BKGND,
             unsigned int sboptions = CANVAS_SCROLL_BOTH);
  virtual ~OXListView();

  virtual int HandleKey(XKeyEvent *event);
  virtual int HandleButton(XButtonEvent *event);

  virtual int ProcessMessage(OMessage *msg);

  virtual void SetViewMode(int ViewMode, int FullLine = False, int Grid = False);

  virtual void AddItem(OListViewItem *item);
  virtual void AddItem(OItem *item) {}
  virtual void DeleteItem(unsigned int num);
  virtual void Layout();

  void AddColumn(OString *name, int index, int align = TEXT_CENTER_X);

  int GetViewMode() const { return _viewMode; }
  std::vector<SColumnData *>& GetColumns() /*const*/ { return _columnData; }

  void SortColumn(int column, int mode);
  void SetAutoSort(int mode = True) { _autoSort = mode; }
  int  GetAutoSort() const { return _autoSort; }
  
  int GetSortColumn() const { return _sortColumn; }

  void SetHeaderFont(const OXFont *font);
  void SetItemFont(const OXFont *font);
  void SetColumnWidth(int column, int width);
  int  GetColumnWidth(int column);
  
  virtual int DrawRegion(OPosition coord, ODimension size, int clear = True);
  
protected:
  virtual bool ItemLayout();
  virtual void CalcMaxItemSize();

  void CalcColumnMaxWidths();
  
  void ShowHeaderBar();
  void HideHeaderBar();
  
  void _KeyLeft();
  void _KeyRight();
  void _KeyUp();
  void _KeyDown();
  void _KeyHome();
  void _KeyEnd();
  void _KeyPageUp();
  void _KeyPageDown();

  OXGC *_normGC;
  OXGC *_selGC;
  OXGC *_hilightGC;

  int _viewMode, _fullRowSelect, _gridLines;
  std::vector<SColumnData *> _columnData;
  int _columnMargin, _sortColumn, _autoSort, _rows, _cols;

  OXFont *_headerFont, *_itemFont;
  OXFrame *_header;
};


#endif  // __OXLISTVIEW_H
