/**************************************************************************

    This file is part of xclass.
    Copyright (C) 2004, Hector Peraza.                 

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

**************************************************************************/  

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <xclass/utils.h>
#include <xclass/OGifImage.h>
#include <xclass/OXRadioButton.h>
#include <xclass/OXTextEntry.h>
#include <xclass/OXHelpDoc.h>


//#define DEBUG_FORM

//----------------------------------------------------------------------

OXHelpDoc::OXHelpDoc(const OXWindow *p, int w, int h, int id) :
  OXHtml(p, w, h, id) {
    visitedUrl.clear();
    docName = 0;
    docNameStart = -1;
}

OXHelpDoc::~OXHelpDoc() {
  for (int i = 0; i < visitedUrl.size(); ++i) delete[] visitedUrl[i];
  visitedUrl.clear();
  if (docName) delete[] docName;
}


//----------------------------------------------------------------------

void OXHelpDoc::Clear() {

  OXHtml::Clear();

  if (docName) delete[] docName;
  docName = 0;
  docNameStart = -1;
}

int OXHelpDoc::IsVisited(const char *url) {
  int i;
  char *z = ResolveUri(url);

  if (!z) return False;

  for (i = 0; i < visitedUrl.size(); ++i) {
    if (strcmp(visitedUrl[i], z) == 0) {
      delete[] z;
      return True;
    }
  }

  delete[] z;
  return False;
}

void OXHelpDoc::AddToVisited(const char *url) {
  char *z = ResolveUri(url);
  if (!z) return;
  if (!IsVisited(z)) visitedUrl.push_back(StrDup(z));
  delete[] z;
}

int OXHelpDoc::ProcessToken(OHtmlElement *pElem, const char *name, int type) {
  if (type == Html_TITLE) {
    docNameStart = pElem->offs;
  } else if (type == Html_EndTITLE) {
    if (docNameStart > 0) {
      if (docName) delete[] docName;
      // StrNDup(...)
      int len = pElem->offs - docNameStart - strlen(name) - 2;
      docName = new char[len + 1];
      strncpy(docName, zText + docNameStart, len);
      docName[len] = '\0';
    }
  }
  return False;
}

int OXHelpDoc::FormAction(OHtmlForm *form, int id) {
  OString str("");

  if (!form) {
#ifdef DEBUG_FORM
    fprintf(stderr, "No form!\n");
#endif
    return False;
  } else {
    OHtmlInput *p;
#ifdef DEBUG_FORM
    printf("method = %s\n", form->MarkupArg("method", 0));
    printf("action = %s\n", form->MarkupArg("action", 0));
#endif
    for (p = firstInput; p; p = p->iNext) {
      if (p->pForm == form) {
        switch (p->itype) {
          case INPUT_TYPE_Hidden: {
            const char *name = p->MarkupArg("name", 0);
            const char *value = p->MarkupArg("value", "");
            if (name && *name) {
              if (str.GetLength() > 0) str.Append("&");
              EncodeText(&str, name);
              str.Append("=");
              EncodeText(&str, value);
            }
            break;
          }

          case INPUT_TYPE_Radio: {
            OXRadioButton *r = (OXRadioButton *) p->frame;
            if (r && r->GetState() == BUTTON_DOWN) {
              const char *name = p->MarkupArg("name", 0);
              const char *value = p->MarkupArg("value", "");
              if (name && *name) {
                if (str.GetLength() > 0) str.Append("&");
                EncodeText(&str, name);
                str.Append("=");
                EncodeText(&str, value);
              }
            }
            break;
          }

          case INPUT_TYPE_Submit:
            if (p->cnt == id) {
              const char *name = p->MarkupArg("name", 0);
              const char *value = p->MarkupArg("value", "");
              if (name && *name) {
                if (str.GetLength() > 0) str.Append("&");
                EncodeText(&str, name);
                str.Append("=");
                EncodeText(&str, value);
              }
            }
            break;

          case INPUT_TYPE_Text:
          case INPUT_TYPE_Password: {
            OXTextEntry *te = (OXTextEntry *) p->frame;
            if (te) {
              const char *name = p->MarkupArg("name", 0);
              const char *value = te->GetString();
              if (name && *name) {
                if (str.GetLength() > 0) str.Append("&");
                EncodeText(&str, name);
                str.Append("=");
                EncodeText(&str, value);
              }
            }
            break;
          }

          default:
            break;
        }
      }
    }
    const char *method = form->MarkupArg("method", "GET");
    if (strcasecmp(method, "GET") == 0) {
      str.Prepend("?");
      str.Prepend(form->MarkupArg("action", ""));
      char *uri = ResolveUri(str.GetString());
      if (uri) {
#ifdef DEBUG_FORM
        printf("Submiting form: %s\n", uri);
#endif
        OHtmlMessage msg(MSG_HTML, MSG_CLICK, _widgetID, uri, 0, 0);
        SendMessage(_msgObject, &msg);
        delete[] uri;
      }
    } else if (strcasecmp(method, "POST") == 0) {
#ifdef DEBUG_FORM
      printf("POST method not implemented\n");
#endif
    } else {
#ifdef DEBUG_FORM
      printf("Unknown form method: %s\n", method);
#endif
    }
  }
  return True;
}
