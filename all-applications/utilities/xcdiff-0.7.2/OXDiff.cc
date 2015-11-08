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

#include <stdio.h>
#include <string.h>

#include "OXDiff.h"
#include "OXDiffView.h"


//----------------------------------------------------------------------

ODifference::ODifference(char *line) {

  leftStart = leftEnd = rightStart = rightEnd = 0;
  next = NULL;
  strncpy(str, line, 20); str[20] = '\0';

  if (sscanf(line, "%d,%dc%d,%d\n",
             &leftStart, &leftEnd, &rightStart, &rightEnd) == 4) {
    type = DIFF_LRC;

  } else
  // 2c2,3  right change 2 and adds 3 
  if (sscanf(line, "%dc%d,%d\n",
             &leftStart, &rightStart, &rightEnd) == 3) {
    type = DIFF_RCA;
    leftEnd = leftStart;

  } else
  // 2,3c2 left change 2 and adds 3
  if (sscanf(line, "%d,%dc%d\n",
             &leftStart, &leftEnd, &rightStart) == 3) {
    type = DIFF_LCA;
    rightEnd = rightStart;

  } else
  // 3c3 single line changed
  if (sscanf(line, "%dc%d\n", &leftStart, &rightStart) == 2) {
    type = DIFF_LC;
    leftEnd = leftStart;
    rightEnd = rightStart;

  } else
  // 2a3,4  right adds 3 and 4
  if (sscanf(line, "%da%d,%d\n",
             &leftStart, &rightStart, &rightEnd) == 3) {
    leftEnd = leftStart;
    type = DIFF_RA;

  } else
  // 3a4 right adds 4
  if (sscanf(line, "%da%d\n", &leftStart, &rightStart) == 2) {
    leftEnd = leftStart;
    rightEnd = rightStart;
    type = DIFF_RA;

  } else
  // 3,4d2 left adds 3 and 4
  if (sscanf(line, "%d,%dd%d\n", &leftStart, &leftEnd, &rightStart) == 3) {
    rightEnd = rightStart;
    type = DIFF_LA;

  } else
  // 5d4  left adds 5
  if (sscanf(line, "%dd%d\n", &leftStart, &rightStart) == 2) {
    leftEnd = leftStart;
    rightEnd = rightStart;
    type = DIFF_LA;

  } else
    type = DIFF_NOTREL;

  if (str[strlen(str)-1] == '\n') str[strlen(str)-1] = '\0';
}

//----------------------------------------------------------------------

OXDiff::OXDiff(const OXWindow *p, int w, int h, unsigned int options) :
  OXCompositeFrame(p, w, h, options),
  /*firstDiff(NULL),*/
  leftFile(NULL),
  rightFile(NULL),
  numDiffs(0) {

  firstDiff = NULL;

  SetLayoutManager(new OHorizontalLayout(this));
  OLayoutHints *_layout1 = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y);
  OLayoutHints *_layout2 = new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X,
                                            0, 0, 0, 2);

  _vframe1 = new OXVerticalFrame(this, 0, 0, 0);
  _leftLabel = new OXLabel(_vframe1, new OString("File: <noname>"));
  _leftLabel->ChangeOptions(_leftLabel->GetOptions() | SUNKEN_FRAME);
  _leftLabel->SetTextAlignment(TEXT_LEFT | TEXT_TOP);
  _leftLabel->SetMargins(3, 0, 1, 1);
  _bodyLeft = new OXDiffView(_vframe1, 300, 400, SUNKEN_FRAME | DOUBLE_BORDER);
  _vframe1->AddFrame(_leftLabel, _layout2);
  _vframe1->AddFrame(_bodyLeft, _layout1);
  
  _vframe2 = new OXVerticalFrame(this, 0, 0, 0);
  _rightLabel = new OXLabel(_vframe2, new OString("File: <noname>"));
  _rightLabel->ChangeOptions(_rightLabel->GetOptions() | SUNKEN_FRAME);
  _rightLabel->SetTextAlignment(TEXT_LEFT | TEXT_TOP);
  _rightLabel->SetMargins(3, 0, 1, 1);
  _bodyRight = new OXDiffView(_vframe2, 300, 400, SUNKEN_FRAME | DOUBLE_BORDER);
  _vframe2->AddFrame(_rightLabel, _layout2);
  _vframe2->AddFrame(_bodyRight, _layout1);
  
  AddFrame(_vframe1, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y,
                                      0, 1, 0, 0));
  AddFrame(_vframe2, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y,
                                      1, 0, 0, 0));
}

OXDiff::~OXDiff() {
  ODifference *curr, *ptr = firstDiff;

  while (ptr) {
    curr = ptr->next;
    delete ptr;
    ptr = curr;
  }

  if (leftFile) delete[] leftFile;
  if (rightFile) delete[] rightFile;
}

