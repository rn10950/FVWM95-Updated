#include <unistd.h>
#include <X11/keysym.h>
#include <xclass/OX3dLines.h>
#include <xclass/utils.h>

#include "OXChannelDialog.h"

extern OSettings  *foxircSettings;

#define CHN_ID_TEXT		1001
#define CHN_ID_LIST		1002
#define CHN_ID_ADD		1003
#define CHN_ID_EDIT		1004
#define CHN_ID_DELETE		1005
#define CHN_ID_JOIN		1006
#define CHN_ID_NAMES		1007
#define CHN_ID_OK		1008
#define CHN_ID_CHECK		1009


//-----------------------------------------------------------------------

OXChannelDialog::OXChannelDialog(const OXWindow *p, OXIrc *main,
                                 unsigned long options) :
  OXTransientFrame(p, main, 10, 10, options) {

    int width = 0, height = 0;

    OLayoutHints *butlay = new OLayoutHints(LHINTS_EXPAND_X, 2, 2, 2, 2);
    OLayoutHints *expand = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y,
                                            2, 2, 2, 2);
    OLayoutHints *lefty = new OLayoutHints(LHINTS_NORMAL, 2, 2, 2, 2);
    OLayoutHints *leftcent = new OLayoutHints(LHINTS_LEFT | LHINTS_CENTER_Y,
                                              2, 2, 2, 2);
    OLayoutHints *cent = new OLayoutHints(LHINTS_CENTER_X, 3, 3, 3, 3);

    _irc = main;

    _v1 = new OXVerticalFrame(this, 10, 10);
    AddFrame(_v1,expand);

    _v2 = new OXVerticalFrame(this, 10, 10, FIXED_WIDTH);
    AddFrame(_v2,leftcent);

    _lab = new OXLabel(_v1, new OHotString("&Channel to join:"));
    _v1->AddFrame(_lab, lefty);

    _te = new OXTextEntry(_v1, new OTextBuffer(100), CHN_ID_TEXT);
    _v1->AddFrame(_te, butlay);
    _te->Associate(this);

    _lb = new OXListBox(_v1, CHN_ID_LIST);
    int   i;
    const OXSNode *ptr;
    for (ptr = foxircSettings->GetChannelList()->GetHead(), i = 1;
         ptr != NULL;
         ptr = ptr->next, ++i) {
      OChannelInfo *e = (OChannelInfo *) ptr->data;
      if (e->name) _lb->AddEntry(new OString(e->name), i);
    }

    _v1->AddFrame(_lb, expand);
    _lb->Associate(this);

    _onconnect = new OXCheckButton(_v1, new OHotString("&Pop up on Connect"),
                                   CHN_ID_CHECK);
    _v1->AddFrame(_onconnect, cent);

    if (foxircSettings->CheckMisc(P_MISC_POPUP_CHAN_CN))
      _onconnect->SetState(BUTTON_DOWN);
    else
      _onconnect->SetState(BUTTON_UP);
 
    _join = new OXTextButton(_v2, new OHotString("&Join"), CHN_ID_JOIN);
    _v2->AddFrame(_join, butlay);
    _join->Disable();
    _join->Associate(this);
    width = _join->GetDefaultWidth();

    _add = new OXTextButton(_v2, new OHotString("&Add"), CHN_ID_ADD);
    _v2->AddFrame(_add, butlay);
    _add->Disable();
    _add->Associate(this);
    width = max(width, _add->GetDefaultWidth());

//    _edit = new OXTextButton(_v2, new OHotString("&Edit"), CHN_ID_EDIT);
//    _v2->AddFrame(_edit, butlay);
//    _edit->Disable();
//    _edit->Associate(this);
//    width = max(width, _edit->GetDefaultWidth());

    _delete = new OXTextButton(_v2, new OHotString("&Remove"), CHN_ID_DELETE);
    _v2->AddFrame(_delete, butlay);
    _delete->Disable();
    _delete->Associate(this);
    width = max(width, _delete->GetDefaultWidth());

    _names = new OXTextButton(_v2, new OHotString("&Names"), CHN_ID_NAMES);
    _v2->AddFrame(_names, butlay);
    _names->Disable();
    _names->Associate(this);
    width = max(width, _names->GetDefaultWidth());

    _v2->AddFrame(new OXHorizontal3dLine(_v2), butlay);

    _ok = new OXTextButton(_v2, new OHotString("&Close"), CHN_ID_OK);
    _v2->AddFrame(_ok, butlay);
    _ok->Associate(this);
    width = max(width, _ok->GetDefaultWidth());

    _v2->Resize(width+20, _v2->GetDefaultHeight());

