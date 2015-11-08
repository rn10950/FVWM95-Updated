/**************************************************************************

    This file is part of xclass.
    Copyright (C) 1996-2004 David Barth, Hector Peraza.

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

#ifndef __OXTOOLBAR_H
#define __OXTOOLBAR_H


#include <xclass/utils.h>
#include <xclass/OXClient.h>
#include <xclass/OXCompositeFrame.h>
#include <xclass/OXWidget.h>
#include <xclass/OString.h>

class OXButton;
class OXToolBarButton;


// Use this structure to create a toolbar with old-style Win95 buttons

struct SToolBarData {
  char *pixmap_name;
  char **pixmap_data;
  char *tip_text;
  int  type;
  int  id;
  OXButton *button;
};

// Use this structure to create a toolbar with Win98/Netscape-style buttons

struct SToolBarDataEx {
  char *pixmap_name;            // normal picture
  char **pixmap_data;
  char *pixmap_on_name;         // mouse-on picture
  char **pixmap_on_data;
  char *pixmap_disabled_name;   // disabled picture
  char **pixmap_disabled_data;
  char *label;
  char *tip_text;
  int  type;
  int  id;
  OXToolBarButton *button;
};
                      

//----------------------------------------------------------------------

class OXToolBar : public OXHorizontalFrame {
public:
  OXToolBar(const OXWindow *p, int w = 1, int h = 1,
            unsigned int options = CHILD_FRAME);
  virtual ~OXToolBar();

  void AddButton(OXButton *b, char *tip_text = NULL);
  OXButton *AddButton(const OPicture *pic, char *tip_text, int type, int id);
  void AddButtons(SToolBarData *data);
  void AddButtons(SToolBarDataEx *data,
                  int show_labels = False, int equal_widths = False);
  void AddSeparator();

  OXButton *GetButtonById(int id);

protected:
  int _spacing;
};


#endif   // __OXTOOLBAR_H
