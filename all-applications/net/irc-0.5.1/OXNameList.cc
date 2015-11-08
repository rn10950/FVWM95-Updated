#include <xclass/OXCompositeFrame.h>
#include <xclass/OXButton.h>
#include <xclass/OXFont.h>
#include <xclass/OGC.h>

#include "OXNameList.h"
#include "OIrcMessage.h"

#include "pixmaps/nl-chanop.xpm"
#include "pixmaps/nl-voice.xpm"
#include "pixmaps/nl-friend.xpm"
#include "pixmaps/nl-ignored.xpm"


const OPicture *OXName::_opPic = NULL;
const OPicture *OXName::_voicedPic = NULL;
const OPicture *OXName::_ignoredPic = NULL;
const OPicture *OXName::_friendPic = NULL;

//-----------------------------------------------------------------

extern OLayoutHints *topexpandxlayout0;

OXNameList::OXNameList(const OXWindow *p, OXPopupMenu *popup, int w, int h,
                       unsigned int options) :
  OXVerticalFrame(p, w, h, options | VERTICAL_FRAME) {

    _popup = popup;
    _stick = True;
    _current = NULL;

    XGrabButton(GetDisplay(), Button1, AnyModifier, _id, True,
		ButtonPressMask | ButtonReleaseMask |
		EnterWindowMask,
		GrabModeAsync, GrabModeAsync, None, None);
}

OXNameList::~OXNameList() {
  SListFrameElt *ptr;
  OXName *t;

  for (ptr = _flist; ptr != NULL; ptr = ptr->next) {
    t = (OXName *) ptr->frame;
    // ...
  }
}

void OXNameList::OpNick(const char *nick, int onoff) {
  OXName *n = GetName(nick);
  if (n) {
    if (onoff && !n->IsOp()) {
      n->SetFlags(NICK_OPED);
    } else if (!onoff && n->IsOp()) {
      n->ClearFlags(NICK_OPED);
    }
  }	
}

void OXNameList::VoiceNick(const char *nick, int onoff) {
  OXName *n = GetName(nick);
  if (n) {
    if (onoff && !n->IsVoiced()) {
      n->SetFlags(NICK_VOICED);
    } else if (!onoff && n->IsVoiced()) {
      n->ClearFlags(NICK_VOICED);
    }
  }
}

void OXNameList::IgnoreNick(const char *nick, int onoff) {
  OXName *n = GetName(nick);
  if (n) {
    if (onoff && !n->IsIgnored()) {
      n->SetFlags(NICK_IGNORED);
    } else if (!onoff && n->IsIgnored()) {
      n->ClearFlags(NICK_IGNORED);
    }
  }
}

void OXNameList::FriendNick(const char *nick, int onoff) {
  OXName *n = GetName(nick);
  if (n) {
    if (onoff && !n->IsFriend()) {
      n->SetFlags(NICK_FRIEND);
    } else if (!onoff && n->IsFriend()) {
      n->ClearFlags(NICK_FRIEND);
    }
  }
}

void OXNameList::NormalNick(const char *nick) {
  OXName *n = GetName(nick);
  if (n) {
    n->ClearFlags(NICK_VOICED | NICK_OPED | NICK_IGNORED | NICK_FRIEND);
  }
}

