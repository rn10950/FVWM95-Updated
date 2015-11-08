/**************************************************************************

    HTML widget for xclass. Based on tkhtml 1.28
    Copyright (C) 1997-2000 D. Richard Hipp <drh@acm.org>
    Copyright (C) 2002-2003 Hector Peraza.

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

// Routines used for processing HTML makeup for forms.

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include <xclass/OXHtml.h>
#include <xclass/OXTextButton.h>
#include <xclass/OXCheckButton.h>
#include <xclass/OXRadioButton.h>
#include <xclass/OXTextEntry.h>
#include <xclass/OXSecretTextEntry.h>
#include <xclass/OXDDListBox.h>
#include <xclass/OXTextEdit.h>


//----------------------------------------------------------------------

// Unmap any input control that is currently mapped.

void OXHtml::UnmapControls() {
  OHtmlInput *p;

  for (p = firstInput; p; p = p->iNext) {
    if (p->frame != 0 /*&& p->frame->IsMapped()*/)
      p->frame->UnmapWindow();
  }
}


// Map any control that should be visible according to the
// current scroll position. At the same time, if any controls that
// should not be visible are mapped, unmap them. After this routine
// finishes, all <INPUT> controls should be in their proper places
// regardless of where they might have been before.
//
// Return the number of controls that are currently visible.

int OXHtml::MapControls() {
  OHtmlInput *p;     // For looping over all controls
  int x, y, w, h;    // Part of the virtual canvas that is visible
  int cnt = 0;       // Number of visible controls

  x = _visibleStart.x;
  y = _visibleStart.y;
  w = _canvas->GetWidth();
  h = _canvas->GetHeight();
  for (p = firstInput; p; p = p->iNext) {
    if (p->frame == 0) continue;
    if (p->y < y + h && p->y + p->h > y &&
        p->x < x + w && p->x + p->w > x) {
      // The control should be visible. Make is so if it isn't already
      p->frame->MoveResize(p->x - x, p->y + formPadding/2 - y,
                           p->w, p->h - formPadding);
      /*if (!p->frame->IsMapped())*/ p->frame->MapWindow();
      ++cnt;
    } else {
      // This control should not be visible. Unmap it.
      /*if (p->frame->IsMapped())*/ p->frame->UnmapWindow();
    }
  }

  return cnt;
}


// Delete all input controls. This happens when the OXHtml widget
// is cleared.

void OXHtml::DeleteControls() {
  OHtmlInput *p;        // For looping over all controls

  p = firstInput;
  firstInput = 0;
  lastInput = 0;
  nInput = 0;

  if (p == 0) return;

  for (; p; p = p->iNext) {
    if (p->pForm && ((OHtmlForm *)p->pForm)->hasctl) {
      ((OHtmlForm *)p->pForm)->hasctl = 0;
    }
    if (p->frame) {
      if (!_exiting) p->frame->DestroyWindow();
      delete p->frame;
      p->frame = 0;
    }
    p->sized = 0;
  }
}


// Return an appropriate type value for the given <INPUT> markup.

static int InputType(OHtmlElement *p) {
  int type = INPUT_TYPE_Unknown;
  char *z;
  int i;
  static struct {
    char *zName;
    int type;
  } types[] = {
    { "checkbox",  INPUT_TYPE_Checkbox },
    { "file",      INPUT_TYPE_File     },
    { "hidden",    INPUT_TYPE_Hidden   },
    { "image",     INPUT_TYPE_Image    },
    { "password",  INPUT_TYPE_Password },
    { "radio",     INPUT_TYPE_Radio    },
    { "reset",     INPUT_TYPE_Reset    },
    { "submit",    INPUT_TYPE_Submit   },
    { "text",      INPUT_TYPE_Text     },
    { "name",      INPUT_TYPE_Text     },
    { "textfield", INPUT_TYPE_Text     },
    { "button",    INPUT_TYPE_Button   },
    { "name",      INPUT_TYPE_Text     },
  };

  switch (p->type) {
    case Html_INPUT:
      z = p->MarkupArg("type", "text");
      if (z == 0) break;
      for (i = 0; i < sizeof(types) / sizeof(types[0]); i++) {
        if (strcasecmp(types[i].zName, z) == 0) {
          type = types[i].type;
          break;
        }
      }
      break;

    case Html_SELECT:
      type = INPUT_TYPE_Select;
      break;

    case Html_TEXTAREA:
      type = INPUT_TYPE_TextArea;
      break;

    case Html_APPLET:
    case Html_IFRAME:
    case Html_EMBED:
      type = INPUT_TYPE_Applet;
      break;

    default:
      CANT_HAPPEN;
      break;
  }
  return type;
}


