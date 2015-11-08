#ifndef __SETTINGS_TAB_H
#define __SETTINGS_TAB_H

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

class OXSettingsTab : public OXCompositeFrame {
public:
  OXSettingsTab(const OXWindow *tab, const OPicture *mon);
  virtual ~OXSettingsTab();

protected:
  OLayoutHints *CommonTLayout, *CommonXLayout;
  //const OPicture *Energy_Star, *Monitor;
};


#endif  // __SETTINGS_TAB_H
