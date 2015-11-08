/* A run program for Fvwm95 and the Xclass set.          */
/* Written by Frank Hall. April 4, 1997                  */
/* Modified May 14, 1997 to incorporate new OXFileDialog */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <X11/keysym.h>

#include <xclass/OXClient.h>
#include <xclass/OXWindow.h>
#include <xclass/OXFrame.h>
#include <xclass/OXTextButton.h>
#include <xclass/OXCheckButton.h>
#include <xclass/OXIcon.h>
#include <xclass/OXTextEntry.h>
#include <xclass/OXMsgBox.h>
#include <xclass/OString.h>
#include <xclass/OMimeTypes.h>
#include <xclass/OXFileDialog.h>
#include <xclass/utils.h>

#include "run.xpm"

#define MaxArgs          10

#define OKButtonID       1
#define CancelButtonID   2
#define BrowseButtonID   3
#define UseXTermID       4
#define RunTextEntryID   5

char *filetypes[] = { "All Files", "*",
                      NULL,        NULL };


class RunFrame : public OXMainFrame {
public:
  RunFrame(const OXWindow *p, int w, int h, unsigned long options);
  virtual ~RunFrame();

  virtual int ProcessMessage(OMessage *msg);

protected:
  OXCompositeFrame *DescriptionFrame;
  OXCompositeFrame *TextFrame;
  OXCompositeFrame *ButtonFrame;
  OXCompositeFrame *ButtonSubFrame;

  OXTextEntry *ProgramTextEntry;

  OXLabel *Description;
  OString *DescriptionText;
  OXIcon  *DescriptionIcon;

  OXLabel *OpenLabel;
  OString *OpenText;

  const OPicture *IconPicture;

  OXCheckButton *UseXTerm;
  OXTextButton *OKButton;
  OXTextButton *CancelButton;
  OXTextButton *BrowseButton;
};


OXClient *ClientX;
RunFrame *MainWindow;

main() 
{
  char mimerc[PATH_MAX];

  /* Create Client */
  ClientX = new OXClient;

  /* Create Main Window */
  MainWindow = new RunFrame(ClientX->GetRoot(), 250, 115, MAIN_FRAME | VERTICAL_FRAME);

  /* Map The Main Window */
  MainWindow->MapWindow();

  /* Activate The Whole Thing */
  ClientX->Run();

  return 0;
}