// 'frame' is the xclass child widget that is used to implement an input
// element. Query the widget for its size and put that information in the
// pElem structure that represents the input.

void OXHtml::SizeAndLink(OXFrame *frame, OHtmlInput *pElem) {

  pElem->frame = frame;
  if (pElem->frame == 0) {
    pElem->Empty();
  } else if (pElem->itype == INPUT_TYPE_Hidden) {
    pElem->w = 0;
    pElem->h = 0;
    pElem->flags &= ~HTML_Visible;
    pElem->style.flags |= STY_Invisible;
  } else {
    pElem->w = frame->GetDefaultWidth();
    pElem->h = frame->GetDefaultHeight() + formPadding;
    pElem->flags |= HTML_Visible;
    pElem->html = this;
  }
  pElem->iNext = 0;
  if (firstInput == 0) {
    firstInput = pElem;
  } else {
    lastInput->iNext = pElem;
  }
  lastInput = pElem;
  pElem->sized = 1;

#if 0
  if (pElem->frame) {
    pElem->frame->ChangeOptions(pElem->frame->GetOptions() | OWN_BKGND);
    pElem->frame->SetBackgroundColor(_defaultFrameBackground);
  }
#else
  if (pElem->frame) {
    int bg = pElem->style.bgcolor;
    int fg = pElem->style.color;
    XColor *cbg = apColor[bg];
    XColor *cfg = apColor[fg];
    pElem->frame->ChangeOptions(pElem->frame->GetOptions() | OWN_BKGND);
    pElem->frame->SetBackgroundColor(cbg->pixel);
  }
#endif

  if (pElem->frame) {
    // the following is needed by some embedded widgets like
    // OXListBox and OXTextEdit
    pElem->frame->MapSubwindows();
    pElem->frame->Layout();
  }
}


// Append all text and space tokens between pStart and pEnd to
// the given OString.  [ OXTextEdit ]

void OXHtml::AppendText(OString *str,
                        OHtmlElement *pFirst, OHtmlElement *pEnd) {

  while (pFirst && pFirst != pEnd) {
    switch (pFirst->type) {
      case Html_Text:
        str->Append(((OHtmlTextElement *)pFirst)->zText);
        break;

      case Html_Space:
        if (pFirst->flags & HTML_NewLine) {
          str->Append("\n");
        } else {
          int cnt;
          static char zSpaces[] = "                             ";
          cnt = pFirst->count;
          while (cnt > sizeof(zSpaces) - 1) {
            str->Append(zSpaces, sizeof(zSpaces) - 1);
            cnt -= sizeof(zSpaces) - 1;
          }
          if (cnt > 0) {
            str->Append(zSpaces, cnt);
          }
        }
        break;

      default:
        break;
    }

    pFirst = pFirst->pNext;
  }
}


class OXHtmlLBEntry : public OXTextLBEntry {
public:
  OXHtmlLBEntry(const OXWindow *p, OString *s, OString *val, int ID) :
    OXTextLBEntry(p, s, ID) { _val = val; }
  virtual ~OXHtmlLBEntry() { if (_val) delete _val; }

  const char *GetValue() const { return _val ? _val->GetString() : 0; }

protected:
  OString *_val;
};


// The "p" argument points to a <select>.  This routine scans all
// subsequent elements (up to the next </select>) looking for
// <option> tags.  For each option tag, it appends the corresponding
// entry to the "lb" listbox element.
//
// lb   -- An OXListBox object
// p    -- The <SELECT> markup
// pEnd -- The </SELECT> markup

