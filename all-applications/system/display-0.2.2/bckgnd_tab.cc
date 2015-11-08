#include <stdlib.h>
#include <stdio.h>

#include "main.h"
#include "bckgnd_tab.h"

//#include "screen.xpm"


//----------------------------------------------------------------------

OXBackgroundTab::OXBackgroundTab(const OXWindow *tab, const OPicture *mon) :
  OXCompositeFrame(tab, 100, 100, VERTICAL_FRAME) {
    OXCompositeFrame *hf, *gf1, *gf2;
    OLayoutHints *CommonXLayout, *CommonTLayout;

    CommonXLayout = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y,
                                     2, 2, 2, 2);
    CommonTLayout = new OLayoutHints(LHINTS_TOP | LHINTS_CENTER_X,
                                     2, 2, 2, 2);

    AddFrame(new OXIcon(this, mon, 185, 170), CommonTLayout);
    AddFrame(hf = new OXCompositeFrame(this, 1, 1, HORIZONTAL_FRAME),
                                                   CommonXLayout);

    hf->AddFrame(gf1 = new OXGroupFrame(hf, new OHotString("&Pattern")),
                                        CommonXLayout);

    hf->AddFrame(gf2 = new OXGroupFrame(hf, new OHotString("&Wall paper")),
                                        CommonXLayout);
}

OXBackgroundTab::~OXBackgroundTab() {
}