RunFrame::RunFrame(const OXWindow *p, int w, int h, unsigned long options) :
  OXMainFrame(p, w, h, options) {
    int dummy, width, height;
    unsigned int ScreenHeight, udummy;
    Window wdummy;

    /* Create Frame To Hold Description */
    DescriptionFrame = new OXCompositeFrame(this, 60, 20, HORIZONTAL_FRAME);

    /* Get Picture From Pool */
    IconPicture = _client->GetPicture("run.xpm", run_xpm);

    /* Create Icon */
    DescriptionIcon = new OXIcon(DescriptionFrame, IconPicture, 32, 32);

    /* Attach Icon To Description Frame */
    DescriptionFrame->AddFrame(DescriptionIcon, new OLayoutHints(LHINTS_LEFT,
                                                     10, 10, 10, 10));

    /* Create Text For Label */
    DescriptionText = new OString("Please enter the program to be run.");

    /* Create Label */
    Description = new OXLabel(DescriptionFrame, DescriptionText);

    /* Attach Label To Description Frame */
    DescriptionFrame->AddFrame(Description, new OLayoutHints(LHINTS_LEFT,
                                                 10, 10, 17, 10));


    /* Create Frame To Hold Text Box */
    TextFrame = new OXCompositeFrame(this, 60, 20, HORIZONTAL_FRAME);

    /* Create Text For Open Label */
    OpenText = new OString("Open:");

    /* Create Label */
    OpenLabel = new OXLabel(TextFrame, OpenText);

    /* Attach Label To Description Frame */
    TextFrame->AddFrame(OpenLabel, new OLayoutHints(LHINTS_LEFT,
                                        10, 10, 3, 10));

    /* Create Text Box */
    ProgramTextEntry = new OXTextEntry(TextFrame, NULL, RunTextEntryID);
    ProgramTextEntry->Associate(this);

    /* Set The Length Of The Text Box */
    ProgramTextEntry->Resize(225, ProgramTextEntry->GetDefaultHeight());

    /* Attach Text Box To Text Frame */
    TextFrame->AddFrame(ProgramTextEntry, new OLayoutHints(LHINTS_TOP,
                                               10, 10, 0, 0));

    /* Create Frame To Hold Buttons */
    ButtonFrame = new OXCompositeFrame(this, 60, 20, HORIZONTAL_FRAME);

    /* Create Check Box */
    UseXTerm = new OXCheckButton(ButtonFrame, new OHotString("Use &XTerm"),
                                 UseXTermID);

    /* Attach Check Box To Button Frame */
    ButtonFrame->AddFrame(UseXTerm, new OLayoutHints(LHINTS_CENTER_Y | LHINTS_LEFT,
                                         10, 5, 0, 0));

    /* Create SubFrame To Hold OK, Cancel and Browse Buttons */
    ButtonSubFrame = new OXCompositeFrame(ButtonFrame, 60, 20,
                                          HORIZONTAL_FRAME | FIXED_WIDTH);

    /* Create OK button */
    OKButton = new OXTextButton(ButtonSubFrame, new OHotString("OK"), OKButtonID);
    OKButton->Associate(this);
    OKButton->Disable();

    /* Attach Button To Button SubFrame */
    ButtonSubFrame->AddFrame(OKButton, new OLayoutHints(LHINTS_CENTER_Y | LHINTS_EXPAND_X,
                                         0, 5, 0, 0));

    /* Create Cancel button */
    CancelButton = new OXTextButton(ButtonSubFrame, new OHotString("Cancel"),
                                    CancelButtonID);
    CancelButton->Associate(this);

    /* Attach Button To Button SubFrame */
    ButtonSubFrame->AddFrame(CancelButton, new OLayoutHints(LHINTS_CENTER_Y | LHINTS_EXPAND_X,
                                             0, 5, 0, 0));

    /* Create Browse button */
    BrowseButton = new OXTextButton(ButtonSubFrame, new OHotString("&Browse"),
                                    BrowseButtonID);
    BrowseButton->Associate(this);

    /* Attach Button To Button SubFrame */
    ButtonSubFrame->AddFrame(BrowseButton, new OLayoutHints(LHINTS_CENTER_Y | LHINTS_EXPAND_X,
                                             0, 5, 0, 0));

    /* Attach SubFrame to Button Frame, resize accordingly */
    ButtonFrame->AddFrame(ButtonSubFrame, new OLayoutHints(LHINTS_TOP | LHINTS_RIGHT,
                                             0, 5, 0, 0));

    width = OKButton->GetDefaultWidth();
    width = max(width, CancelButton->GetDefaultWidth());
    width = max(width, BrowseButton->GetDefaultWidth());
    ButtonSubFrame->Resize(3 * (width + 10), ButtonSubFrame->GetDefaultHeight());

    SetDefaultAcceptButton(OKButton);
    SetDefaultCancelButton(CancelButton);

    /* Attach Description Frame To Main Window */
    AddFrame(DescriptionFrame, new OLayoutHints(LHINTS_TOP, 0, 0, 1, 0));

    /* Attach Text Frame To Main Window */
    AddFrame(TextFrame, new OLayoutHints(LHINTS_TOP, 0, 0, 1, 0));

    /* Attach Button Frame To Main Window */
    AddFrame(ButtonFrame, new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X,
                                           0, 0, 8, 12));


    /* Set The Name Displayed In The Title Bar */
    SetWindowName("Run...");

    /* Set class name and resource names */
    SetClassHints("Run", "Run");

    /* Map All Sub Windows (And Frames) Of The Main Window */
    MapSubwindows();

    /* Set The Size Of The Main Window To Something Decent */
    width  = GetDefaultWidth();
    height = GetDefaultHeight();
    Resize(width, height);
    SetWMSize(width, height);
    SetWMSizeHints(width, height, width, height, 0, 0);

    /* Move to a decent position */
    ScreenHeight = _client->GetDisplayHeight();
    Move(20, ScreenHeight - height - 100);

    /* Tell the WM we want that position */
    SetWMPosition(20, ScreenHeight - height - 100);

    SetMWMHints(MWM_DECOR_ALL | MWM_DECOR_RESIZEH | MWM_DECOR_MAXIMIZE,
                MWM_FUNC_ALL | MWM_FUNC_RESIZE | MWM_FUNC_MAXIMIZE, 
                MWM_INPUT_MODELESS);

    /* Set the default input focus to the text entry widget */
    SetFocusOwner(ProgramTextEntry);
}

