#include <stdlib.h>
#include <stdio.h>

#include "main.h"
#include "scrnsav_tab.h"

#include "es.xpm"

#include "sslist.h"

//----------------------------------------------------------------------

OXScreenSaversTab::OXScreenSaversTab(const OXWindow *tab, const OPicture *mon) :
  OXCompositeFrame(tab, 100, 100, VERTICAL_FRAME) {
    OXCompositeFrame *gf1, *gf2, *hf1, *vf1;
    int i;

    //Energy_Star = _client->GetPicture("es.xpm", es);
    //if (!Energy_Star) FatalError("Pixmap not found: es.xpm");

    //Monitor = _client->GetPicture("screen.xpm", ecran);
    //if (!Monitor) FatalError("Pixmap not found: screen.xpm");

    CommonTLayout = new OLayoutHints(LHINTS_TOP | LHINTS_CENTER_X,
                                     2, 2, 2, 2);
    CommonXLayout = new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y,
                                     2, 2, 2, 2);
    CommonLLayout = new OLayoutHints(LHINTS_LEFT | LHINTS_CENTER_Y,
                                     2, 2, 2, 2);

    AddFrame(new OXIcon(this, mon, 185, 170), CommonTLayout);
    AddFrame(gf1 = new OXGroupFrame(this, new OHotString("&Screen saver")),
                                    CommonXLayout);

    gfly = new OLayoutHints(LHINTS_NORMAL, 0, 0, 7, 0);
    gf1->AddFrame(lbf = new OXDDListBox(gf1, 1001), gfly);

    for (i = 0; i < num_ss-1; i++)
      lbf->AddEntry(new OString(ScreenSavers[i]), 1001);

    AddFrame(gf2 = new OXGroupFrame(this,
                   new OHotString("&Energy saving features of monitor")),
                   CommonXLayout);

    gf2->AddFrame(hf1 = new OXCompositeFrame(gf2, 1, 1, HORIZONTAL_FRAME),
                                             CommonXLayout);
    hf1->AddFrame(vf1 = new OXCompositeFrame(hf1, 1, 1, VERTICAL_FRAME),
                                             CommonXLayout);
    vf1->AddFrame(new OXCheckButton(vf1,
                      new OHotString("&Low-power standby"), 0),
                      CommonLLayout);
    vf1->AddFrame(new OXCheckButton(vf1,
                      new OHotString("Sh&ut off monitor"), 0),
                      CommonLLayout);
}

OXScreenSaversTab::~OXScreenSaversTab() {
}
