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

#include <X11/X.h>
#include <X11/Xlib.h>

#include <xclass/utils.h>
#include <xclass/OXWindow.h>
#include <xclass/OGC.h>


//---------------------------------------------------------------------

OGCPool::OGCPool(const OXClient *client) {
  _client = client;
  _gclist = NULL;
}

OGCPool::~OGCPool() {
  SListGCElt *ptr, *next;

  ptr = _gclist;
  while (ptr) {
    next = ptr->next;
    delete ptr->gc;
    delete ptr;
    ptr = next;
  }
}

// ReleaseGC

void OGCPool::FreeGC(const OXGC *gc) {
  SListGCElt *ptr, *ptr2;

  // free the gc if it is not being used anymore

  if (_gclist == NULL) return;

  if (_gclist->gc == gc) {
    ptr = _gclist;
    ptr->count--;
    if (ptr->count > 0) return;
    _gclist = _gclist->next;
    delete ptr->gc;  // this frees the GC...
    delete ptr;
  } else {
    for (ptr = _gclist;
         ptr->next != NULL && ptr->next->gc != gc;
         ptr = ptr->next);
    if (ptr->next == NULL) return; // not found!
    ptr2 = ptr->next;
    ptr2->count--;
    if (ptr2->count > 0) return;
    ptr->next = ptr2->next;
    delete ptr2->gc;
    delete ptr2;
  }

  return;
}


void OGCPool::FreeGC(const GC gc) {
  SListGCElt *ptr, *ptr2;

  // free the gc if it is not being used anymore

  if (_gclist == NULL) return;

  if (_gclist->gc->GetGC() == gc) {
    ptr = _gclist;
    ptr->count--;
    if (ptr->count > 0) return;
    _gclist = _gclist->next;
    delete ptr->gc;  // this frees the GC...
    delete ptr;
  } else {
    for (ptr = _gclist;
         ptr->next != NULL && ptr->next->gc->GetGC() != gc;
         ptr = ptr->next);
    if (ptr->next == NULL) return; // not found!
    ptr2 = ptr->next;
    ptr2->count--;
    if (ptr2->count > 0) return;
    ptr->next = ptr2->next;
    delete ptr2->gc;
    delete ptr2;
  }

  return;
}


const OXGC *OGCPool::GetGC(Drawable d, unsigned long mask, XGCValues *values,
                           int rw) {
  SListGCElt *ptr, *nw, *best_match = NULL;
  int matching_bits, exact = False;
  int best_matching_bits = -1;

  if (!rw) {

    // First, try to find an exact matching GC.
    // If no one found, then use the closest one

    for (ptr = _gclist; ptr != NULL; ptr = ptr->next) {
      if (ptr->rwflag) continue;
      matching_bits = MatchGC(ptr->gc, mask, values);
      if (matching_bits > best_matching_bits) {
        best_matching_bits = matching_bits;
        best_match = ptr;
        if ((ptr->gc->_mask & mask) == mask) {
          exact = True;
          break;
        }
      }
    }

    if (best_match) {
      Debug(DBG_MISC, "GC pool: %smatching GC found\n", exact ? "exact " : ""); 
      best_match->count++;  // increase the usage counter...
      if (!exact) {
        // add missing values to the best_match'ing GC...
        UpdateGC(best_match->gc, mask, values);
      }
      return best_match->gc;
    }
  }

  OXGC *gc = new OXGC(_client->GetDisplay(), d, mask, values);

  nw = new SListGCElt;
  nw->next   = NULL;
  nw->gc     = gc;
  nw->count  = 1;
  nw->rwflag = rw;
  if (_gclist == NULL) {
    _gclist = nw;
  } else {
    for (ptr = _gclist; ptr->next != NULL; ptr = ptr->next);
    ptr->next = nw;
  }

  return gc;
}


// Check the drawable too, they should have the same root!

// On success returns the amount of matching bits (which may be zero if
// masks have no common bits), -1 on failure.