void OXHtml::AddSelectOptions(OXListBox *lb,
                              OHtmlElement *p, OHtmlElement *pEnd) {
  int id = 0;

  while (p && p != pEnd && p->type != Html_EndSELECT) {
    if (p->type == Html_OPTION) {
      OString *str;

      char *zValue = p->MarkupArg("value", "");
      int selected = (p->MarkupArg("selected", 0) != 0);

      p = p->pNext;

      str = new OString("");
      while (p && p != pEnd &&
             p->type != Html_EndOPTION &&
             p->type != Html_OPTION &&
             p->type != Html_EndSELECT) {
        if (p->type == Html_Text) {
          str->Append(((OHtmlTextElement *)p)->zText);
        } else if (p->type == Html_Space) {
          str->Append(" ");
        }
        p = p->pNext;
      }
      lb->AddEntry(new OXHtmlLBEntry(lb, str, new OString(zValue), id),
                   new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X));
      if (p->MarkupArg("selected", 0) != 0) lb->Select(id);
      ++id;
    } else {
      p = p->pNext;
    }
  }
}


// This routine implements the Sizer() function for <INPUT>,
// <SELECT> and <TEXTAREA> markup.
//
// A side effect of sizing these markups is that widgets are
// created to represent the corresponding input controls.
//
// The function normally returns 0.  But if it is dealing with
// a <SELECT> or <TEXTAREA> that is incomplete, 1 is returned.
// In that case, the sizer will be called again at some point in
// the future when more information is available.

int OXHtml::ControlSize(OHtmlInput *pElem) {
  int incomplete = 0;    // True if data is incomplete
 
  if (pElem->sized) return 0;

  pElem->itype = InputType(pElem);   //// pElem->InputType();
                                     //// or done in the constructor!

//  if (pElem->pForm == 0) {
//    pElem->Empty();
//    return incomplete;
//  }

  switch (pElem->itype) {
    case INPUT_TYPE_File:
    case INPUT_TYPE_Hidden:
    case INPUT_TYPE_Image:
      pElem->Empty();
      SizeAndLink(0, pElem);
      break;

    case INPUT_TYPE_Checkbox: {
      pElem->cnt = ++nInput;
      OXFrame *f = new OXCheckButton(_canvas, NULL, pElem->cnt);
      if (pElem->MarkupArg("checked", 0))
        ((OXCheckButton *)f)->SetState(BUTTON_DOWN);
      //f->Associate(this);
      f->Resize(f->GetDefaultSize());
      SizeAndLink(f, pElem);
      break;
    }

    case INPUT_TYPE_Radio: {
      pElem->cnt = ++nInput;
      OXFrame *f = new OXRadioButton(_canvas, NULL, pElem->cnt);
      if (pElem->MarkupArg("checked", 0))
        ((OXRadioButton *)f)->SetState(BUTTON_DOWN);
      f->Associate(this);
      f->Resize(f->GetDefaultSize());
      SizeAndLink(f, pElem);
      break;
    }

    case INPUT_TYPE_Reset: {
      pElem->cnt = ++nInput;
      char *z = pElem->MarkupArg("value", 0);
      if (!z) z = "Reset";
      OXFrame *f = new OXTextButton(_canvas, new OHotString(z), pElem->cnt);
      f->TakeFocus(False);
      f->Associate(this);
      f->Resize(f->GetDefaultSize());
      SizeAndLink(f, pElem);
      break;
    }

    case INPUT_TYPE_Button:
    case INPUT_TYPE_Submit: {
      pElem->cnt = ++nInput;
      char *z = pElem->MarkupArg("value", 0);
      if (!z) z = "Submit";
      OXFrame *f = new OXTextButton(_canvas, new OHotString(z), pElem->cnt);
      f->TakeFocus(False);
      f->Associate(this);
      // TODO: bg color!
      f->Resize(f->GetDefaultSize());
      SizeAndLink(f, pElem);
      break;
    }

    case INPUT_TYPE_Text: {
      pElem->cnt = ++nInput;
      char *z = pElem->MarkupArg("maxlength", 0);
      int maxlen = z ? atoi(z) : 256;
      if (maxlen < 2) maxlen = 2;
      z = pElem->MarkupArg("size", 0);
      int size = z ? atoi(z) * 5 : 150;
      OXTextEntry *f = new OXTextEntry(_canvas, new OTextBuffer(maxlen),
                                       pElem->cnt);
      z = pElem->MarkupArg("value", 0);
      if (z) f->AddText(0, z);
      f->Resize(size, f->GetDefaultHeight());
      SizeAndLink(f, pElem);
      break;
    }

    case INPUT_TYPE_Password: {
      pElem->cnt = ++nInput;
      char *z = pElem->MarkupArg("maxlength", 0);
      int maxlen = z ? atoi(z) : 256;
      if (maxlen < 2) maxlen = 2;
      z = pElem->MarkupArg("size", 0);
      int size = z ? atoi(z) * 5 : 150;
      OXTextEntry *f = new OXSecretTextEntry(_canvas, new OTextBuffer(maxlen),
                                             pElem->cnt);
      z = pElem->MarkupArg("value", 0);
      if (z) f->AddText(0, z);
      f->Resize(size, f->GetDefaultHeight());
      SizeAndLink(f, pElem);
      break;
    }

    case INPUT_TYPE_Select: {  // listbox or dd-listbox?
      pElem->cnt = ++nInput;
      char *z = pElem->MarkupArg("size", 0);
      int size = z ? atoi(z) : 1;

      if (size == 1) {
        OXDDListBox *lb = new OXDDListBox(_canvas, pElem->cnt);
        AddSelectOptions((OXListBox *) lb->GetListBox(), pElem, pElem->pEnd);
        OXLBEntry *e = lb->GetListBox()->GetSelectedEntry();
        if (e) lb->Select(e->ID());
        lb->Resize(200, lb->GetDefaultHeight());
        lb->Associate(this);
        SizeAndLink(lb, pElem);
      } else {
        OXListBox *lb = new OXListBox(_canvas, pElem->cnt);
        z = pElem->MarkupArg("multiple", 0);
        if (z) lb->SetMultipleSelect(True);
        AddSelectOptions(lb, pElem, pElem->pEnd);
        lb->Resize(200, lb->GetDefaultHeight() * size);
        lb->Associate(this);
        SizeAndLink(lb, pElem);
      }
      break;
    }

    case INPUT_TYPE_TextArea: {
      pElem->cnt = ++nInput;
      char *z = pElem->MarkupArg("rows", 0);
      int rows = z ? atoi(z) : 10;
      z = pElem->MarkupArg("cols", 0);
      int cols = z ? atoi(z) : 10;
      OXTextEdit *f = new OXTextEdit(_canvas, 300, 200, pElem->cnt);
      OString str("");
      AppendText(&str, pElem, pElem->pEnd);
      f->InsertText(&str);
      SizeAndLink(f, pElem);
      break;
    }

    case INPUT_TYPE_Applet: {
      int result;

      OXFrame *f = ProcessApplet(pElem);
      if (!f) {
        pElem->Empty();
        break;
      }
      pElem->cnt = ++nInput;
      SizeAndLink(f, pElem);
      break;
    }

    default: {
      CANT_HAPPEN;
      pElem->flags &= ~HTML_Visible;
      pElem->style.flags |= STY_Invisible;
      pElem->frame = 0;
      break;
    }
  }

  return incomplete;
}


