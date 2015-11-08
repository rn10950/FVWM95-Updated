/**************************************************************************

    HTML widget for xclass. Based on tkhtml 1.28
    Copyright (C) 1997-2000 D. Richard Hipp <drh@acm.org>
    Copyright (C) 2002-2003 Hector Peraza.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

**************************************************************************/

#include <string.h>

#include <xclass/OXHtml.h>

// TODO: make these OXHtml static members

extern void HtmlTranslateEscapes(char *z);
extern void ToLower(char *z);

//----------------------------------------------------------------------

OHtmlElement::OHtmlElement(int etype) {
  pNext = pPrev = 0;
  style.font = 0;   
  style.color = 0;  
  style.subscript = 0;
  style.align = 0;
  style.bgcolor = 0;
  style.expbg = 0;
  style.flags = 0;  
  type = etype;     
  flags = 0;        
  count = 0;
  id = 0;
  offs = 0;
}

OHtmlTextElement::OHtmlTextElement(int size) :
  OHtmlElement(Html_Text) {
    zText = new char[size + 1];
    x = 0; y = 0; w = 0;
    ascent = 0;
    descent = 0;
    spaceWidth = 0;
}

OHtmlTextElement::~OHtmlTextElement() {
  delete[] zText;
}


//----------------------------------------------------------------------

OHtmlMarkupElement::OHtmlMarkupElement(int type, int argc,
                                       int arglen[], char *av[]) :
  OHtmlElement(type) {

    count = argc - 1;

    if (argc > 1) {
      argv = new char*[argc+1];
      for (int i = 1; i < argc; i++) {
        if (arglen) {
          argv[i-1] = new char[arglen[i]+1];
          //sprintf(argv[i-1], "%.*s", arglen[i], av[i]);
          strncpy(argv[i-1], av[i], arglen[i]);
          argv[i-1][arglen[i]] = 0;
          HtmlTranslateEscapes(argv[i-1]);
          if ((i & 1) == 1) ToLower(argv[i-1]);
        } else {
          argv[i-1] = StrDup(av[i]);
          HtmlTranslateEscapes(argv[i-1]);
          if ((i & 1) == 1) ToLower(argv[i-1]);
        }
      }  
      argv[argc-1] = 0;

      // Following is just a flag that this is unmodified
      argv[argc] = (char *) argv;

    } else {
      argv = 0;
    }

}

OHtmlMarkupElement::~OHtmlMarkupElement() {
  if (argv) {
    for (int i = 0; i < count; ++i) delete[] argv[i];
    delete[] argv;
  }
}  
   
// Lookup an argument in the given markup with the name given.
// Return a pointer to its value, or the given default  
// value if it doesn't appear.

char *OHtmlMarkupElement::MarkupArg(const char *tag, char *zDefault) {
  int i;

  for (i = 0; i < count; i += 2) {
    if (strcmp(argv[i], tag) == 0) return argv[i+1];
  }
  return zDefault; 
}

// Return an alignment or justification flag associated with the
// given markup. The given default value is returned if no alignment is
// specified.

int OHtmlMarkupElement::GetAlignment(int dflt) {
  char *z = MarkupArg("align", 0);
  int rc = dflt;

  if (z) {
    if (strcasecmp(z, "left") == 0) {
      rc = ALIGN_Left;
    } else if (strcasecmp(z, "right") == 0) {
      rc = ALIGN_Right;
    } else if (strcasecmp(z, "center") == 0) {
      rc = ALIGN_Center;
    }
  }  
     
  return rc;
}


// The "type" argument to the given element might describe the type
// for an ordered list. Return the corresponding LI_TYPE_* entry  
// if this is the case, or the default value if it isn't.

// (this and the following should be defined only for OXHtmlLi)

int OHtmlMarkupElement::GetOrderedListType(int dflt) {
  char *z;

  z = MarkupArg("type", 0);
  if (z) {
    switch (*z) {
      case 'A': dflt = LI_TYPE_Enum_A; break;
      case 'a': dflt = LI_TYPE_Enum_a; break;
      case '1': dflt = LI_TYPE_Enum_1; break;
      case 'I': dflt = LI_TYPE_Enum_I; break;
      case 'i': dflt = LI_TYPE_Enum_i; break;
      default:  break;
    }
  }  
     
  return dflt;
}

// The "type" argument to the given element might describe a type
// for an unordered list.  Return the corresponding LI_TYPE entry
// if this is the case, or the default value if it isn't.

int OHtmlMarkupElement::GetUnorderedListType(int dflt) {
  char *z;

  z = MarkupArg("type", 0);
  if (z) {
    if (strcasecmp(z, "disc") == 0) {
      dflt = LI_TYPE_Bullet1;
    } else if (strcasecmp(z, "circle") == 0) {
      dflt = LI_TYPE_Bullet2;
    } else if (strcasecmp(z, "square") == 0) {
      dflt = LI_TYPE_Bullet3;
    }
  }  
     
  return dflt;
}

