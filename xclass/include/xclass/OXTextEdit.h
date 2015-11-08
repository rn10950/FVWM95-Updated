/**************************************************************************

    This is file is part of xclass.
    Copyright (C) 1998-2001, Harald Radke, Hector Peraza.

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

#ifndef __OXTEXTEDIT_H
#define __OXTEXTEDIT_H

#include <xclass/OResourcePool.h>
#include <xclass/OXFont.h>
#include <xclass/OXItemView.h>
#include <xclass/OTextBuffer.h>
#include <xclass/OMessage.h>


class OTextLine;


//----------------------------------------------------------------------

class OTextEditMessage : public OWidgetMessage {
public:
  OTextEditMessage(int typ, int act = MSG_NULL, int wid = -1,
                   OPosition cpos = OPosition(0, 0)) : 
    OWidgetMessage(typ, act, wid) { cursor_pos = cpos; }
      
  int id;
  OPosition cursor_pos;
};
        

//----------------------------------------------------------------------

class OXTextEdit : public OXItemView {
protected:
  static const OXFont *_defaultFont;
  static Atom  _XC_SELECTION_DATA;

public:
  OXTextEdit(const OXWindow *p, int w, int h, int id,
             unsigned int options = SUNKEN_FRAME | DOUBLE_BORDER | OWN_BKGND,
             unsigned int sboptions = CANVAS_SCROLL_BOTH);
  virtual ~OXTextEdit();

  virtual int HandleTimer(OTimer *t);
  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleDoubleClick(XButtonEvent *event);
  virtual int HandleTripleClick(XButtonEvent *event);
  virtual int HandleMotion(XMotionEvent *event);
  virtual int HandleKey(XKeyEvent *event);
  virtual int HandleSelection(XSelectionEvent *event);
  virtual int HandleSelectionRequest(XSelectionRequestEvent *event);

  //---- text operations

  virtual int LoadFromFile(char *filename);
  virtual int SaveToFile(char *filename);

  virtual void Clear(bool null_line = True);
  virtual void AddLine(const char *string);
  virtual void AddLine(const OString *string);
  virtual void InsertText(const char *string, int len = -1);
  virtual void InsertText(const OString *string);
  virtual OString *GetText();
  virtual OString *GetSelectedText();
  
  virtual int Search(const char *pattern, int direction, bool match_case);

  //---- selection operations

  virtual void SelectAll();
  virtual void UnSelectAll();
  virtual void SelectFrom(OPosition tindex);
  virtual void SelectTo(OPosition tindex);
  virtual void SelectToPos(OPosition pos);
  virtual void DeleteSelection();

  //---- cursor / insertion point operations

  OPosition GetCursorPosition() const { return _cursorPos; }
  OPosition GetInsertionPoint() const { return _textIndex; }
  virtual void SetCursorPosition(OPosition newpos, int ensure_visible = False);
  
  virtual void CursorLeft();
  virtual void CursorRight();
  virtual void CursorUp();
  virtual void CursorDown();
  
  virtual void CursorPageUp();
  virtual void CursorPageDown();
  
  virtual void CursorHome();
  virtual void CursorEnd();
  virtual void CursorTop();
  virtual void CursorBottom();

  //---- clipboard operations
  
  virtual int  Cut();
  virtual int  Copy();
  virtual void Paste();

  //---- setup font, color, tab width...

  void SetTextColor(unsigned int color);

  void SetFont(const OXFont *f);
  const OXFont *GetFont() const { return _font; }

  unsigned int GetTabWidth() const { return _tabWidth; }
  virtual void SetTabWidth(unsigned int tabWidth);

  //---- text buffer states

  bool TextChanged() const { return _changed; }
  bool IsReadOnly() const { return _readOnly; }
  virtual void SetReadOnly(bool ro);
  
  void AutoUpdate(int onoff = True);

protected:
  virtual void _GotFocus();
  virtual void _LostFocus();

  virtual bool ItemLayout();

  OPosition AdjustCoord(OPosition pos);

  virtual void DrawItem(OItem *item, OPosition coord, ODimension size);
  virtual void DrawDragOutline() {}

  virtual void DrawCursor(int state);
  void EnsureVisible(OPosition cpos);

  void JoinLine(int lnum);
  void SplitLine(int lnum, int charpos);
  void DeleteChar(int lnum, int charpos);
  
  void _UpdateCursor(OPosition newPos);
  void _CheckLayout(OTextLine *line);

  void _PastePrimary(Window wid, Atom property, int Delete);

  const OXFont *_font;
  OXGC *_normGC;
  OXGC *_selGC;
  OXGC *_hilightGC;
  OTextLine *_dragStartLine;

  bool _changed, _readOnly;
  unsigned int _tabWidth;
  ODimension _margin;
  int _cursorState, _blinkTime, _cursorLastX;
  int _autoUpdate;
  OTimer *_cursorBlink;
  OPosition _cursorPos;  // the cursor position is in screen text units
                         // (line;char)
  OPosition _textIndex;  // same as cursor position, but directly indexes
                         // into the text (tabs count as a single char)
  OString *_clipboardText;   // OString to save the text after a cut or
                             // copy operation
};


//----------------------------------------------------------------------

class OTextLine : public OItem, public OTextBuffer {
public:
  OTextLine(OXTextEdit *p, const char *string = "");
  OTextLine(OXTextEdit *p, OTextLine *line);
  virtual ~OTextLine() {}

  virtual ODimension GetDefaultSize() const;

  void SetFont(const OXFont *font);

  // standard selection, whole line
  virtual void Select(bool state);

  // 'from' and 'to' are character indexes
  virtual void SelectChars(int from, int to);

  int GetSelectionStart() const { return _selStart; }
  int GetSelectionEnd() const { return _selEnd; }
  int GetSelectionLength() const { return _selEnd - _selStart; }

  OString *GetSelectedText();
  friend class OXTextEdit;

  int GetTextWidth(int from, int to) const;
  int GetTextHeight() const { return ((OXTextEdit *)_parent)->GetFont()->TextHeight(); }
  int Column2Index(int col) const;
  int Index2Column(int index) const;

protected:
  int GetCharIndexByCoord(int x0, int xCoord) const;
  char *GetUntabbedText(int from, int to) const;

  int _selStart, _selEnd;
};


#endif  // __OXTEXTEDIT_H
