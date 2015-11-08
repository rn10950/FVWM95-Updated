#ifndef __SCRNSAV_TAB_H
#define __SCRNSAV_TAB_H

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

class OXScreenSaversTab : public OXCompositeFrame {
public:
  OXScreenSaversTab(const OXWindow *tab, const OPicture *mon);
  virtual ~OXScreenSaversTab();

protected:
  OLayoutHints *CommonTLayout, *CommonXLayout, *CommonLLayout;
  //const OPicture *Energy_Star, *Monitor;

  OLayoutHints *gfly;
  OXDDListBox *lbf;
};


#endif  // __SCRNSAV_TAB_H
