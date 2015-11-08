/**************************************************************************

    This file is part of xclass, a Win95-looking GUI toolkit.
    Copyright (C) 1996, 1997 David Barth, Hector Peraza.

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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>
#include <X11/Xlib.h>
#include <X11/xpm.h>

#include <xclass/utils.h>
#include <xclass/OXWindow.h>
#include <xclass/OPicture.h>
#include <xclass/OResourcePool.h>


struct OPicKey {
  const char *name;
  int scaled;
  int width;
  int height;
};

bool OPicturePool::_inited = false;
XpmColorSymbol OPicturePool::_colors[6];

//---------------------------------------------------------------------

OPicturePool::OPicturePool(const OXClient *client, const OResourcePool *rp,
                           const char *path) {
  _client = client;

  _path = path ? StrDup(path) : (char *) NULL;

  _defaultColormap = _client->GetDefaultColormap();
  _ntable = new OStringHashTable();
  _ptable = new OArrayHashTable(sizeof(OPicKey) / sizeof(int));

  if (!_inited) {
    _colors[0].name  = "DocumentBgndColor";
    _colors[0].value = NULL;
    _colors[0].pixel = rp->GetDocumentBgndColor();
    _colors[1].name  = "DocumentFgndColor";
    _colors[1].value = NULL;
    _colors[1].pixel = rp->GetDocumentFgndColor();
    _colors[2].name  = "FrameHiliteColor";
    _colors[2].value = NULL;
    _colors[2].pixel = rp->GetFrameHiliteColor();
    _colors[3].name  = "FrameShadowColor";
    _colors[3].value = NULL;
    _colors[3].pixel = rp->GetFrameShadowColor();
    _colors[4].name  = "FrameBgndColor";
    _colors[4].value = NULL;
    _colors[4].pixel = rp->GetFrameBgndColor();
    _colors[5].name  = "FrameFgndColor";
    _colors[5].value = NULL;
    _colors[5].pixel = rp->GetFrameFgndColor();
    _inited = true;
  }
}


OPicturePool::~OPicturePool() {
  OHashSearch search;
  OHashEntry *h;

  for (h = _ptable->FirstHashEntry(&search);
       h != NULL; h = _ptable->NextHashEntry(&search)) {
    OPicture *p = (OPicture *) h->GetHashValue();
    XFreePixmap(_client->GetDisplay(), p->_pic);
    if (p->_mask != None) XFreePixmap(_client->GetDisplay(), p->_mask);
    delete p;
    //_ptable->DeleteHashEntry(h); this will be done in OHashTable destructor.
  }

  if (_path) delete[] _path;

  delete _ntable;
  delete _ptable;
}


void OPicturePool::SetPath(const char *path) {
  if (_path) delete[] _path;
  _path = path ? StrDup(path) : (char *) NULL;
}


void OPicturePool::AddPath(const char *path) {
  if (!path || !*path) return;
  if (!_path || !*_path) { SetPath(path); return; }
  char *tmp = new char[strlen(_path) + strlen(path) + 2];
  sprintf(tmp, "%s:%s", _path, path);
  delete[] _path;
  _path = tmp;
}


void OPicturePool::FreePicture(const OPicture *pic) {
  OPicture *p = (OPicture *) pic;

  // remove the picture if nobody is using it anymore

  if (!p) return;

  if (--p->_count > 0) return;

  _ptable->DeleteHashEntry(p->_hash);
  XFreePixmap(_client->GetDisplay(), p->_pic);
  if (p->_mask != None) XFreePixmap(_client->GetDisplay(), p->_mask);
  XpmFreeAttributes(&(p->_attributes));
  delete p;

  return;
}

const OPicture *OPicturePool::GetPicture(const char *name, char **data) {
  OPicKey key;
  OHashEntry *h;
  OPicture *pic;
  int inew;
  const char *picName = GetUid(name);

  key.name = picName;
  key.scaled = 0;
  key.width = 0;
  key.height = 0;

  h = _ptable->Create((char *) &key, &inew);
  if (!inew) {
    pic = (OPicture *) h->GetHashValue();
    pic->_count++;
    return pic;
  }

  pic = new OPicture(picName);
  pic->_attributes.colorsymbols = _colors;
  pic->_attributes.numsymbols = 6;
  pic->_attributes.colormap  = _defaultColormap;
  pic->_attributes.closeness = 40000; // Allow for "similar" colors
  pic->_attributes.valuemask = XpmSize | XpmReturnPixels | XpmColormap |
                               XpmCloseness | XpmColorSymbols;
  
  if (XpmCreatePixmapFromData(_client->GetDisplay(),
                          (_client->GetRoot())->GetId(), data,
                          &(pic->_pic), &(pic->_mask),
                          &(pic->_attributes)) == XpmSuccess) { 
    pic->_count = 1;
    pic->_hash = h;
    h->SetHashValue(pic);
    return pic;
  } else {
    _ptable->DeleteHashEntry(h);
    delete pic;
    return NULL;
  }
}

const OPicture *OPicturePool::GetPicture(const char *name) {
  char tmp[MAXPATHLEN];
  OPicKey key;
  OHashEntry *h;
  OPicture *pic;
  int inew;
  const char *picName = GetUid(name);

  key.name = picName;
  key.scaled = 0;
  key.width = 0;
  key.height = 0;

  h = _ptable->Create((char *) &key, &inew);
  if (!inew) {
    pic = (OPicture *) h->GetHashValue();
    pic->_count++;
    return pic;
  }

  pic = new OPicture(picName);
  pic->_attributes.colorsymbols = _colors;
  pic->_attributes.numsymbols = 6;
  pic->_attributes.colormap  = _defaultColormap;
  pic->_attributes.closeness = 40000; // Allow for "similar" colors
  pic->_attributes.valuemask = XpmSize | XpmReturnPixels | XpmColormap |
                               XpmCloseness | XpmColorSymbols;
  
  if (!LocatePicture(picName, _path, tmp)) {
    // unable to locate picture
    _ptable->DeleteHashEntry(h);
    delete pic;
    return NULL;
  }

  if (XpmReadFileToPixmap(_client->GetDisplay(),
                          (_client->GetRoot())->GetId(), tmp,
			  &(pic->_pic), &(pic->_mask),
                          &(pic->_attributes)) == XpmSuccess) { 
    pic->_count = 1;
    pic->_hash = h;
    h->SetHashValue(pic);
    return pic;
  } else {
    _ptable->DeleteHashEntry(h);
    delete pic;
    return NULL;
  }
}

const OPicture *OPicturePool::GetPicture(const char *name, char **data,
                                         int new_width, int new_height) {
  OPicKey key;
  OHashEntry *h;
  OPicture *pic;
  char **smalldata;
  double x, y, xscale, yscale;
  int    x1, y1;
  int    retc, i, height, width, colors, chars, inew;
  int    totalheight;
  int    headersize;
  int    pixels;
  const  char *picName = GetUid(name);

  key.name = picName;
  key.scaled = 1;
  key.width = new_width;
  key.height = new_height;

  h = _ptable->Create((char *) &key, &inew);
  if (!inew) {
    pic = (OPicture *) h->GetHashValue();
    pic->_count++;
    return pic;
  }

  pic = new OPicture(picName, True);
  pic->_attributes.colorsymbols = _colors;
  pic->_attributes.numsymbols = 6;
  pic->_attributes.colormap  = _defaultColormap;
  pic->_attributes.closeness = 40000; // Allow for "similar" colors
  pic->_attributes.valuemask = XpmSize | XpmReturnPixels | XpmColormap |
                               XpmCloseness | XpmColorSymbols;

  sscanf(data[0], "%d %d %d %d", &width, &height, &colors, &chars);
  headersize = colors + 1;
  yscale = (double) new_height / (double) height;
  xscale = (double) new_width / (double) width;
  totalheight = (colors + new_height + 1);

  if ((width != new_width) || (height != new_height)) {

    smalldata = new char* [totalheight + 1];
    if (!smalldata) FatalError("Not enough memory.");

    smalldata[0] = new char[30];
    for (i = 1; i < headersize; i++){
      smalldata[i] = new char [strlen(data[i]) + 1];
      if (!smalldata[i])
        FatalError("Not enough memory.");
    }
    for (i = headersize; i < totalheight + 1; i++) {
      smalldata[i] = new char[(new_width * chars) + 1];
      if (!smalldata[i])
        FatalError("Not enough memory.");
    }

    sprintf(smalldata[0], "%d %d %d %d", new_width, new_height, colors, chars);

    for (i = 1; i < headersize; i++) strcpy(smalldata[i], data[i]);

    y = headersize;
    for (y1 = headersize; y1 < new_height + headersize; y1++) {
      x = 0;
      for (x1 = 0; x1 < new_width; x1++) {
        for (pixels = 0; pixels < chars; pixels++)
          smalldata[y1][x1+pixels] = data[(int)y][(int)x + pixels];
        x += 1.0 / xscale;
      }
      smalldata[y1][x1] = '\0';
      y += 1.0 / yscale;
    }

    retc = XpmCreatePixmapFromData(_client->GetDisplay(),
                            (_client->GetRoot())->GetId(), smalldata,
                           &(pic->_pic), &(pic->_mask),
                           &(pic->_attributes));

    for (i = 0; i < totalheight + 1; i++) delete[] smalldata[i];
    delete[] smalldata;

  } else {

    retc = XpmCreatePixmapFromData(_client->GetDisplay(),
                            (_client->GetRoot())->GetId(), data,
                           &(pic->_pic), &(pic->_mask),
                           &(pic->_attributes));
  }

  XpmFree(data);

  if (retc == XpmSuccess) {
    pic->_count = 1;
    pic->_hash = h;
    h->SetHashValue(pic);
    return pic;
  } else {
    _ptable->DeleteHashEntry(h);
    delete pic;
    return NULL;
  }
}

const OPicture *OPicturePool::GetPicture(const char *name,
                                         int new_width, int new_height) {
  char tmp[MAXPATHLEN];
  OPicKey key;
  OHashEntry *h;
  OPicture *pic;
  char **data;
  char **smalldata;
  double x, y, xscale, yscale;
  int    x1, y1;
  int    retc, i, height, width, colors, chars, inew;
  int    totalheight;
  int    headersize;
  int    pixels;
  const  char *picName = GetUid(name);

  key.name = picName;
  key.scaled = 1;
  key.width = new_width;
  key.height = new_height;

  h = _ptable->Create((char *) &key, &inew);
  if (!inew) {
    pic = (OPicture *) h->GetHashValue();
    pic->_count++;
    return pic;
  }

  pic = new OPicture(picName, True);
  pic->_attributes.colorsymbols = _colors;
  pic->_attributes.numsymbols = 6;
  pic->_attributes.colormap  = _defaultColormap;
  pic->_attributes.closeness = 40000; // Allow for "similar" colors
  pic->_attributes.valuemask = XpmSize | XpmReturnPixels | XpmColormap |
                               XpmCloseness | XpmColorSymbols;

  if (!LocatePicture(picName, _path, tmp)) {
    // unable to locate picture   
    _ptable->DeleteHashEntry(h);
    delete pic;
    return NULL;
  } 

  if (XpmReadFileToData(tmp, &data) != XpmSuccess) {
    _ptable->DeleteHashEntry(h);
    delete pic;
    return NULL;
  }

  sscanf(data[0], "%d %d %d %d", &width, &height, &colors, &chars);
  headersize = colors + 1;
  yscale = (double) new_height / (double) height;
  xscale = (double) new_width / (double) width;
  totalheight = (colors + new_height + 1);

  if ((width != new_width) || (height != new_height)) {

    smalldata = new char* [totalheight + 1];
    if (!smalldata) FatalError("Not enough memory.");

    smalldata[0] = new char[30];
    for (i = 1; i < headersize; i++){
      smalldata[i] = new char [strlen(data[i]) + 1];
      if (!smalldata[i])
        FatalError("Not enough memory.");
    }
    for (i = headersize; i < totalheight + 1; i++) {
      smalldata[i] = new char[(new_width * chars) + 1];
      if (!smalldata[i])
        FatalError("Not enough memory.");
    }

    sprintf(smalldata[0], "%d %d %d %d", new_width, new_height, colors, chars);

    for (i = 1; i < headersize; i++) strcpy(smalldata[i], data[i]);

    y = headersize;
    for (y1 = headersize; y1 < new_height + headersize; y1++) {
      x = 0;
      for (x1 = 0; x1 < new_width; x1++) {
        for (pixels = 0; pixels < chars; pixels++)
          smalldata[y1][x1+pixels] = data[(int)y][(int)x + pixels];
        x += 1.0 / xscale;
      }
      smalldata[y1][x1] = '\0';
      y += 1.0 / yscale;
    }

    retc = XpmCreatePixmapFromData(_client->GetDisplay(),
                            (_client->GetRoot())->GetId(), smalldata,
                           &(pic->_pic), &(pic->_mask),
                           &(pic->_attributes));

    for (i = 0; i < totalheight + 1; i++) delete[] smalldata[i];
    delete[] smalldata;

  } else {

    retc = XpmCreatePixmapFromData(_client->GetDisplay(),
                            (_client->GetRoot())->GetId(), data,
                           &(pic->_pic), &(pic->_mask),
                           &(pic->_attributes));
  }

  XpmFree(data);

  if (retc == XpmSuccess) {
    pic->_count = 1;
    pic->_hash = h;
    h->SetHashValue(pic);
    return pic;
  } else {
    _ptable->DeleteHashEntry(h);
    delete pic;
    return NULL;
  }
}

const OPicture *OPicturePool::GetPicture(const OPicture *pic) {
  if (pic) {
    OPicture *p = (OPicture *) pic;
    ++p->_count;
    return p;
  }
  return NULL;
}

int OPicturePool::LocatePicture(const char *name, char *path, char *bfr) {
  char *colon_pos;

  // if name contains path
  if (strchr(name, '/' )) {
    // consider it as absolute file referece
    strcpy(bfr, name);
    if (access(bfr, R_OK) == 0) {
      // file found
      return 1;
    }
  }

  // if no path given, use default
  if (!path) {
    if ((path = getenv("OX_POOL")) == NULL) {
      path = OX_DEFAULT_POOL;
    }
  }

  while ((colon_pos = strchr(path, ':'))) {
    // colon found
    if (colon_pos != path) {
      memcpy(bfr, path, colon_pos-path);
      sprintf(&bfr[colon_pos-path], "/%s", name);
      if (access(bfr, R_OK) == 0) {
        // file found
        return 1;
      }
      sprintf(&bfr[colon_pos-path], "/%s.gz", name);
      if (access(bfr, R_OK) == 0 ) {
        // file found
        return 1;
      } 
    } // else empty path
    path = colon_pos+1;
  }

  if (path[0]) {
    // non empty rest
    sprintf(bfr, "%s/%s", path, name);
    if (access(bfr, R_OK) == 0) {
      // file found
      return 1;
    } 
    sprintf(bfr, "%s/%s.gz", path, name);
    if (access(bfr, R_OK ) == 0) {
      // file found
      return 1;
    } 
  }

  bfr[0] = '\0';
  return 0;
}

const char *OPicturePool::GetUid(const char *string) {
  int dummy;

  OHashEntry *h = _ntable->Create(string, &dummy);
  return (const char *) h->GetHashKey();
}


//---------------------------------------------------------------------

void OPicture::Draw(Display *dpy, XID id, GC gc, int x, int y) const {
  XGCValues gcv;  

  unsigned long gcm = GCClipMask | GCClipXOrigin | GCClipYOrigin;
  gcv.clip_mask = _mask;
  gcv.clip_x_origin = x;
  gcv.clip_y_origin = y;
  XChangeGC(dpy, gc, gcm, &gcv);
  XCopyArea(dpy, _pic, id, gc, 0, 0,
                 _attributes.width, _attributes.height, x, y);
  gcm = GCClipMask;
  gcv.clip_mask = None;
  XChangeGC(dpy, gc, gcm, &gcv);
}

void OPicture::DrawMask(Display *dpy, XID id, GC gc, int x, int y) const {
  XGCValues gcv;  

  unsigned long gcm = GCClipMask | GCClipXOrigin | GCClipYOrigin;
  gcv.clip_mask = _mask;
  gcv.clip_x_origin = x;
  gcv.clip_y_origin = y;
  XChangeGC(dpy, gc, gcm, &gcv);
  if (_mask) {
    XCopyPlane(dpy, _mask, id, gc, 0, 0,
                    _attributes.width, _attributes.height, x, y, 1);
  } else {
    XFillRectangle(dpy, id, gc, x, y, _attributes.width, _attributes.height);
  }
  gcm = GCClipMask;
  gcv.clip_mask = None;
  XChangeGC(dpy, gc, gcm, &gcv);
}