//    Resize(width = 250, height = 300);
    width  = GetDefaultWidth();
    height = GetDefaultHeight() *2;
    Resize(width, height);

    CenterOnParent();

    SetWMSize(width, height);
    SetWMSizeHints(width, height, width, height, 0, 0);
    SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_RESIZEH | MWM_DECOR_MAXIMIZE |
                                MWM_DECOR_MINIMIZE | MWM_DECOR_MENU,
                MWM_FUNC_ALL | MWM_FUNC_RESIZE | MWM_FUNC_MAXIMIZE |
                               MWM_FUNC_MINIMIZE,
                MWM_INPUT_MODELESS);

    SetWindowName("Join Channel...");
    SetIconName("Channel Dialog");
    SetClassHints("MsgBox", "MsgBox");

    SetDefaultAcceptButton(_join);
    SetDefaultCancelButton(_ok);
    SetFocusOwner(_te);

    MapSubwindows();
    Layout();
    MapWindow();
}

int OXChannelDialog::CloseWindow() {
  foxircSettings->Save();
  return OXTransientFrame::CloseWindow();
}

int OXChannelDialog::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;
  OTextEntryMessage *tmsg = (OTextEntryMessage *) msg;
  
  switch (msg->type) {
    case MSG_LISTBOX:
      switch (msg->action) {
         case MSG_CLICK:
               {
               OXTextLBEntry *te = (OXTextLBEntry *)_lb->GetSelectedEntry();
               if (!te) return true;
               _te->Clear();
               _te->AddText(0, te->GetText()->GetString());
               _add->Disable();
               _delete->Enable();
               _join->Enable();
               }
               break;
      }
      break;

    case MSG_MENU:
    case MSG_BUTTON:
      switch (msg->action) {
        case MSG_CLICK:
        switch (wmsg->id) {
	  case CHN_ID_LIST:
//	  case CHN_ID_EDIT:
	  case CHN_ID_NAMES:
	  case CHN_ID_CHECK:
		break;
	  case CHN_ID_ADD:
                _DoAdd();
                break;
	  case CHN_ID_JOIN:
		_DoJoin();
		break;
	  case CHN_ID_DELETE:
                _DoRemove();
                break;
          case CHN_ID_OK:
                CloseWindow();
                break;
          }
        break;
      }
      break;

    case MSG_TEXTENTRY:
      switch (msg->action) {
        case MSG_TEXTCHANGED:
          switch (tmsg->keysym) {
            case XK_Return:
	      _DoJoin();
              break;

            default:
              if (_te->GetTextLength() > 0) {
                _add->Enable();
                _join->Enable();
              } else {  
                _add->Disable();
                _join->Disable();
              }
              break;

          }
          break;
      }
      break;
  }

  return True;
}

void OXChannelDialog::_DoJoin() {
  if (_te->GetTextLength() > 0)
    _irc->JoinChannel(_te->GetString());
}

void OXChannelDialog::_DoAdd() {
  if (_te->GetTextLength() > 0) {
    OChannelInfo *info = new OChannelInfo();
    info->name = StrDup(_te->GetString());
    int i = foxircSettings->GetChannelList()->NoOfItems();
    foxircSettings->GetChannelList()->Add(i+1, (XPointer) info);
    _lb->AddEntry(new OString(info->name), i+1);
  }
}

void OXChannelDialog::_DoRemove() {
  OXTextLBEntry *te = (OXTextLBEntry *) _lb->GetSelectedEntry();
  if (!te) return;
  OXSNode *e = foxircSettings->GetChannelList()->GetNode(te->ID());
  foxircSettings->GetChannelList()->Remove(te->ID());
  if (e) delete e->data;  // delete the OChannelInfo item
  _lb->RemoveEntry(te->ID());
}
