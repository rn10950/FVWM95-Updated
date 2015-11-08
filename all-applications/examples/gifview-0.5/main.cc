#include "OXGifFrame.h"

#include <xclass/OXClient.h>
#include <xclass/OXMainFrame.h>
#include <xclass/OTimer.h>
#include <xclass/utils.h>

//---------------------------------------------------------------

class OXGifViewer : public OXMainFrame {
public:
  OXGifViewer(const OXWindow *p, char *filename, int w, int h);
  virtual ~OXGifViewer();

  virtual int HandleKey(XKeyEvent *event);

private:
  OXGifFrame *GifPicture;
};

//---------------------------------------------------------------

OXClient *clientX;

int main(int argc, char *argv[]) {
  OXGifViewer *viewer;

  if (argc == 1) {
    printf("USAGE: %s giffile\n", argv[0]);
    printf("\t   s   - start / stop animation\n");
    printf("\t space - step animation\n");
    printf("\t   q   - quit\n");
    exit(0);
  }
  
  clientX= new OXClient;
  viewer = new OXGifViewer(clientX->GetRoot(), argv[1], 10, 10);
  
  clientX->Run();
}

//---------------------------------------------------------------

OXGifViewer::OXGifViewer(const OXWindow *p, char *filename,
                         int w, int h) :
  OXMainFrame(p, w, h) {

    GifPicture = new OXGifFrame(this, filename);
    AddFrame(GifPicture, new OLayoutHints(LHINTS_CENTER_X | LHINTS_CENTER_Y));

//    if (GifPicture->isInvalidGif)
//      FatalError("This is NOT a valid GifFile !!!\n");

    SetWindowName("Gif Viewer");
    SetIconName("Gif Viewer");
    SetClassHints("XCGifView", "gifview");

    AddInput(KeyPressMask);
//    Resize(GifPicture->GetWidth(), GifPicture->GetHeight());
    Resize(GetDefaultSize());
    MapSubwindows();
    MapWindow();
}

OXGifViewer::~OXGifViewer() {
}

int OXGifViewer::HandleKey(XKeyEvent *event) {
  char tmp[10];
  KeySym keysym;
  XComposeStatus compose = { NULL, 0 };

  if (OXMainFrame::HandleKey(event)) return True;

  if (event->type == KeyPress) {
    XLookupString(event, tmp, sizeof(tmp)-1, &keysym, &compose);
    if (tmp[0] == 's' || tmp[0] == 'S') {
      if (GifPicture->AnimationRunning())  // If the animation is not running
        GifPicture->StopAnimation();       // start it, else stop it.
      else
        GifPicture->StartAnimation();
    } else if (tmp[0] == ' ') {
      if (GifPicture->AnimationRunning())  // If the animation is running
        GifPicture->StopAnimation();       // stop it.
      GifPicture->StepAnimation();
    } else if (tmp[0] == 'q' || tmp[0] == 'Q') {
      delete this;
      delete clientX;
      exit(0);
    }
  }

  return True;
}