//int OXHtmlMarkupElement::GetVerticalAlignment(int dflt);


//----------------------------------------------------------------------

OHtmlTable::OHtmlTable(int type, int argc, int arglen[], char *argv[]) :
  OHtmlMarkupElement(type, argc, arglen, argv) {
    borderWidth = 0;
    nCol = 0;
    nRow = 0;
    x = 0; y = 0; w = 0; h = 0;
    pEnd = 0;
    bgImage = 0;
    hasbg = 0;
}

OHtmlTable::~OHtmlTable() {
  if (bgImage) delete bgImage;
}
   
OHtmlCell::OHtmlCell(int type, int argc, int arglen[], char *argv[]) :
  OHtmlMarkupElement(type, argc, arglen, argv) {
    rowspan = 0;
    colspan = 0;
    x = 0; y = 0; w = 0; h = 0;
    pTable = 0;
    pRow = 0;
    pEnd = 0;
    bgImage = 0;  
}

OHtmlCell::~OHtmlCell() {
  if (bgImage) delete bgImage;
}
   
OHtmlRef::OHtmlRef(int type, int argc, int arglen[], char *argv[]) :
  OHtmlMarkupElement(type, argc, arglen, argv) {
    pOther = 0;
    bgImage = 0;
}

OHtmlRef::~OHtmlRef() {
  if (bgImage) delete bgImage;
}
   
OHtmlLi::OHtmlLi(int type, int argc, int arglen[], char *argv[]) :
  OHtmlMarkupElement(type, argc, arglen, argv) {
    ltype = 0;
    ascent = 0;
    descent = 0;
    cnt = 0;
    x = 0; y = 0;
}

OHtmlListStart::OHtmlListStart(int type, int argc,
                               int arglen[], char *argv[]) :
  OHtmlMarkupElement(type, argc, arglen, argv) {
    ltype = 0;
    compact = 0;
    cnt = 0;
    width = 0;
    lPrev = 0;
}
   
OHtmlImageMarkup::OHtmlImageMarkup(int type, int argc,
                                   int arglen[], char *argv[]) :
  OHtmlMarkupElement(type, argc, arglen, argv) {
    align = 0;
    textAscent = 0;
    textDescent = 0;
    redrawNeeded = 0;
    x = 0; y = 0; w = 0; h = 0;
    ascent = 0;
    descent = 0;
    zAlt = 0;   
    pImage = 0;
    pMap = 0; 
    iNext = 0;  
}
   
OHtmlForm::OHtmlForm(int type, int argc, int arglen[], char *argv[]) :
  OHtmlMarkupElement(type, argc, arglen, argv) {
    formId = 0;
    elements = 0;
    hasctl = 0;
    pFirst = 0;
    pEnd = 0;
}

OHtmlHr::OHtmlHr(int type, int argc, int arglen[], char *argv[]) :
  OHtmlMarkupElement(type, argc, arglen, argv) {
    x = 0; y = 0; w = 0; h = 0;
    is3D = 0;
}
   
OHtmlAnchor::OHtmlAnchor(int type, int argc, int arglen[], char *argv[]) :
  OHtmlMarkupElement(type, argc, arglen, argv) {
    y = 0;
}
   
OHtmlScript::OHtmlScript(int type, int argc, int arglen[], char *argv[]) :
  OHtmlMarkupElement(type, argc, arglen, argv) {
    nStart = -1;
    nScript = 0;
}
   
OHtmlMapArea::OHtmlMapArea(int type, int argc, int arglen[], char *argv[]) :
  OHtmlMarkupElement(type, argc, arglen, argv) {
    mType = 0;
    coords = 0;
    num = 0;
}
   

//----------------------------------------------------------------------

#if 0
OHtmlBlock::OHtmlBlock() {
}

OHtmlBlock::~OHtmlBlock() {
}
#endif


//----------------------------------------------------------------------

OHtmlInput::OHtmlInput(int type, int argc, int arglen[], char *argv[]) :
  OHtmlMarkupElement(type, argc, arglen, argv) {
    pForm = 0;
    iNext = 0;
    frame = 0;
    html = 0; 
    pEnd = 0;
    inpId = 0; subId = 0;
    x = 0; y = 0; w = 0; h = 0;
    padLeft = 0;
    align = 0;  
    textAscent = 0;
    textDescent = 0;
    itype = 0;
    sized = 0;
    cnt = 0;  
}
   
// Mark this element as being empty. It has no widget and doesn't appear on
// the screen.
//
// This is called for HIDDEN inputs or when the corresponding widget is
// not created.

void OHtmlInput::Empty() {
  frame = NULL;
  w = 0;
  h = 0;
  flags &= ~HTML_Visible;
  style.flags |= STY_Invisible;
  sized = 1;
}
