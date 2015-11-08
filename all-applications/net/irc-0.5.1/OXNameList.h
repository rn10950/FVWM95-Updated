#ifndef __OXNAMELIST_H
#define __OXNAMELIST_H

#include <xclass/OXMenu.h>
#include <xclass/OMessage.h>

#define NICK_OPED    (1<<0)
#define NICK_VOICED  (1<<1)
#define NICK_IGNORED (1<<2)
#define NICK_FRIEND  (1<<3)

#define C_WHOIS       1
#define C_MESSAGE     2
#define C_NOTICE      3
#define C_NOTIFY      4
#define C_SPEAK       5
#define C_CHAN_OP     6
#define C_KICK        7
#define C_BAN         8
#define C_BANKICK     9
#define C_KILL       10

class OXNameList;


//----------------------------------------------------------------------

class ONickListMessage : public OWidgetMessage {
public:
  ONickListMessage(int typ, int act, int id, const char *n) :
    OWidgetMessage(typ, act, id) { name = n; }

public:
  const char *name;
};

class OXName : public OXMenuTitle {
protected:
  static const OPicture *_opPic, *_voicedPic, *_ignoredPic, *_friendPic;

public:
  OXName(const OXWindow *p, OHotString *s, OXPopupMenu *menu,
         unsigned int options = 0);

  virtual ODimension GetDefaultSize() const 
    { return ODimension(_tw+8, _th+4); }

  void SetState(int state);

  void SetFlags(int flags);
  void ClearFlags(int flags);

  bool IsOp() const { return _flags & NICK_OPED; }
  bool IsVoiced() const { return _flags & NICK_VOICED; }
  bool IsFriend() const { return _flags & NICK_FRIEND; }
  bool IsIgnored() const { return _flags & NICK_IGNORED; }
  
  void DoSendMessage();

  friend class OXNameList;

protected:
  void _DoRedraw();
  void _DrawTrianglePattern(GC gc, int l, int t, int r, int b);

  int _tw, _th;
  int _flags;
};

class OXNameList : public OXVerticalFrame {
public:
  OXNameList(const OXWindow *p, OXPopupMenu *popup, int w, int h,
	     unsigned int options = VERTICAL_FRAME);
  virtual ~OXNameList();

  void AddPopup(OHotString *s, OXPopupMenu *menu, OLayoutHints *l);

  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleMotion(XMotionEvent *event);

  OXName *GetName(const char *s);
  void AddName(const char *s);
  void RemoveName(const char *s);
  void ChangeName(const char *o, const char *n);
  void OpNick(const char *nick, int onoff = True);
  void VoiceNick(const char *nick, int onoff = True);
  void IgnoreNick(const char *nick, int onoff = True);
  void FriendNick(const char *nick, int onoff = True);
  void NormalNick(const char *nick);

protected:
  OXName *_current;
  int _stick; 
  OXPopupMenu *_popup;
};


#endif  // __OXNAMELIST_H
