#include <stdlib.h>
#include <stdio.h>

#include "main.h"
#include "settings_tab.h"

//#include "screen.xpm"


//----------------------------------------------------------------------

OXSettingsTab::OXSettingsTab(const OXWindow *tab, const OPicture *mon) :
  OXCompositeFrame(tab, 100, 100, VERTICAL_FRAME) {

    CommonXLayout = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y,
                                     2, 2, 2, 2);
    CommonTLayout = new OLayoutHints(LHINTS_TOP | LHINTS_CENTER_X,
                                     2, 2, 2, 2);

    AddFrame(new OXIcon(this, mon, 185, 170), CommonTLayout);
//    AddFrame(new OXTextButton(this, new OHotString("&Test"), 0),
//                                    CommonXLayout);
}

OXSettingsTab::~OXSettingsTab() {
}