int OXNameList::HandleButton(XButtonEvent *event) {
  SListFrameElt *ptr;

  OXName *target;

  // We don't need to check the button number as XGrabButton will
  // only allow button1 events

  if (event->type == ButtonPress) {

    target = (OXName *) _client->GetWindowById(event->subwindow);

    if (target != NULL) {
      _stick = True;

      if (target != _current) {
        // Deactivate all others
        for (ptr = _flist; ptr != NULL; ptr = ptr->next)
          ((OXName *)ptr->frame)->SetState(False);

        target->SetState(True);
        _stick = True;
        _current = target;

        XGrabPointer(GetDisplay(), _id, True,
                     ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                     GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
      }
    }
  }

  if (event->type == ButtonRelease) {
    if (_stick) {
      _stick = False;
      return True;
    }
    XUngrabPointer(GetDisplay(), CurrentTime);

    for (ptr = _flist; ptr != NULL; ptr = ptr->next)
      ((OXName *)ptr->frame)->SetState(False);

    if (_current != NULL) {
      target = _current; /* tricky, because WaitFor */
      _current = NULL;
      target->DoSendMessage();
    }
  }

  return True;
}

int OXNameList::HandleMotion(XMotionEvent *event) {
  SListFrameElt *ptr;
  OXName *target;

  _stick = False; // use some threshold!

  if (_current && _current->HandleMotion(event)) return True;

  target = (OXName *) _client->GetWindowById(event->subwindow);

  if (target != NULL && target != _current) {
    // Deactivate all others
    for (ptr = _flist; ptr != NULL; ptr = ptr->next)
      ((OXMenuTitle *)ptr->frame)->SetState(False);

    target->SetState(True);
    _stick = True;
    _current = target;
  }

  return True;
}

void OXNameList::AddPopup(OHotString *s, OXPopupMenu *menu,
                          OLayoutHints *l) {
  OXName *name;

  AddFrame(name = new OXName(this, s, menu), l);
  name->Associate(_msgObject);
}

OXName *OXNameList::GetName(const char *s) {
  SListFrameElt *ptr;
  OXName *t;

  for (ptr = _flist; ptr != NULL; ptr = ptr->next) {
    t = (OXName *) ptr->frame;
    if (!strcasecmp(s, t->_label->GetString())) return t;
  }
  return NULL;
}

void OXNameList::AddName(const char *s) {
  int flags = 0;
  const char *nick = s;
  OXName *name;

  while (1) {
    if (*nick == '@') {
      flags |= NICK_OPED;
      ++nick;
    } else if (*nick == '+') {
      flags |= NICK_VOICED;
      ++nick;
    } else {
      break;
    }
  }

  name = GetName(nick);
  if (!name) {
    AddPopup(new OHotString(nick), _popup, topexpandxlayout0);
    MapSubwindows();
    Layout();
    name = GetName(nick);
  }
  name->ClearFlags(NICK_OPED | NICK_VOICED);
  name->SetFlags(flags);
}

void OXNameList::RemoveName(const char *s) {
  OXName *t = GetName(s);

  if (t) {
    RemoveFrame(t);
    XDestroyWindow(GetDisplay(), t->GetId());
    delete t;
    Layout();
  }
}

void OXNameList::ChangeName(const char *o, const char *n) {
  const char *nick = n;
  OXName *t = GetName(o);

  // assume that the modes are the same, so don't check for '@' and '+'

  if (t) {
    delete t->_label;
    t->_label = new OHotString(nick);
    t->_DoRedraw();
  }
}

//-----------------------------------------------------------------

OXName::OXName(const OXWindow *p, OHotString *s, OXPopupMenu *menu,
               unsigned int options) :
  OXMenuTitle(p, s, menu, options) {

  if (!_opPic) {
    _opPic = _client->GetPicture("nl-chanop.xpm", nl_chanop_xpm);
    _voicedPic = _client->GetPicture("nl-voice.xpm",  nl_voice_xpm);
    _ignoredPic = _client->GetPicture("nl-ignored.xpm", nl_ignored_xpm);
    _friendPic = _client->GetPicture("nl-friend.xpm",  nl_friend_xpm);
  }

  _tw = _font->TextWidth(_label->GetString(), _label->GetLength());
  _th = _font->TextHeight();

  _flags = 0;
} 

void OXName::SetState(int state) {
  _state = state;
  if (state) {
    if (_menu != NULL) {
      int ax, ay;
      Window wdummy;

      XTranslateCoordinates(GetDisplay(), _id, (_menu->GetParent())->GetId(),
                            _w, 2, &ax, &ay, &wdummy);

      ODimension ps = _menu->GetDefaultSize();

      int dpyw = _client->GetDisplayWidth();
      int dpyh = _client->GetDisplayHeight();

      if (ax + ps.w > dpyw) ax -= ps.w + _w;
      if (ax + ps.w > dpyw) ax = dpyw - ps.w - 1;  // if still off-screen
      if (ay + ps.h > dpyh) ay -= ps.h - _h;

      if (IsVoiced())
        _menu->CheckEntry(C_SPEAK);
      else
        _menu->UnCheckEntry(C_SPEAK);

      if (IsOp())
        _menu->CheckEntry(C_CHAN_OP);
      else
        _menu->UnCheckEntry(C_CHAN_OP);

      _menu->PlaceMenu(ax, ay, True, False); //True);
    }
  } else {
    if (_menu != NULL) {
      _ID = _menu->EndMenu();
    }
  }
  NeedRedraw();
}

void OXName::SetFlags(int flags) {
  _flags |= flags;
  NeedRedraw();
}

void OXName::ClearFlags(int flags) {
  _flags &= ~flags;
  NeedRedraw();
}

void OXName::_DoRedraw() {
  const char *name = _label->GetString();
  int x, y, xp;
  OFontMetrics fm;

  _font->GetFontMetrics(&fm);

  x = (_w - _font->TextWidth(name, strlen(name))) >> 1;
  xp = 3 + _insets.l;
  y = 2;
  if (_state) {
    _options |= SUNKEN_FRAME;
    _options &= ~RAISED_FRAME;
    ++x; ++y; ++xp;
  } else {
    _options |= RAISED_FRAME;
    _options &= ~SUNKEN_FRAME;
  }
  OXFrame::_DoRedraw();
  if (_state) _DrawTrianglePattern(_normGC->GetGC(), _w-10, y+3, _w-6, y+11);
  _label->Draw(GetDisplay(), _id, _normGC->GetGC(), x, y + fm.ascent);

  if (IsOp()) {
    _opPic->Draw(GetDisplay(), _id, _normGC->GetGC(), xp, y+1);
    xp += _opPic->GetWidth() + 1;
  }
  if (IsVoiced()) {
    _voicedPic->Draw(GetDisplay(), _id, _normGC->GetGC(), xp, y+1);
    xp += _voicedPic->GetWidth() + 1;
  }
  if (IsIgnored()) {
    _ignoredPic->Draw(GetDisplay(), _id, _normGC->GetGC(), xp, y+1);
    xp += _ignoredPic->GetWidth() + 1;
  }
  if (IsFriend()) {
    _friendPic->Draw(GetDisplay(), _id, _normGC->GetGC(), xp, y+1);
  }
}

void OXName::_DrawTrianglePattern(GC gc, int l, int t, int r, int b) {
  XPoint points[3];

  int m = (t + b) >> 1;

  points[0].x = l;
  points[0].y = t;
  points[1].x = l;
  points[1].y = b;
  points[2].x = r;
  points[2].y = m;

  XFillPolygon(GetDisplay(), _id, gc, points, 3, Convex, CoordModeOrigin);
}

void OXName::DoSendMessage() {
  if (_ID != -1) {
    ONickListMessage message(IRC_NICKLIST, MSG_CLICK, _ID, 
                             _label->GetString());
    SendMessage(_msgObject, &message);
  }
}
