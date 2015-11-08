#ifndef __BCKGND_TAB_H
#define __BCKGND_TAB_H

#include <xclass/OXClient.h>
#include <xclass/OXWindow.h>
#include <xclass/OXCompositeFrame.h>
#include <xclass/OXIcon.h>
#include <xclass/OXLabel.h>
#include <xclass/OXTextButton.h>
#include <xclass/OXCheckButton.h>
#include <xclass/OXRadioButton.h>
#include <xclass/OXGroupFrame.h>
#include <xclass/OXDDListBox.h>
#include <xclass/OString.h>
#include <xclass/utils.h>

//----------------------------------------------------------------------

class OXBackgroundTab : public OXCompositeFrame {
public:
  OXBackgroundTab(const OXWindow *tab, const OPicture *mon);
  virtual ~OXBackgroundTab();

protected:
  //OLayoutHints *CommonTLayout, *CommonXLayout;
  //const OPicture *Energy_Star, *Monitor;
};


#endif  // __BCKGND_TAB_H