void OXDiff::SetLeftFile(char *left) {
  char buf[256], *p;

  if (leftFile) delete[] leftFile;
	
  if (left) {
    leftFile = new char[strlen(left) + 1];
    strcpy(leftFile, left);
    p = strrchr(leftFile, '/');
    if (p) p++; else p = leftFile;
    sprintf(buf, "File: %s", p);
    _leftLabel->SetText(new OString(buf));
  } else {
    _leftLabel->SetText(new OString(" File: <noname>"));
  }
  //_leftLabel->Resize(_leftLabel->GetDefaultSize());

  Layout();
  UnDoDiff();
}

void OXDiff::SetRightFile(char *right) {
  char buf[256], *p;

  if (rightFile) delete[] rightFile;
	
  if (right) {
    rightFile = new char[strlen(right) + 1];
    strcpy(rightFile, right);
    p = strrchr(rightFile, '/');
    if (p) p++; else p = rightFile;
    sprintf(buf, "File: %s", p);
    _rightLabel->SetText(new OString(buf));
  } else {
    _rightLabel->SetText(new OString(" File: <noname>"));
  }
  //_rightLabel->Resize(_rightLabel->GetDefaultSize());

  Layout();
  UnDoDiff();
}

int OXDiff::DoDiff() {
  char cmd[256];
  FILE *pipe;

  if (!leftFile || !rightFile) return 0;
	
  ODifference *curr, *ptr = firstDiff;
  while (ptr) {
    curr = ptr->next;
    delete ptr;
    ptr = curr;
  }

  sprintf(cmd, "diff -w \"%s\" \"%s\"", leftFile, rightFile);
  if ((pipe = popen(cmd, "r")) == NULL) {
    return 0;
  }
	
  firstDiff = NULL;
  numDiffs = 0;

  while (fgets(cmd, sizeof(cmd), pipe) != NULL) {
    if (cmd[0] != '<' && cmd[0] != '>' && cmd[0] != '-') {
      ptr = new ODifference(cmd);
      numDiffs++;

      if (!firstDiff)
        firstDiff = ptr;
      else
        curr->next = ptr;

      curr = ptr;
    }
  }

  pclose(pipe);
  _bodyLeft->LoadFile(leftFile);
  _bodyRight->LoadFile(rightFile);
  ParseOutput();
  _bodyLeft->Layout();
  _bodyRight->Layout();

  return 1;
}

void OXDiff::UnDoDiff() {
  if (leftFile)
    _bodyLeft->LoadFile(leftFile);
  else
    _bodyLeft->Clear();

  if (rightFile)
    _bodyRight->LoadFile(rightFile);
  else
    _bodyRight->Clear();
}

void OXDiff::ShowDiff(int index) {
  ODifference *curr = GetDifference(index);
  int rstart, rend;
  int ldiff, rdiff;
  OXDiffFrame *leftFrame = _bodyLeft->GetTextFrame();
  OXDiffFrame *rightFrame = _bodyRight->GetTextFrame();

  if (curr) {
    switch (curr->type) {
      case DIFF_LRC:
      case DIFF_LC:
        ldiff = curr->leftEnd - curr->leftStart;
        rdiff = curr->rightEnd - curr->rightStart;
        if (ldiff > rdiff) {
          leftFrame->CalcMarkRegion(curr->leftStart - 1,
                                    curr->leftEnd - 1,
                                    rstart, rend);
          rightFrame->MarkRegion(rstart, rend);
          //_bodyRight->AdjustScrollBar();
        } else {
          rightFrame->CalcMarkRegion(curr->rightStart - 1,
                                     curr->rightEnd - 1,
                                     rstart, rend);
          leftFrame->MarkRegion(rstart, rend);
          //_bodyLeft->AdjustScrollBar();
        }
        break;

      case DIFF_RA:  // 2a3,4  right adds 3 and 4 --> left deleted 3,4
      case DIFF_RCA: // 2c2,3  right change 2 and adds 3 
        rightFrame->CalcMarkRegion(curr->rightStart - 1,
                                   curr->rightEnd - 1,
                                   rstart, rend);
        leftFrame->MarkRegion(rstart, rend);
        //_bodyLeft->AdjustScrollBar();
        break;

      case DIFF_LA:  // 3,4d2 left adds 3 and 4   OR // 3d2  left adds 3
      case DIFF_LCA: // 2,3c2 left change 2 and adds 3
        leftFrame->CalcMarkRegion(curr->leftStart - 1,
                                  curr->leftEnd - 1,
                                  rstart, rend);
        rightFrame->MarkRegion(rstart, rend);
        //_bodyRight->AdjustScrollBar();
        break;
    }	

    int top = min(leftFrame->GetTopLine(), rightFrame->GetTopLine());
    if (top < 0) top = 0;
    leftFrame->SetTopLine(top);
    rightFrame->SetTopLine(top);

    _bodyRight->AdjustScrollBar();
    _bodyLeft->AdjustScrollBar();

  }
}

