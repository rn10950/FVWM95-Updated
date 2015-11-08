#include <xclass/OXFrame.h>
#include "OXViewDoc.h"

void OViewDoc::SetCanvas(OXViewDocFrame *c) {
  _canvas = c;
}

void OViewDoc::CreateCanvas(OXViewDoc *p) {
  _canvas = new OXViewDocFrame(p, 1, 1, CHILD_FRAME, p->GetBackground());
  _canvas->SetDocument(this);
}
