#ifndef __APPEARANCE_TAB_H
#define __APPEARANCE_TAB_H

#include <vector>

#include <xclass/OXClient.h>
#include <xclass/OXWindow.h>
#include <xclass/OXCompositeFrame.h>
#include <xclass/OXColorSelect.h>
#include <xclass/OXLabel.h>
#include <xclass/OXTextButton.h>
#include <xclass/OXDDListBox.h>
#include <xclass/OString.h>
#include <xclass/utils.h>


class OXGC;
class OXFont;
class Scheme;

//----------------------------------------------------------------------

class OXSampleView : public OXFrame {
public:
  OXSampleView(const OXWindow *p);
  ~OXSampleView();
  
  void ChangeSettings(Scheme *scheme);

  void LoadSchemes();
  void SaveSchemes();

protected:
  virtual void _Resized();
  virtual void _DoRedraw();

  void _CreatePixmap();
  void _Draw();
  
  void _DrawWindow(Drawable dst, int x, int y, int w, int h, int active);
  void _DrawButton(Drawable dst, int x, int y, int w, int h, const OPicture *p);

  Pixmap _pix;
  OXGC *_drawGC;
  unsigned long _desktopBgndPixel,
                _frameBgndPixel, _frameFgndPixel,
                _selBgndPixel, _selFgndPixel,
                _docBgndPixel, _docFgndPixel,
                _hilitePixel, _shadowPixel; //_blackPixel;
  const OXFont *_menuFont, *_iconFont, *_statusFont, *_defaultFont,
               *_tipFont, *_docFixedFont, *_docPropFont;
  int _pixw, _pixh;
  int _wmBorderWidth, _wmTitleHeight;
  const OPicture *_tbutton[3];
};

class OXAppearanceTab : public OXCompositeFrame {
public:
  OXAppearanceTab(const OXWindow *tab, const OPicture *mon);
  virtual ~OXAppearanceTab();

  virtual int ProcessMessage(OMessage *msg);

protected:
  int InitServer();
  int SetProperty();
  int SelItem(int ix);
  
  int LoadSchemes();
  int SaveSchemes();
  
  int SelectScheme(const char *name);

  void PrintSettings();

  char *ColorName(unsigned long color);
  char *ColorName(OColor color);
  char *FontName(const OXFont *font);
  char *PixmapName(const OPicture *pic);

  OXSampleView  *_sample;
  OXDDListBox   *ddlb;
  OLayoutHints  *ly1, *ly2;
  OXColorSelect *fgtr, *bgtr;
  OXTextButton  *fontb, *applyb;

  Atom _XCLASS_RESOURCES;
  char *_mimeFileName;
  char *_currentFont, *_currentFG, *_currentBG;
  std::vector<Scheme *> _schemes;
  Scheme *currentScheme;
  Colormap root_colormap;

  OLayoutHints *CommonCLayout;
};


#endif  // __APPEARANCE_TAB_H