void OXDiff::CenterDiff() {
  OXDiffFrame *leftFrame = _bodyLeft->GetTextFrame();
  OXDiffFrame *rightFrame = _bodyRight->GetTextFrame();

  leftFrame->CenterDiff();
  rightFrame->CenterDiff();

  int top = min(leftFrame->GetTopLine(), rightFrame->GetTopLine());
  if (top < 0) top = 0;
  leftFrame->SetTopLine(top);
  rightFrame->SetTopLine(top);

  _bodyRight->AdjustScrollBar();
  _bodyLeft->AdjustScrollBar();
}

void OXDiff::SetLineNumOn(int on) {
  _bodyLeft->SetLineNumOn(on);
  _bodyRight->SetLineNumOn(on);
}

void OXDiff::ParseOutput() {
  ODifference *curr = firstDiff;
  OXDiffFrame *leftFrame = _bodyLeft->GetTextFrame();
  OXDiffFrame *rightFrame = _bodyRight->GetTextFrame();
  int i, ldiff, rdiff;
	
  while (curr) {
    switch (curr->type) {
      case DIFF_LRC: // 6,9c5,7
      case DIFF_LC:  // 5c7
        ldiff = curr->leftEnd - curr->leftStart;
        rdiff = curr->rightEnd - curr->rightStart;
        for (i = curr->leftStart; i <= curr->leftEnd; i++)
          leftFrame->SetLineType(i, DISPLAY_CHANGE);
        for (i = curr->rightStart; i <= curr->rightEnd; i++)
          rightFrame->SetLineType(i, DISPLAY_CHANGE);
        if (ldiff > rdiff) {
          for (i = rdiff; i < ldiff; i++)
            rightFrame->InsLine(curr->rightEnd, "",
                                DISPLAY_CHANGE | DISPLAY_DELETE);
 	} else if (ldiff < rdiff) {
          for (i = ldiff; i < rdiff; i++)
            leftFrame->InsLine(curr->leftEnd, "",
                               DISPLAY_CHANGE | DISPLAY_DELETE);
        }
        break;

      case DIFF_RCA: // 2c2,3  right change 2 and adds 3 
        leftFrame->SetLineType(curr->leftStart, DISPLAY_CHANGE);
        for (i = curr->rightStart; i <= curr->rightEnd; i++) {
 	  rightFrame->SetLineType(i, DISPLAY_CHANGE);
 	  if (i != curr->rightStart)
 	    leftFrame->InsLine(curr->leftEnd, "",
                               DISPLAY_CHANGE | DISPLAY_DELETE);
 	}
        break;

      case DIFF_LCA: // 2,3c2 left change 2 and adds 3
        rightFrame->SetLineType(curr->rightStart, DISPLAY_CHANGE);
        for (i = curr->leftStart; i <= curr->leftEnd; i++) {
          leftFrame->SetLineType(i, DISPLAY_CHANGE);
          if (i != curr->leftStart)
          rightFrame->InsLine(curr->rightEnd, "",
                              DISPLAY_CHANGE | DISPLAY_DELETE);
 	}
        break;

      case DIFF_LA: // 3,4d2 left adds 3 and 4   OR // 3d2  left adds 3
        //rightFrame->SetLineType(curr->rightStart, DISPLAY_INSERTL);
        for (i = curr->leftStart; i <= curr->leftEnd; i++) {
          leftFrame->SetLineType(i, DISPLAY_INSERTL);
          rightFrame->InsLine(curr->rightEnd, "",
                              DISPLAY_INSERTL | DISPLAY_DELETE);
        }
        break;

      case DIFF_RA: // 2a3,4  right adds 3 and 4 --> left deleted 3,4
        for (i = curr->rightStart; i <= curr->rightEnd; i++) {
          leftFrame->InsLine(curr->leftEnd, "",
                             DISPLAY_INSERTR | DISPLAY_DELETE);
          rightFrame->SetLineType(i, DISPLAY_INSERTR);
        }
        break;	

    }
	
    curr = curr->next;
  }
}

void OXDiff::SetFont(OXFont *f) {
  OXDiffFrame *leftFrame = _bodyLeft->GetTextFrame();
  OXDiffFrame *rightFrame = _bodyRight->GetTextFrame();

  leftFrame->SetFont(_client->GetFontPool()->GetFont(f));
  rightFrame->SetFont(f);

  Layout();
}

void OXDiff::SetColors(ODiffColors *colors) {
  OXDiffFrame *leftFrame = _bodyLeft->GetTextFrame();
  OXDiffFrame *rightFrame = _bodyRight->GetTextFrame();

  leftFrame->SetColors(colors);
  rightFrame->SetColors(colors);

  Layout();
}

char *OXDiff::GetDiffStr(int index) {
  ODifference *curr = GetDifference(index);
  if (curr) return curr->str;
  return NULL;
}

ODifference *OXDiff::GetDifference(int index) {
  ODifference *curr = firstDiff;

  for (int i = 1; curr && i != index; ++i) curr = curr->next;
  return curr;
}
