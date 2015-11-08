/**************************************************************************

    This file is part of rx320, a control program for the Ten-Tec RX320
    receiver. Copyright (C) 2000-2004, Hector Peraza.

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

#ifndef __OXFREQDB_H
#define __OXFREQDB_H

#include <xclass/utils.h>
#include <xclass/OXMainFrame.h>
#include <xclass/OXToolBar.h>
#include <xclass/OXStatusBar.h>
#include <xclass/OX3dLines.h>
#include <xclass/OXMenu.h>
#include <xclass/OXListView.h>
#include <xclass/OMessage.h>

#include "ORX320.h"
#include "OFreqRecord.h"
#include "main.h"


//----------------------------------------------------------------------

class OFDBitem : public OListViewItem {
public:
  OFDBitem(const OXListView *p, int id, OFreqRecord *fr,
           std::vector<OString *> names);
  virtual ~OFDBitem();
  
  OFreqRecord *GetFreqRecord() const { return _freqRec; }
  void ChangeFreqRecord(OFreqRecord *fr);

protected:
  OFreqRecord *_freqRec;
};

class OXFreqDB : public OXMainFrame {
public:
  OXFreqDB(const OXWindow *p, OXMain *m, int w, int h);
  virtual ~OXFreqDB();

  virtual int ProcessMessage(OMessage *msg);
  virtual int CloseWindow();

  void SetWindowTitle(char *title);
  void UpdateStatus();

  void ReadFile(char *fname);
  void WriteFile(char *fname);

  void DoOpen();
  void DoSave(char *fname);
  void DoPrint();
  void DoAbout();
  void DoToggleToolBar();
  void DoToggleStatusBar();

  void ClearFreqList();
  
  void AddStation(OFreqRecord *frec, int recno = -1);
  
  int  IsSaved();
  void SetChanged(int onoff);

public:
  OXFreqDB *prev, *next;

protected:
  OXMain *_rxmain;

  OXPopupMenu *_MakePopup(struct _popup *);

  OXToolBar *_toolBar;
  OXStatusBar *_statusBar;
  OXHorizontal3dLine *_toolBarSep;
  OXListView *_listView;

  OLayoutHints *_menuBarLayout, *_menuBarItemLayout;

  OXMenuBar *_menuBar;
  OXPopupMenu *_menuFile, *_menuEdit, *_menuView, *_menuHelp;

  char *_filename, *_path;
  int _changed, _exiting;
};


#endif  // __OXFREQDB_H