int OGCPool::MatchGC(const OXGC *gc, unsigned long mask, XGCValues *values) {
  unsigned long bit, common_bits;
  int match, matching_bits = 0;
  const XGCValues *gcv = &gc->_values;

  common_bits = mask & gc->_mask;

  if (common_bits == 0) return 0;  // no common bits, a possible
                                   // candidate anyway.

  // Careful, check first the tile and stipple mask bits, as these
  // influence nearly all other GC functions... (do the same for
  // some other such bits as GCFunction, etc...) Perhaps we should
  // allow only exact GC matches.

  if (gc->_mask & GCTile)
    if ((gcv->tile != None) && !(mask & GCTile)) return -1; // GC not good.
  if (mask & GCTile)
    if ((values->tile != None) && !(gc->_mask & GCTile)) return -1;
  if (gc->_mask & GCStipple)
    if ((gcv->stipple != None) && !(mask & GCStipple)) return -1; // ditto.
  if (mask & GCStipple)
    if ((values->stipple != None) && !(gc->_mask & GCStipple)) return -1;

  // We have a GC with tile and/or stipple, and we requested one of
  // these, the loop below will check whether they match.

  for (bit = 1L; bit <= common_bits /*bit != (1<<GCLastBit)*/; bit <<= 1) {
    switch (bit & common_bits) {
      default:
      case 0:
        continue;
        break;
      case GCFunction:
        match = (values->function == gcv->function);
        break;
      case GCPlaneMask:
        match = (values->plane_mask == gcv->plane_mask);
        break;
      case GCForeground:
        match = (values->foreground == gcv->foreground);
        break;
      case GCBackground:
        match = (values->background == gcv->background);
        break;
      case GCLineWidth:
        match = (values->line_width == gcv->line_width);
        break;
      case GCLineStyle:
        match = (values->line_style == gcv->line_style);
        break;
      case GCCapStyle:
        match = (values->cap_style == gcv->cap_style);
        break;
      case GCJoinStyle:
        match = (values->join_style == gcv->join_style);
        break;
      case GCFillStyle:
        match = (values->fill_style == gcv->fill_style);
        break;
      case GCFillRule:
        match = (values->fill_rule == gcv->fill_rule);
        break;
      case GCTile:
        match = (values->tile == gcv->tile);
        break;
      case GCStipple:
        match = (values->stipple == gcv->stipple);
        break;
      case GCTileStipXOrigin:
        match = (values->ts_x_origin == gcv->ts_x_origin);
        break;
      case GCTileStipYOrigin:
        match = (values->ts_y_origin == gcv->ts_y_origin);
        break;
      case GCFont:
        match = (values->font == gcv->font);
        break;
      case GCSubwindowMode:
        match = (values->subwindow_mode == gcv->subwindow_mode);
        break;
      case GCGraphicsExposures:
        match = (values->graphics_exposures == gcv->graphics_exposures);
        break;
      case GCClipXOrigin:
        match = (values->clip_x_origin == gcv->clip_x_origin);
        break;
      case GCClipYOrigin:
        match = (values->clip_y_origin == gcv->clip_y_origin);
        break;
      case GCClipMask:
        match = (values->clip_mask == gcv->clip_mask);
        break;
      case GCDashOffset:
        match = (values->dash_offset == gcv->dash_offset);
        break;
      //case GCDashList:
      //  match = !strcmp(values->dashes, gcv->dashes);
      //  break;
      case GCArcMode:
        match = (values->arc_mode == gcv->arc_mode);
        break;
    }
    if (!match) return -1;
    ++matching_bits;
  }

  return matching_bits;
}

void OGCPool::UpdateGC(OXGC *gc, unsigned long mask, XGCValues *values) {
  unsigned long bit, diff_mask;
  XGCValues *gcv = &gc->_values;
  int do_dashes = False;

  diff_mask = mask & ~gc->_mask;

  for (bit = 1L; bit <= diff_mask /*bit != (1<<GCLastBit)*/; bit <<= 1) {
    switch (bit & diff_mask) {
      default:
      case 0:
        continue;
        break;
      case GCFunction:
        gcv->function = values->function;
        break;
      case GCPlaneMask:
        gcv->plane_mask = values->plane_mask;
        break;
      case GCForeground:
        gcv->foreground = values->foreground;
        break;
      case GCBackground:
        gcv->background = values->background;
        break;
      case GCLineWidth:
        gcv->line_width = values->line_width;
        break;
      case GCLineStyle:
        gcv->line_style = values->line_style;
        break;
      case GCCapStyle:
        gcv->cap_style = values->cap_style;
        break;
      case GCJoinStyle:
        gcv->join_style = values->join_style;
        break;
      case GCFillStyle:
        gcv->fill_style = values->fill_style;
        break;
      case GCFillRule:
        gcv->fill_rule = values->fill_rule;
        break;
      case GCTile:
        gcv->tile = values->tile;
        break;
      case GCStipple:
        gcv->stipple = values->stipple;
        break;
      case GCTileStipXOrigin:
        gcv->ts_x_origin = values->ts_x_origin;
        break;
      case GCTileStipYOrigin:
        gcv->ts_y_origin = values->ts_y_origin;
        break;
      case GCFont:
        gcv->font = values->font;
        break;
      case GCSubwindowMode:
        gcv->subwindow_mode = values->subwindow_mode;
        break;
      case GCGraphicsExposures:
        gcv->graphics_exposures = values->graphics_exposures;
        break;
      case GCClipXOrigin:
        gcv->clip_x_origin = values->clip_x_origin;
        break;
      case GCClipYOrigin:
        gcv->clip_y_origin = values->clip_y_origin;
        break;
      case GCClipMask:
        gcv->clip_mask = values->clip_mask;
        break;
      case GCDashOffset:
        gcv->dash_offset = values->dash_offset;
        break;
      case GCDashList:
        do_dashes = True;
        break;
      case GCArcMode:
        gcv->arc_mode = values->arc_mode;
        break;
    }
  }

  XChangeGC(_client->GetDisplay(), gc->_gc, diff_mask, gcv);
  //if (do_dashes)
  //  XSetDashes(_client->GetDisplay(), gc->_gc, values->dash_offset,
  //             values->dashes, n);

  gc->_mask |= mask;

  return;
}
