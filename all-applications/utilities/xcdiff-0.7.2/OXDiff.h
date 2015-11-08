/**************************************************************************
 
    This file is part of xcdiff, a front-end to the diff command.              
    Copyright (C) 1998-2002 Matzka Gerald, Hector Peraza.            

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

#ifndef __OXDIFF_H
#define __OXDIFF_H

#include "OXDiffView.h"
#include "OXDialogs.h"


#define DIFF_LRC      1  // Left Right Change
#define DIFF_RCA      2  // Right Change Add
#define DIFF_LCA      3	 // Left Change Add
#define DIFF_LA       4  // Left Add
#define DIFF_RA       5  // Right Add
#define DIFF_LC       6  // Left Change
#define DIFF_NOTREL   7	 // Undefined


//----------------------------------------------------------------------

struct ODifference {
  ODifference(char *line);

  int  type, leftStart, leftEnd, rightStart, rightEnd;
  char str[21];
  ODifference *next;
};

class OXFont;

class OXDiff : public OXCompositeFrame {
public:
  OXDiff(const OXWindow *p, int w, int h,
         unsigned int options = HORIZONTAL_FRAME);
  virtual ~OXDiff();
    
  void SetLineNumOn(int);
  void SetLeftFile(char * = NULL);
  void SetRightFile(char * = NULL);

  int  GetNumDiffs() { return numDiffs; }
  int  DoDiff();
  void UnDoDiff();
  void ParseOutput();
  void ShowDiff(int);
  void CenterDiff();
  int  CanDoDiff() { return (leftFile && rightFile); }
	
  char *GetDiffStr(int index);
  
  OXFont *GetFont() const
     { return _bodyLeft->GetTextFrame()->GetFont(); }
  void SetFont(OXFont *f);
  
  ODiffColors *GetColors() const
     { return _bodyLeft->GetTextFrame()->GetColors(); }
  void SetColors(ODiffColors *colors);

private:
  OXDiffView *_bodyLeft, *_bodyRight;
  OXLabel *_leftLabel, *_rightLabel;
  OXVerticalFrame *_vframe1, *_vframe2;
  ODifference *GetDifference(int);

  char *leftFile;
  char *rightFile;
  int numDiffs;
  ODifference *firstDiff;
};


#endif  // __OXDIFF_H