// Return the number of elments of type p in a form.

int OXHtml::FormCount(OHtmlInput *p, int radio) {
  OHtmlElement *q = p;

  switch (p->type) {
    case Html_SELECT:
      return p->subId;
    case Html_TEXTAREA:
    case Html_INPUT:
      if (radio && p->type == INPUT_TYPE_Radio)
        return p->subId;
      return ((OHtmlForm *)p->pForm)->elements;
    case Html_OPTION:
      while ((q = q->pPrev))
        if (q->type == Html_SELECT) return ((OHtmlInput *)q)->subId;
  }
  return -1;
}


// Add the DOM control information for form elements.

void OXHtml::AddFormInfo(OHtmlElement *p) {
  OHtmlElement *q;
  OHtmlForm *f;
  char *name, *z;
  int t;

  switch (p->type) {
    case Html_SELECT:
    case Html_TEXTAREA:
    case Html_INPUT: {
      OHtmlInput *input = (OHtmlInput *) p;
      if (!(f = formStart)) return;
      input->pForm = formStart;
      if (!f->pFirst)
        f->pFirst = p;
      if (formElemLast)
        formElemLast->iNext = input;
      formElemLast = input;
      input->inpId = inputIdx++;
      t = input->itype = InputType(input);
      if (t == INPUT_TYPE_Radio) {
        if ((name = p->MarkupArg("name", 0))) {
          for (q = f->pFirst; q; q = ((OHtmlInput *)q)->iNext) {
            if ((z = q->MarkupArg("name", 0)) && !strcmp(z, name)) {
              input->subId = radioIdx++;
              break;
            }
          }
          if (!q) input->subId = radioIdx = 0;
        }
      }
      break;
    }

    case Html_FORM:
      formStart = (OHtmlForm *) p;
      ((OHtmlForm *)p)->formId = nForm++;
      break;

    case Html_EndTEXTAREA:
    case Html_EndSELECT:
    case Html_EndFORM:
      formStart = 0;
      inputIdx = 0;
      radioIdx = 0;
      formElemLast = 0;
      break;

    case Html_OPTION:
      if (formElemLast && formElemLast->type == Html_SELECT)
        formElemLast->subId++;
      break;

    default:
      break;
  }
}