RunFrame::~RunFrame() {
  _client->FreePicture(IconPicture);
}

int RunFrame::ProcessMessage(OMessage *msg) {
  int  i, j, k, Slen;
  char errmsg[256];
  char P2Run[256];
  char *Parg[MaxArgs];
  char PDirectory[PATH_MAX];
  OFileInfo fi;
  OWidgetMessage *wmsg;

  switch (msg->type) {

  case MSG_BUTTON:
    switch (msg->action) {

    case MSG_CLICK:
      wmsg = (OWidgetMessage *) msg;
      switch (wmsg->id) {

      case OKButtonID:
	strcpy(P2Run, ProgramTextEntry->GetString());
	if (strlen(P2Run) > 0) {
	  Slen = strlen(P2Run);
	  for (i=0, j=0, k=0; (i<Slen) && (j<MaxArgs); i++) {
	    if (P2Run[i] == ' ') {
	      P2Run[i] = '\0';
	      Parg[j++] = &P2Run[k];
	      k = i + 1;
	    }
	  }

	  Parg[j++] = &P2Run[k];
	  for (; j<MaxArgs; j++) Parg[j] = NULL;

	  if (UseXTerm->GetState() == BUTTON_DOWN) {
	    j = execlp("xterm", " ", "-e", Parg[0], Parg[1], Parg[2], Parg[3], Parg[4], Parg[5], Parg[6], Parg[7], Parg[8], Parg[9], 0);
	  } else {
	    j = execlp(Parg[0], " ", Parg[1], Parg[2], Parg[3], Parg[4], Parg[5], Parg[6], Parg[7], Parg[8], Parg[9], 0);
	  }

	  if (j == -1) {
	    strcpy(errmsg, "Could not open ");
	    strcat(errmsg, ProgramTextEntry->GetString());
	    strcat(errmsg, ".");
	    new OXMsgBox(this->GetParent(), this, new OString("Run Error"),
                         new OString(errmsg), MB_ICONSTOP, ID_OK);
	  }
	}
	break;

      case CancelButtonID:
	CloseWindow();
	break;

      case BrowseButtonID:
	fi.file_types = filetypes;
	new OXFileDialog(ClientX->GetRoot(), this, FDLG_OPEN, &fi);
	if (fi.filename) {
	  getcwd(PDirectory, PATH_MAX - 1);
	  if (PDirectory[strlen(PDirectory) - 1] != '/')
            strcat(PDirectory, "/");
	  strcat(PDirectory, fi.filename);
	  ProgramTextEntry->Clear();
	  ProgramTextEntry->AddText(0, PDirectory);
	  OKButton->Enable();
	}
	break;
      }
      break;

    }
    break;

  case MSG_TEXTENTRY:
    switch (msg->action) {
    case MSG_TEXTCHANGED:
      ProgramTextEntry->GetTextLength() ? OKButton->Enable() : OKButton->Disable();
      break;
    }
    break;

  }

  return True;
}
