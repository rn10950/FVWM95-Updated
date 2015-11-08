/**************************************************************************

    This file is part of xclass.
    Copyright (C) 2004, Hector Peraza.

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

#ifndef __OXHELPDOC_H
#define __OXHELPDOC_H

#include <vector>

#include <xclass/utils.h>
#include <xclass/OHtmlUri.h>
#include <xclass/OXHtml.h>


//----------------------------------------------------------------------

class OXHelpDoc : public OXHtml {
public:
  OXHelpDoc(const OXWindow *p, int w, int h, int id = -1);
  virtual ~OXHelpDoc();

  virtual void Clear();
  virtual int  IsVisited(const char *url);
  virtual int  ProcessToken(OHtmlElement *pElem, const char *name, int type);
  virtual int  FormAction(OHtmlForm *form, int id);

  void AddToVisited(const char *url);

  char *GetDocName() const { return docName; }

protected:
  char *docName;
  int docNameStart;
  
  std::vector<char *> visitedUrl;
};


#endif  // __OXHELPDOC_H