// The following array determines which characters can be put directly
// in a query string and which must be escaped.

static char needEscape[] = {
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
   1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0,
   1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1,
};
#define NeedToEscape(C) ((C)>0 && (C)<127 && needEscape[(int)(C)])

// Append to the given OString an encoded version of the given text.

void OXHtml::EncodeText(OString *str, const char *z) {
  int i;

  while (*z) {
    for (i = 0; z[i] && !NeedToEscape(z[i]); ++i) {}
    if (i > 0) str->Append(z, i);
    z += i;
    while (*z && NeedToEscape(*z)) {
      if (*z == ' ') {
        str->Append("+", 1);
      } else if (*z == '\n') {
        str->Append("%0D%0A", 6);
      } else if (*z == '\r') {
        // Ignore it...
      } else {
        char zBuf[5];
        sprintf(zBuf, "%%%02X", 0xff & *z);
        str->Append(zBuf, 3);
      }
      z++;
    }
  }
}


//----------------------------------------------------------------------

int OXHtml::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;
  OHtmlInput *p;

  switch (msg->type) {
    case MSG_BUTTON:
    case MSG_RADIOBUTTON:
    case MSG_CHECKBUTTON:
    case MSG_LISTBOX:
    case MSG_DDLISTBOX:
      for (p = firstInput; p; p = p->iNext) {
        if (p->cnt == wmsg->id) {
          switch (p->itype) {
            case INPUT_TYPE_Button:
            case INPUT_TYPE_Submit:
              if (p->pForm) {
                FormAction(p->pForm, wmsg->id);
              } else {
                printf("action, but no form!\n");
              }
              break;

            case INPUT_TYPE_Reset: {
              //ClearForm(p->pForm);
              OHtmlInput *pr;
              for (pr = firstInput; pr; pr = pr->iNext) {
                if (pr->pForm == p->pForm) {
                  switch (pr->itype) {
                    case INPUT_TYPE_Radio: {
                      OXRadioButton *rb = (OXRadioButton *) pr->frame;
                      if (pr->MarkupArg("checked", 0))
                        rb->SetState(BUTTON_DOWN);
                      else
                        rb->SetState(BUTTON_UP);
                      break;
                    }

                    case INPUT_TYPE_Checkbox: {
                      OXCheckButton *cb = (OXCheckButton *) pr->frame;
                      if (pr->MarkupArg("checked", 0))
                        cb->SetState(BUTTON_DOWN);
                      else
                        cb->SetState(BUTTON_UP);
                      break;
                    }

                    case INPUT_TYPE_Text:
                    case INPUT_TYPE_Password: {
                      OXTextEntry *te = (OXTextEntry *) pr->frame;
                      te->Clear();
                      const char *z = pr->MarkupArg("value", 0);
                      if (z) te->AddText(0, z);
                      break;
                    }

                    case INPUT_TYPE_Select: {
                      /**/
                      break;
                    }

                    default:
                      break;
                  }
                }
              }
              break;
            }

            case INPUT_TYPE_Radio: {
              OHtmlInput *pr;
              for (pr = firstInput; pr; pr = pr->iNext) {
                if ((pr->pForm == p->pForm) &&
                    (pr->itype == INPUT_TYPE_Radio)) {
                  if (pr != p) {
                    if (strcmp(pr->MarkupArg("name", ""),
                               p->MarkupArg("name", "")) == 0)
                      ((OXRadioButton *)pr->frame)->SetState(BUTTON_UP);
                  }
                }
              }
              break;
            }

            case INPUT_TYPE_Select: {
              break;
            }

            default:
              break;
          }
          return True;
        }
      }
      break;

    default:
      break;
  }

  return OXView::ProcessMessage(msg);
}
