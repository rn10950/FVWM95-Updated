#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>

#include <xclass/OPicture.h>
#include <xclass/OXFont.h>
#include <xclass/utils.h>

#include "OColorTable.h"
#include "OXPaintCanvas.h"

#include "weave.xbm"

#define DefaultGridTolerance 5
#define DefaultPixmapWidth   640        // 32
#define DefaultPixmapHeight  480        // 32
#define DefaultStippled      True
#define DefaultGrid          True
#define DefaultDistance      1;         // 10
#define DefaultSquareSize    1;         // 20
#define DefaultTransparent   "#808080"  //"gray90"


//Atom targets[] = {
//  XA_BITMAP,
//  XA_PIXMAP,
//  XA_STRING,
//};

Atom OXPaintCanvas::_XC_SELECTION_DATA = None;

extern char *rgb_fname;

//-------------------------------------------------------------------------

static Pixel WhitePixelOfColormap(Display *dpy, Colormap cmap) {
  XColor color;

  color.red   = 0xffff;
  color.blue  = 0xffff;
  color.green = 0xffff;
  XAllocColor(dpy, cmap, &color);
  XFreeColors(dpy, cmap, &color.pixel, 1, 0);

  return color.pixel;
}


//-------------------------------------------------------------------------

OXPaintCanvas::OXPaintCanvas(const OXWindow *p, int w, int h, 
                             unsigned int options, unsigned long back) :
    OXFrame(p, w, h, options | OWN_BKGND, _whitePixel) {

  XGCValues gcval;
  unsigned  long gcmask;
  Display   *dpy;

  dpy = GetDisplay();

  if (_XC_SELECTION_DATA == None)
    _XC_SELECTION_DATA = XInternAtom(dpy, "_XC_SELECTION_DATA", False);

  _colormap = _client->GetDefaultColormap();

  _screen = _client->GetScreenNumber();
  _depth  = _client->GetDisplayDepth();

  _drawCursor       = XCreateFontCursor(GetDisplay(), XC_pencil);
  _selCursor        = XCreateFontCursor(GetDisplay(), XC_tcross);
  _moveCursor       = XCreateFontCursor(GetDisplay(), XC_fleur);
  _fgndPixel        = _blackPixel;
  _hilitePixel      = _blackPixel;
  _framingPixel     = _blackPixel;
  _transparentPixel = _client->GetColorByName(DefaultTransparent);
  _grid             = DefaultGrid;
  _gridTolerance    = DefaultGridTolerance;
  _stippled         = DefaultStippled;
  _distance         = DefaultDistance;
  _squareSize       = DefaultSquareSize;
  _width            = DefaultPixmapWidth;
  _height           = DefaultPixmapHeight;
  _filename         = NULL;
#if 0
  _stipple          = None;
#else
  _stipple = XCreatePixmapFromBitmapData(GetDisplay(), _id,
                    (char *) weave_bits, weave_width, weave_height, 1, 0, 1);
#endif

  ChangeFilename("Untitled.xpm");

  _colorTable = new OColorTable(dpy, _screen, _depth, _colormap);

  _drawMode = _lastMode = DRAW_POINT;

  _fold     = False;
  _changed  = False;

  _selection.own   = False;
  _selection.limbo = False;

  _clearPixel = WhitePixelOfColormap(GetDisplay(), _colormap);

  _hints_cmt  = NULL;
  _colors_cmt = NULL;
  _pixels_cmt = NULL;

  _extensions  = NULL;
  _nextensions = 0;
  _buffer_extensions  = NULL;
  _buffer_nextensions = 0;

  _text_string = NULL;
  _font_struct = NULL;

  SetFont(_client->GetFont("fixed"));
  SetText("Text");

  _hot.x = _hot.y = NOT_SET;
  _buffer_hot.x = _buffer_hot.y = NOT_SET;

  _mark.from_x = _mark.from_y = NOT_SET;
  _mark.to_x   = _mark.to_y   = NOT_SET;

  _buffer_mark.from_x = _buffer_mark.from_y = NOT_SET;
  _buffer_mark.to_x   = _buffer_mark.to_y   = NOT_SET;

  gcval.foreground = _fgndPixel;
  gcval.background = back; //core.background_pixel;
  gcval.foreground ^= gcval.background;
  gcval.function   = GXcopy;
  gcval.plane_mask = AllPlanes;
  gcmask = GCForeground | GCBackground | GCFunction | GCPlaneMask;
  _drawingGC = XCreateGC(dpy, _id, gcmask, &gcval);

  gcval.foreground = _hilitePixel;
  gcval.background = back; //core.background_pixel;
  gcval.foreground ^= gcval.background;
  gcval.function   = GXxor;
  gcval.plane_mask = AllPlanes;
  gcmask = GCForeground | GCBackground | GCFunction | GCPlaneMask;
  _highlightingGC = XCreateGC(dpy, _id, gcmask, &gcval);

  gcval.foreground = _framingPixel;
  gcval.background = back; //core.background_pixel;
  //gcval.foreground ^= gcval.background;
  gcval.function   = GXcopy; //GXxor;
  gcval.plane_mask = AllPlanes;
  gcmask = GCForeground | GCBackground | GCFunction | GCPlaneMask;
  _framingGC = XCreateGC(dpy, _id, gcmask, &gcval);

  gcval.foreground = _transparentPixel;
  gcval.background = _clearPixel;
  gcval.function   = GXcopy;
  gcmask = GCForeground | GCBackground | GCFunction;
  if (_stipple != None) {
    gcval.stipple = _stipple;
    gcmask |= GCStipple | GCFillStyle;
  }
  gcval.fill_style = _stippled ? FillOpaqueStippled : FillSolid;
  _transparentGC = XCreateGC(dpy, _id, gcmask, &gcval);

  _storage = NULL;
  _mask_storage = NULL;
    
  _image = CreatePixmapImage(_width, _height);
  _mask_image = CreateMaskImage(_width, _height);

  _buffer = CreatePixmapImage(_width, _height);
  _mask_buffer = CreateMaskImage(_width, _height);

  // add transparent pixel to ColorTable
  _colorTable->UseColorInTable(TRANSPARENT(GetDisplay(), _screen), NULL,
                               NoColorName, NULL, NULL, NULL, NoColorName);

  // add clear pixel to ColorTable
  _colorTable->UseColorInTable(_clearPixel, NULL,
                               NULL, NULL, NULL, NULL, "white");

  // add black pixel to ColorTable
  _colorTable->UseColorInTable(_blackPixel, NULL,
                               NULL, NULL, NULL, NULL, "black");

  //-------- this is temp here: add some useful colors until we
  //         will be able to load a palette
  _colorTable->UseColorInTable(_client->GetColorByName("red"), NULL,
                               NULL, NULL, NULL, NULL, "red");
  _colorTable->UseColorInTable(_client->GetColorByName("yellow"), NULL,
                               NULL, NULL, NULL, NULL, "yellow");
  _colorTable->UseColorInTable(_client->GetColorByName("green"), NULL,
                               NULL, NULL, NULL, NULL, "green");
  _colorTable->UseColorInTable(_client->GetColorByName("blue"), NULL,
                               NULL, NULL, NULL, NULL, "blue");
  _colorTable->UseColorInTable(_client->GetColorByName("cyan"), NULL,
                               NULL, NULL, NULL, NULL, "cyan");
  _colorTable->UseColorInTable(_client->GetColorByName("magenta"), NULL,
                               NULL, NULL, NULL, NULL, "magenta");

  _colorTable->UseColorInTable(_client->GetColorByName("#c0c0c0"), NULL,
                               NULL, NULL, NULL, NULL, "#c0c0c0");

  _colorTable->UseColorInTable(_client->GetColorByName("#808080"), NULL,
                               NULL, NULL, NULL, NULL, "#808080");
  _colorTable->UseColorInTable(_client->GetColorByName("#000080"), NULL,
                               NULL, NULL, NULL, NULL, "#000080");
  _colorTable->UseColorInTable(_client->GetColorByName("#008080"), NULL,
                               NULL, NULL, NULL, NULL, "#008080");
  _colorTable->UseColorInTable(_client->GetColorByName("#008000"), NULL,
                               NULL, NULL, NULL, NULL, "#008000");
  _colorTable->UseColorInTable(_client->GetColorByName("#808000"), NULL,
                               NULL, NULL, NULL, NULL, "#808000");
  _colorTable->UseColorInTable(_client->GetColorByName("#800000"), NULL,
                               NULL, NULL, NULL, NULL, "#800000");
  _colorTable->UseColorInTable(_client->GetColorByName("#800080"), NULL,
                               NULL, NULL, NULL, NULL, "#800080");


  // Read file
  _readFile(_filename);

  XGrabButton(GetDisplay(), Button1, AnyModifier, _id, True,
              ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
              GrabModeAsync, GrabModeAsync, None, None);

  _pressed = False;
  SetMode(DRAW_POINT);

  Resize(_width * _squareSize + 2 * _distance,
         _height * _squareSize + 2 * _distance);

  _Resize();
}

OXPaintCanvas::~OXPaintCanvas() {
  delete _colorTable;

  XFreeGC(GetDisplay(), _drawingGC);
  XFreeGC(GetDisplay(), _highlightingGC);
  XFreeGC(GetDisplay(), _framingGC);
  XFreeGC(GetDisplay(), _transparentGC);

  DestroyPixmapImage(&_image);
  DestroyPixmapImage(&_buffer);
  DestroyMaskImage(&_mask_image);
  DestroyMaskImage(&_mask_buffer);

  if (_stipple != None) XFreePixmap(GetDisplay(), _stipple);

  if (_font_struct) {
    XFreeFontInfo(NULL, _font_struct, 0);
    _client->FreeFont(_font);
  }
}

ODimension OXPaintCanvas::GetDefaultSize() const {
  return ODimension(_width * _squareSize + 2 * _distance,
                    _height * _squareSize + 2 * _distance);
}

int OXPaintCanvas::HandleExpose(XExposeEvent *event) {
  Refresh(event->x, event->y, event->width, event->height);
  return True;
}

void OXPaintCanvas::SetMode(int mode) {
  _drawMode = mode;

  switch (_drawMode) {
    default:
    case DRAW_POINT:
    case DRAW_CURVE:
    case DRAW_LINE:
      XDefineCursor(GetDisplay(), _id, _drawCursor);
      break;

    case DRAW_RECTANGLE:
    case DRAW_FILLED_RECTANGLE:
    case DRAW_CIRCLE:
    case DRAW_FILLED_CIRCLE:
    case DRAW_TEXT:
    case FLOOD_FILL:
    case SELECT_REGION:
      XDefineCursor(GetDisplay(), _id, _selCursor);
      break;

    case MOVE_REGION:
      XDefineCursor(GetDisplay(), _id, _moveCursor);
      break;

  }
}

int OXPaintCanvas::HandleButton(XButtonEvent *event) {
  if (event->type == ButtonPress) {

    _x0 = _x1 = InPixmapX(event->x);
    _y0 = _y1 = InPixmapY(event->y);
    _pressed = True;

    //StoreToBuffer();

    switch (_drawMode) {
      case DRAW_POINT:
        DrawPoint(_x0, _y0, SET);
        _changed = True;
        break;

      case DRAW_CURVE:
        DrawLine(_x0, _y0, _x1, _y1, SET);
        _changed = True;
        break;

      case DRAW_LINE:
        DrawLine(_x0, _y0, _x1, _y1, HIGHLIGHT);
        break;

      case DRAW_RECTANGLE:
      case DRAW_FILLED_RECTANGLE:
      case SELECT_REGION:
        DrawRectangle(_x0, _y0, _x1, _y1, HIGHLIGHT);
        break;

      case DRAW_CIRCLE:
      case DRAW_FILLED_CIRCLE:
        DrawCircle(_x0, _y0, _x1, _y1, HIGHLIGHT);
        break;

      case DRAW_TEXT:
        DragText(_x0, _y0, HIGHLIGHT);
        break;

      case MOVE_REGION:
        DragStored(_x0, _y0);
        break;

      default:
        break;
    }

  } else {
    int old_x1 = _x1, old_y1 = _y1;
    _x1 = InPixmapX(event->x);
    _y1 = InPixmapY(event->y);
    _pressed = False;

    switch (_drawMode) {
      case DRAW_POINT:
        break;

      case DRAW_CURVE:
        DrawLine(old_x1, old_y1, _x1, _y1, SET);
        _changed = True;
        break;

      case DRAW_LINE:
        DrawLine(_x0, _y0, old_x1, old_y1, HIGHLIGHT);
        DrawLine(_x0, _y0, _x1, _y1, SET);
        _changed = True;
        break;

      case DRAW_RECTANGLE:
        DrawRectangle(_x0, _y0, old_x1, old_y1, HIGHLIGHT);
        DrawRectangle(_x0, _y0, _x1, _y1, SET);
        _changed = True;
        break;

      case DRAW_FILLED_RECTANGLE:
        DrawRectangle(_x0, _y0, old_x1, old_y1, HIGHLIGHT);
        DrawFilledRectangle(_x0, _y0, _x1, _y1, SET);
        _changed = True;
        break;

      case DRAW_CIRCLE:
        DrawCircle(_x0, _y0, old_x1, old_y1, HIGHLIGHT);
        DrawCircle(_x0, _y0, _x1, _y1, SET);
        _changed = True;
        break;

      case DRAW_FILLED_CIRCLE:
        DrawCircle(_x0, _y0, old_x1, old_y1, HIGHLIGHT);
        DrawFilledCircle(_x0, _y0, _x1, _y1, SET);
        _changed = True;
        break;

      case FLOOD_FILL:
        FloodFill(_x1, _y1, SET);
        _changed = True;
        break;

      case DRAW_TEXT:
        DrawText(old_x1, old_y1, HIGHLIGHT);
        DrawText(_x1, _y1, SET);
        _changed = True;
        break;

      case SELECT_REGION:
        DrawRectangle(_x0, _y0, old_x1, old_y1, HIGHLIGHT);
        Select(_x0, _y0, _x1, _y1, CurrentTime);
        break;

      case MOVE_REGION:
        //???
        DragStored(_x1, _y1);
        Restore(_x1, _y1, SET);
        SetMode(_lastMode);
        _changed = True;
        break;

      default:
        break;
    }

  }

  return True;
}

int OXPaintCanvas::HandleMotion(XMotionEvent *event) {
  int old_x1 = _x1, old_y1 = _y1;

  if (_pressed) {
    _x1 = InPixmapX(event->x);
    _y1 = InPixmapY(event->y);

    if ((_x1 != old_x1) || (_y1 != old_y1)) {

      switch (_drawMode) {
        case DRAW_POINT:
          DrawPoint(_x1, _y1, SET);
          _changed = True;
          break;

        case DRAW_CURVE:
          DrawLine(old_x1, old_y1, _x1, _y1, SET);
          _changed = True;
          break;

        case DRAW_LINE:
          DrawLine(_x0, _y0, old_x1, old_y1, HIGHLIGHT);
          DrawLine(_x0, _y0, _x1, _y1, HIGHLIGHT);
          break;

        case DRAW_RECTANGLE:
        case DRAW_FILLED_RECTANGLE:
        case SELECT_REGION:
          DrawRectangle(_x0, _y0, old_x1, old_y1, HIGHLIGHT);
          DrawRectangle(_x0, _y0, _x1, _y1, HIGHLIGHT);
          break;

        case DRAW_CIRCLE:
        case DRAW_FILLED_CIRCLE:
          DrawCircle(_x0, _y0, old_x1, old_y1, HIGHLIGHT);
          DrawCircle(_x0, _y0, _x1, _y1, HIGHLIGHT);
          break;

        case DRAW_TEXT:
          DragText(old_x1, old_y1, HIGHLIGHT);
          DragText(_x1, _y1, HIGHLIGHT);
          break;

        case MOVE_REGION:
          DragStored(old_x1, old_y1);
          DragStored(_x1, _y1);
          break;

        default:
          break;

      }
    }

  }
  return True;
}

//-------------------------------------------------------------------------

Pixmap OXPaintCanvas::GetPixmap(XImage *image) {
  Pixmap pix;
  XGCValues gcv;
  GC gc;  // could use _drawingGC as well, as it has GXcopy mode

  pix = XCreatePixmap(GetDisplay(), _id,
		      image->width, image->height, image->depth);
  gcv.function = GXcopy;
  gc = XCreateGC(GetDisplay(), pix, GCFunction, &gcv);
  
  XPutImage(GetDisplay(), pix, gc, image, 0, 0, 0, 0,
	    image->width, image->height);

  XFreeGC(GetDisplay(), gc);
  return pix;
}

XImage *OXPaintCanvas::GetImage(Pixmap pixmap) {
  Window root;
  int x, y;
  unsigned int width, height, border_width, depth;
  XImage *image;

  XGetGeometry(GetDisplay(), pixmap, &root, &x, &y,
               &width, &height, &border_width, &depth);

  image = XGetImage(GetDisplay(), pixmap, x, y, width, height,
                    AllPlanes, ZPixmap);

  return image;
}

XImage *OXPaintCanvas::CreatePixmapImage(int width, int height) {
  int bitmap_pad;
  XImage *image;
  Pixel wp;
 
  if (_depth <= 8)
    bitmap_pad = 8;
  else if (_depth <= 16)
    bitmap_pad = 16;
  else
    bitmap_pad = 32;
  
  image = XCreateImage(GetDisplay(), DefaultVisual(GetDisplay(), _screen),
		       _depth, ZPixmap, 0,
		       NULL, width, height, bitmap_pad, 0);

  if (!image) {
    FatalError("CreatePixmapImage -- error creating XImage\n");
    exit(1);
  } else {
    // use calloc here, as data will be free'd by Xpm lib
    image->data = (char *) calloc(1, image->bytes_per_line * height);
  }

  wp = WhitePixelOfColormap(GetDisplay(), _colormap);
  if (wp != 0)			// to clear the image, hope white is or 0 
				// or greater than black, because
    XAddPixel(image, wp);	// image pixels are initialized to 0

  return image;
}

XImage *OXPaintCanvas::CreateMaskImage(int width, int height) {
  XImage *mask_image;

  mask_image = XCreateImage(GetDisplay(),
                            DefaultVisual(GetDisplay(), _screen),
                            1, ZPixmap, 0, NULL, width, height, 8, 0);

  if (!mask_image) {
    FatalError("CreateMaskImage -- error creating XImage\n");
    exit(1);
  } else {
    // use calloc here, as it will be free'd by Xpm lib...
    mask_image->data = (char *) calloc(1, mask_image->bytes_per_line *
		                          mask_image->height);
  }

  // Initialize all pixels to 1: default mask is rectangular
  XAddPixel(mask_image, 1);
  
  return mask_image;
}

void OXPaintCanvas::DestroyPixmapImage(XImage **image) {
  if (*image) {
    XDestroyImage(*image);
    *image = NULL;
  }
}

void OXPaintCanvas::Select(int from_x, int from_y,
                           int to_x, int to_y, Time time) {
  Mark(from_x, from_y, to_x, to_y);
  _GrabSelection(time);
}

void OXPaintCanvas::SetTransparentPixels(XImage *image, XImage *mask_image) {
  register int x, y;
    
  for (x=0; x<image->width; x++)
    for (y=0; y<image->height; y++)
      if (XGetPixel(mask_image, x, y) == 0)
	XPutPixel(image, x, y, _transparentPixel);
}

void OXPaintCanvas::_InitializeXpmAttributes(XpmAttributes *attribs) {
  attribs->valuemask   = XpmVisual | XpmColormap | XpmDepth;
  attribs->visual      = DefaultVisual(GetDisplay(), _screen);
  attribs->colormap    = _colormap;
  attribs->depth       = _depth;
  attribs->width       = 0;
  attribs->height      = 0;
  attribs->x_hotspot   = 0;
  attribs->y_hotspot   = 0;
  attribs->cpp         = 0;
  attribs->pixels      = (Pixel *)0;
  attribs->npixels     = 0;
  attribs->colorsymbols = (XpmColorSymbol *)0;
  attribs->numsymbols  = 0;
  attribs->rgb_fname   = rgb_fname;
  attribs->nextensions = 0;
  attribs->extensions  = (XpmExtension *)0;
  attribs->ncolors     = 0;
  attribs->colorTable  = (XpmColor *)0;
  attribs->hints_cmt   = NULL;
  attribs->colors_cmt  = NULL;
  attribs->pixels_cmt  = NULL;
  attribs->closeness   = 0;
  attribs->exactColors = 1;
}

void OXPaintCanvas::_InitializeXpmInfo(XpmInfo *infos) {
  infos->hints_cmt   = NULL;
  infos->colors_cmt  = NULL;
  infos->pixels_cmt  = NULL;
  infos->nextensions = 0;
  infos->extensions  = NULL;
}

void OXPaintCanvas::_InitializeXpmImage(XpmImage *xpm_image) {
  xpm_image->width      = 0;
  xpm_image->height     = 0;
  xpm_image->cpp        = 0;
  xpm_image->ncolors    = 0;
  xpm_image->colorTable = (XpmColor *)0;
  xpm_image->data       = (unsigned int *)0;
}

int OXPaintCanvas::_readFile(char *filename) {
  int status, i, shift = 0;
  XImage *image, *mask_image;
  XpmAttributes attribs;
  Pixel *pixel;
  XpmInfo infos;
  XpmImage xpm_image;
  XpmColor *color;

  _InitializeXpmInfo(&infos);
  _InitializeXpmImage(&xpm_image);
  _InitializeXpmAttributes(&attribs);
  infos.valuemask = XpmReturnComments | XpmReturnColorTable |
                    XpmReturnExtensions;
  attribs.valuemask |= XpmReturnPixels;

  status = XpmReadFileToXpmImage(filename, &xpm_image, &infos);
  if (status == XpmSuccess)
    status = XpmCreateImageFromXpmImage(GetDisplay(), &xpm_image, &image,
                                        &mask_image, &attribs);
  if (status == XpmSuccess) {
    // Notify colors to be loaded with this pixmap
    for (i = 0, color = xpm_image.colorTable, pixel = attribs.pixels;
	        i < xpm_image.ncolors; i++, color++, pixel++) {
      // look for a defined color value
      char *c_name;
      char **defaults = (char **)color;
      int default_index, def_index;

      switch (_depth) {
	  case 1:
	      default_index = 2;
	      break;
	  case 4:
	      default_index = 3;
	      break;
	  case 6:
	      default_index = 4;
	      break;
	  default:
	  case 8:
	      default_index = 5;
	      break;
      }
      def_index = default_index;
      while (def_index > 1 && defaults[def_index] == NULL) --def_index;
      if (def_index < 2) {
        // nothing towards mono, so try towards color
        def_index = default_index + 1;
        while (def_index <= 5 && defaults[def_index] == NULL) ++def_index;
      }
      if (def_index > 5) {
        fprintf(stderr, "color #%d undefined, using black.\n", i + 1);
        c_name = "black";
      } else {
        c_name = defaults[def_index];
      }

      if (strcasecmp(c_name, "none") == 0) {
        _colorTable->UpdateColor(TRANSPARENT(GetDisplay(), _screen), 
			       color->string, 
			       color->symbolic,
			       color->m_color,
			       color->g4_color,
			       color->g_color,
			       color->c_color);
        shift++;
      } else {
        _colorTable->UseColorInTable(*(pixel-shift),
			             color->string, 
			             color->symbolic,
			             color->m_color,
			             color->g4_color,
			             color->g_color,
			             color->c_color);
      }
    }

    if (_hints_cmt) delete[] _hints_cmt;
    if (infos.hints_cmt)
      _hints_cmt = _NewString(infos.hints_cmt);
    else
      _hints_cmt = NULL;

    if (_colors_cmt) delete[] _colors_cmt;
    if (infos.colors_cmt) 
      _colors_cmt = _NewString(infos.colors_cmt);
    else
      _colors_cmt = NULL;

    if (_pixels_cmt) delete[] _pixels_cmt;
    if (infos.pixels_cmt) 
      _pixels_cmt = _NewString(infos.pixels_cmt);
    else
      _pixels_cmt = NULL;

    if (!mask_image) // libXpm returns NULL when transp. is not used
      mask_image = CreateMaskImage(image->width, image->height);
    else
      SetTransparentPixels(image, mask_image);
      
    DestroyPixmapImage(&_buffer);
    DestroyMaskImage(&_mask_buffer);

    _buffer = _image;
    _mask_buffer = _mask_image;

    _image = image;
    _mask_image = mask_image;
    _width  = xpm_image.width;
    _height = xpm_image.height;

    if (infos.valuemask & XpmHotspot) {
      _hot.x = infos.x_hotspot;
      _hot.y = infos.y_hotspot;	
    } else {
      _hot.x = _hot.y = NOT_SET;
    }

    //CopyExtensions(&_extensions, &_nextensions,
    //                   infos.extensions, infos.nextensions );
    //if (_extensionNotify) _extensionNotify();

    _changed = False;
    XpmFreeAttributes(&attribs);
    XpmFreeXpmImage(&xpm_image);
    XpmFreeXpmInfo(&infos);
  }

  return status;
}

void OXPaintCanvas::_FixMark() {
  if (QuerySet(_mark.from_x, _mark.from_y)) {
    _mark.from_x = min(_mark.from_x, _image->width);
    _mark.from_y = min(_mark.from_y, _image->height);
    _mark.to_x   = min(_mark.to_x,   _image->width);
    _mark.to_y   = min(_mark.to_y,   _image->height);

    if ((_mark.from_x == _mark.from_y) &&
        (_mark.to_x   == _mark.to_y)) {
      _mark.from_x = 
      _mark.from_y =
      _mark.to_x   = 
      _mark.to_y   = NOT_SET;
    }
  }
}

int OXPaintCanvas::StoreFile(char *filename) {
  int status, i, shift = 0;
  XImage *image, *mask_image;
  XpmAttributes attribs;
  Pixel *pixel;
  XpmInfo infos;
  XpmImage xpm_image;
  XpmColor *color;

  _InitializeXpmInfo(&infos);
  _InitializeXpmImage(&xpm_image);
  _InitializeXpmAttributes(&attribs);

  infos.valuemask = XpmReturnColorTable;
  attribs.valuemask |= XpmReturnPixels;

  if ((status = XpmReadFileToXpmImage(filename, &xpm_image, &infos))
	== XpmSuccess)
    status = XpmCreateImageFromXpmImage(GetDisplay(), &xpm_image, &image,
					  &mask_image, &attribs);
  if (status == XpmSuccess) {
    // Notify colors to be loaded with this pixmap
    for (i = 0, color = xpm_image.colorTable, pixel = attribs.pixels;
	        i < xpm_image.ncolors; i++, color++, pixel++) {
      char *c_name;

      switch (_depth) {
        case 1:
	  c_name = color->m_color;
	  break;
        case 4:
	  c_name = color->g4_color;
	  break;
        case 6:
	  c_name = color->g_color;
	  break;
        case 8:
        default:
	  c_name = color->c_color;
	  break;
      }

      if (strcasecmp(c_name, "none") == 0) {
	_colorTable->UpdateColor(TRANSPARENT(GetDisplay(), _screen), 
			color->string, color->symbolic,
			color->m_color, color->g4_color,
			color->g_color, color->c_color);
	shift = 1;
      } else {
	_colorTable->UseColorInTable(*(pixel-shift),
			color->string, color->symbolic,
			color->m_color, color->g4_color,
			color->g_color, color->c_color);
      }
    }

    if (!mask_image) // Xpm returns NULL when transp. is not used
      mask_image = CreateMaskImage(image->width, image->height);
    else
      SetTransparentPixels(image, mask_image);
	
    DestroyPixmapImage(&_storage);
    DestroyMaskImage(&_mask_storage);

    _storage = image;
    _mask_storage = mask_image;
    
    if (infos.valuemask & XpmHotspot) {
      _storage_hot.x = infos.x_hotspot;
      _storage_hot.y = infos.y_hotspot;	
    } else {
      _storage_hot.x = _storage_hot.y = NOT_SET;
    }

    XpmFreeAttributes(&attribs);
    XpmFreeXpmImage(&xpm_image);
    XpmFreeXpmInfo(&infos);  
  } else {
//    XtAppWarning(XtWidgetToApplicationContext(w),
//	     " read file failed.  PixmapWidget");
  }

  return status;
}

char *OXPaintCanvas::_NewString(char *str) {
  if (!str) return NULL;

  char *s = new char[strlen(str)+1];
  if (s) strcpy(s, str);

  return s;
}

void OXPaintCanvas::ChangeFilename(char *str) {
  if (str)
    if (strcmp(str, "")) {
      if (_filename) delete[] _filename;
      _filename = _NewString(str);
    }
}


// This function sets or gets the pixmap comments.
// If passing NULL, a comment is returned, otherwise it is stored. 
// Memory is allocated for returned comments, should be freed by 
// the application

void OXPaintCanvas::Comments(char **hints_cmt, char **colors_cmt, 
                        char **pixels_cmt) {
  
  if ((*hints_cmt) && (_hints_cmt)) {
    delete[] _hints_cmt;
    _hints_cmt = _NewString(*hints_cmt);
  } else if (*hints_cmt) {
    _hints_cmt = _NewString(*hints_cmt);
  } else {
    *hints_cmt = _NewString(_hints_cmt);
  }

  if ((*colors_cmt) && (_colors_cmt)) {
    delete[] _colors_cmt;
    _colors_cmt = _NewString(*colors_cmt);
  } else if (*colors_cmt) {
    _colors_cmt = _NewString(*colors_cmt);
  } else {
    *colors_cmt = _NewString(_colors_cmt);
  }

  if ((*pixels_cmt) && (_pixels_cmt)) {
    delete[] _pixels_cmt;
    _pixels_cmt = _NewString(*pixels_cmt);
  } else if (*pixels_cmt) {
    _pixels_cmt = _NewString(*pixels_cmt);
  } else {
    *pixels_cmt = _NewString(_pixels_cmt);
  }
}

  
int OXPaintCanvas::ReadFile(char *filename) {
  int status, resized;

  status = _readFile(filename);
  if (status == XpmSuccess) {
    if (_filename) delete[] _filename;
    _filename = _NewString(filename);

    Unmark();

    resized = (((_width  * _squareSize + 2 * _distance) != _w) ||
               ((_height * _squareSize + 2 * _distance) != _h));
    Resize(_width * _squareSize + 2 * _distance,
           _height * _squareSize + 2 * _distance);

    if (!resized)
      XClearArea(GetDisplay(), _id, 0, 0, _w, _h, True); 

  } else {
//    XtAppWarning(XtWidgetToApplicationContext(w),
//		     " read file failed.  PixmapWidget");
  }

  return status;
}

void OXPaintCanvas::SetImage(XImage *image, XImage *mask_image) {
  XImage *buffer, *mask_buffer;
  int resized;
    
  buffer = CreatePixmapImage(image->width, image->height);
  mask_buffer = CreateMaskImage(image->width, image->height);

  TransferImageData(_image, buffer);
  TransferImageData(_mask_image, mask_buffer);
    
  DestroyPixmapImage(&_image);
  DestroyPixmapImage(&_buffer);
  DestroyMaskImage(&_mask_image);
  DestroyMaskImage(&_mask_buffer);

  _image = image;
  _mask_image = mask_image;
  _buffer = buffer;
  _mask_buffer = mask_buffer;
  _width = image->width;
  _height = image->height;
    
  resized = (((_width  * _squareSize + 2 * _distance) != _w) ||
             ((_height * _squareSize + 2 * _distance) != _h));
  Resize(_width * _squareSize + 2 * _distance,
         _height * _squareSize + 2 * _distance);
  if (!resized) XClearArea(GetDisplay(), _id, 0, 0, _w, _h, True); 

}

// Number of printable ASCII chars minus \ and " for string compat
// and ? to avoid ANSI trigraphs.

#define MAXPRINTABLE 92		

static char *printable =
" .XoO+@#$%&*=-;:>,<1234567890qwertyuipasdfghjklzxcvbnmMNBVCZ\
ASDFGHJKLPIUYTREWQ!~^/()_`'][{}|";

void OXPaintCanvas::BuildXpmImageAndAttributes(XImage *image, XImage *mask_image,
                   XImage **xpm_image_return, XpmInfo **infos_return) {
  register int x, y;
  register Pixel pxl;
  unsigned int *data, color_index = 0;
  unsigned int i, j, k;
  XpmColor *color;
  XpmImage *xpm_image = (XpmImage *) calloc(1, sizeof(XpmImage)); //new XpmImage;
  XpmInfo *infos = (XpmInfo *) calloc(1, sizeof(XpmInfo)); //new XpmInfo;

  _colorTable->ResetColorsUse();

  if (_hints_cmt || _colors_cmt || _pixels_cmt) {
    infos->valuemask = XpmComments;

    if (_hints_cmt)
      infos->hints_cmt = strdup(_hints_cmt);
    else
      infos->hints_cmt = NULL;

    if (_colors_cmt)
      infos->colors_cmt = strdup(_colors_cmt);
    else
      infos->colors_cmt = NULL;

    if (_pixels_cmt)
      infos->pixels_cmt = strdup(_pixels_cmt);
    else
      infos->pixels_cmt = NULL;

  } else {
    infos->valuemask = 0;
  }

  infos->x_hotspot = 0;
  infos->y_hotspot = 0;
  infos->nextensions = 0;
  infos->extensions = NULL;

  xpm_image->width  = image->width;
  xpm_image->height = image->height;
  xpm_image->cpp = 0;
  // use calloc here, as data will be free'd by Xpm lib 
  xpm_image->data = (unsigned int *) calloc(image->width * image->height,
                                            sizeof(unsigned int));
  xpm_image->colorTable = NULL;
  xpm_image->ncolors = 0;
  
  for (y = 0, data = xpm_image->data; y < image->height; y++)
    for (x = 0; x < image->width; x++, data++) {
      OColorInfo *pw_color;

      pxl = _GetPxlFromImageAndMask(image, mask_image, x, y);
      pw_color = _colorTable->GetColor(pxl);

      if ((color_index = pw_color->in_xpm) == -1) {
        // Add the color in colorTable
        color_index = xpm_image->ncolors;
        pw_color->in_xpm = color_index;

        xpm_image->ncolors++;

        // use realloc and strdup here, as the structure will be
        // free'd by Xpm lib.
        xpm_image->colorTable =
          (XpmColor *)realloc((char *)xpm_image->colorTable,
				    xpm_image->ncolors * sizeof(XpmColor));
        color = xpm_image->colorTable+color_index;

        color->symbolic = 
          (char *) (pw_color->s_name ? strdup(pw_color->s_name) : NULL);
        color->m_color  =
          (char *) (pw_color->m_name ? strdup(pw_color->m_name) : NULL);
        color->g4_color =
          (char *) (pw_color->g4_name ? strdup(pw_color->g4_name) : NULL);
        color->g_color  =
          (char *) (pw_color->g_name ? strdup(pw_color->g_name) : NULL);
        color->c_color  =
          (char *) (pw_color->c_name ? strdup(pw_color->c_name) : NULL);
      }

      // put color index in XpmImage
      *data = color_index;
    }

  xpm_image->cpp = (xpm_image->ncolors / MAXPRINTABLE) + 1;

  for (i=0, color=xpm_image->colorTable; i<xpm_image->ncolors;
       i++, color++) {
    char *s;
      
    color->string = (char *) malloc(xpm_image->cpp + 1);
    s = color->string;
      
    *s++ = printable[k = i % MAXPRINTABLE];
    for (j=1; j<xpm_image->cpp; j++, s++)
      *s = printable[k = ((i - k) / MAXPRINTABLE) % MAXPRINTABLE];
    *s = '\0';
  }
	
  *xpm_image_return = (XImage *) xpm_image;
  *infos_return = infos;
}

int OXPaintCanvas::WriteFile(char *filename) {
  XImage *image, *mask_image;
  XPoint hot;
  int status;
  XpmImage *xpm_image;
  XpmInfo *infos;

  image = _image;
  mask_image = _mask_image;
  hot = _hot;
    
  if (filename) ChangeFilename(filename);

  BuildXpmImageAndAttributes(image, mask_image, (XImage **) &xpm_image, &infos);
  if (QuerySet(hot.x, hot.y)) {
    infos->valuemask |= XpmHotspot;
    infos->x_hotspot = hot.x;
    infos->y_hotspot = hot.y;
  }

  // need to account for zooming for ports ??
  //infos->valuemask |= XpmExtensions;   // save extensions
  //CopyExtensions(&infos->extensions, &infos->nextensions,
  //               _extensions, _nextensions);

  status = XpmWriteFileFromXpmImage(_filename, xpm_image, infos);

  XpmFreeXpmImage(xpm_image);
  delete xpm_image;
  XpmFreeXpmInfo(infos);
  delete infos;

  if (status == XpmSuccess) _changed = False;

  return status;
}

char *OXPaintCanvas::GetFilepath(char **str) {
  char *end;

  *str = _NewString(_filename);
  end = rindex(*str, '/');

  if (end)
    *(end + 1) = '\0';
  else 
    **str = '\0';

  return *str;
}

void OXPaintCanvas::_FixHotSpot() {
  if (!QueryInPixmap(_hot.x, _hot.y))
    _hot.x = _hot.y = NOT_SET;
}

void OXPaintCanvas::SetZoom(int zoom) {
  _squareSize = max(min(zoom, 20), 1);
  Resize(_width * _squareSize + 2 * _distance,
         _height * _squareSize + 2 * _distance);
  //XClearArea(GetDisplay(), _id, 0, 0, _w, _h, True); 
}

void OXPaintCanvas::RescaleImage(int width, int height) {
  XImage *image, *mask_image;

  image = ScalePixmapImage(_image, 
		       (double) width / (double) _image->width,
		       (double) height / (double) _image->height);
  mask_image = ScaleMaskImage(_mask_image, 
		     (double) width / (double) _mask_image->width,
		     (double) height / (double) _mask_image->height);

  DestroyPixmapImage(&_image);
  DestroyMaskImage(&_mask_image);
    
  _image = image;
  _mask_image = mask_image;
  _width  = image->width;
  _height = image->height;
    
  _FixHotSpot();
  _FixMark();

  Resize(_width * _squareSize + 2 * _distance,
         _height * _squareSize + 2 * _distance);
  //XClearArea(GetDisplay(), _id, 0, 0, _w, _h, True);

  _changed = True;
}

void OXPaintCanvas::ResizeImage(int width, int height) {
  XImage *image, *mask_image;

  image = CreatePixmapImage(width, height);
  mask_image = CreateMaskImage(width, height);

  TransferImageData(_image, image);
  TransferImageData(_mask_image, mask_image);
    
  DestroyPixmapImage(&_image);
  DestroyMaskImage(&_mask_image);

  _image = image;
  _mask_image = mask_image;
  _width  = width;
  _height = height;

  _FixHotSpot();
  _FixMark();

  Resize(_width * _squareSize + 2 * _distance,
         _height * _squareSize + 2 * _distance);
  //XClearArea(GetDisplay(), _id, 0, 0, _w, _h, True);

  _changed = True;
}

void OXPaintCanvas::_Resize() {
  _horizOffset = max(_distance, (_w - _width * _squareSize) >> 1);
  _vertOffset  = max(_distance, (_h - _height * _squareSize) >> 1);
  _grid &= (_squareSize > _gridTolerance);
}

void OXPaintCanvas::Clip(int from_x, int from_y, int to_x, int to_y) {
  XRectangle rectangle;
  
  QuerySwap(from_x, to_x);
  QuerySwap(from_y, to_y);
  from_x = max(0, from_x);
  from_y = max(0, from_y);
  to_x = min(_width - 1, to_x);
  to_y = min(_height - 1, to_y);

  rectangle.x = InWindowX(from_x);
  rectangle.y = InWindowY(from_y);
  rectangle.width  = InWindowX(to_x + 1) - InWindowX(from_x);
  rectangle.height = InWindowY(to_y + 1) - InWindowY(from_y);

  XSetClipRectangles(GetDisplay(), _highlightingGC,
		       0, 0, &rectangle, 1, Unsorted);

  XSetClipRectangles(GetDisplay(), _drawingGC,
		       0, 0, &rectangle, 1, Unsorted);

  XSetClipRectangles(GetDisplay(), _framingGC,
                       0, 0, &rectangle, 1, Unsorted);
}

void OXPaintCanvas::Unclip() {
  XRectangle rectangle;
  
  rectangle.x = InWindowX(0);
  rectangle.y = InWindowY(0);
  rectangle.width = InWindowX(_width) - InWindowX(0);
  rectangle.height = InWindowY(_height) - InWindowY(0);

  XSetClipRectangles(GetDisplay(), _highlightingGC,
		       0, 0, &rectangle, 1, Unsorted);

  XSetClipRectangles(GetDisplay(), _drawingGC,
		       0, 0, &rectangle, 1, Unsorted);

  XSetClipRectangles(GetDisplay(), _framingGC,
                       0, 0, &rectangle, 1, Unsorted);
}

void OXPaintCanvas::Refresh(int x, int y, int width, int height) {
  XRectangle rectangle;

  rectangle.x = min(x, InWindowX(InPixmapX(x)));
  rectangle.y = min(y, InWindowY(InPixmapY(y)));
  rectangle.width  = max(x + width,
		      InWindowX(InPixmapX(x + width)+1)) - rectangle.x;
  rectangle.height = max(y + height,
		      InWindowY(InPixmapY(y + height)+1)) - rectangle.y;

  XClearArea(GetDisplay(), _id, rectangle.x, rectangle.y,
             rectangle.width, rectangle.height, False);

  XSetClipRectangles(GetDisplay(), _framingGC,
                     0, 0, &rectangle, 1, Unsorted);

  XDrawRectangle(GetDisplay(), _id, _framingGC,
		 InWindowX(0) - 1, InWindowY(0) - 1,
		 InWindowX(_width) - InWindowX(0) + 1, 
		 InWindowY(_height) - InWindowY(0) + 1);

  Clip(InPixmapX(x), InPixmapY(y),
       InPixmapX(x + width), InPixmapY(y + height));

  RedrawSquares(InPixmapX(x), InPixmapY(y),
		InPixmapX(x + width), InPixmapY(y + height));

  RedrawGrid(InPixmapX(x), InPixmapY(y),
	     InPixmapX(x + width), InPixmapY(y + height));

  RedrawMark();
  RedrawHotSpot();

  Unclip();
}

void OXPaintCanvas::SetGrid(int state) {
  if (_grid == state) return;
  _grid = state;
  if (_grid)
    RedrawGrid(0, 0, _image->width - 1, _image->height - 1);
  else
    Refresh(InWindowX(0), InWindowY(0),
            InWindowX(_image->width - 1), InWindowY(_image->height - 1));
}

void OXPaintCanvas::SetFont(OXFont *font) {
  if (_font_struct) {
    XFreeFontInfo(NULL, _font_struct, 0);
    _client->FreeFont(_font);
  }

  _font = font;
  _font_struct = XQueryFont(GetDisplay(), _font->GetId());

  if (_font_struct && _text_string) {
    int         direction, ascent, descent;
    XCharStruct size;

    XTextExtents(_font_struct, _text_string, strlen(_text_string), 
                 &direction, &ascent, &descent, &size);

    _text_rbearing = size.rbearing;
    _text_lbearing = size.lbearing;
    _text_ascent   = size.ascent;
    _text_descent  = size.descent;
  }
}

void OXPaintCanvas::SetText(char *s) {

  if (_text_string) delete[] _text_string;
  _text_string = _NewString(s);

  if (_font_struct && _text_string) {
    int          direction, ascent, descent;
    XCharStruct  size;

    XTextExtents(_font_struct, _text_string, strlen(_text_string), 
                 &direction, &ascent, &descent, &size );

    _text_rbearing = size.rbearing;
    _text_lbearing = size.lbearing;
    _text_ascent   = size.ascent;
    _text_descent  = size.descent;
  }
}


//-------------------------------------------------------------------------

void OXPaintCanvas::CopyImageData(XImage *source, XImage *destination, 
     int from_x, int from_y, int to_x, int to_y, int at_x, int at_y) {
  register int x, y, delta_x, delta_y; 

  delta_x = to_x - from_x + 1;
  delta_y = to_y - from_y + 1;
  
  for (x=0; x<delta_x; x++)
    for (y=0; y<delta_y; y++)
      XPutPixel(destination, at_x + x, at_y + y,
                XGetPixel(source, from_x + x, from_y + y));
}

void OXPaintCanvas::TransferImageData(XImage *source, XImage *destination) {
  register int x, y;
  register Pixel color;  
  
  for (x=0; x<min(source->width, destination->width); x++)
    for (y=0; y<min(source->height, destination->height); y++)
      if ((color = XGetPixel(source, x, y)) != XGetPixel(destination, x, y))
        XPutPixel(destination, x, y, color);
}
 

//-------------------------------------------------------------------------

// Drawing routines...

Pixel OXPaintCanvas::_GetPxl(int x, int y) {
  Pixel pixel;
  
  pixel = XGetPixel(_image, x, y);
  if ((pixel == _transparentPixel) && (XGetPixel(_mask_image, x, y) == 0))
    return TRANSPARENT(GetDisplay(), _screen);
  else
    return pixel;
}

void OXPaintCanvas::_SetPxl(int x, int y, Pixel color) {
  if (color == TRANSPARENT(GetDisplay(), _screen)) {
    XPutPixel(_image, x, y, _transparentPixel);
    XPutPixel(_mask_image, x, y, 0);
  } else {   
    XPutPixel(_image, x, y, color);
    XPutPixel(_mask_image, x, y, 1);
  }
}

Pixel OXPaintCanvas::_GetPxlFromImageAndMask(XImage *image,
                           XImage *mask_image, int x, int y) {
  Pixel pixel;

  pixel = XGetPixel(image, x, y);
  if (XGetPixel(mask_image, x, y) == 0)
    return TRANSPARENT(GetDisplay(), _screen);
  else
    return pixel;
}

Pixel OXPaintCanvas::GetPxl(int x, int y) {
  if (QueryInPixmap(x, y))
    return _GetPxl(x, y);  
  else
    return (Pixel) NOT_SET;
}

XPoint *OXPaintCanvas::_HotSpotShape(int x, int y) {
  static XPoint points[5];
    
  points[0].x = InWindowX(x);
  points[0].y = InWindowY(y + 1.0/2);
  points[1].x = InWindowX(x + 1.0/2);
  points[1].y = InWindowY(y + 1);
  points[2].x = InWindowX(x + 1);
  points[2].y = InWindowY(y + 1.0/2);
  points[3].x = InWindowX(x + 1.0/2);
  points[3].y = InWindowY(y);
  points[4].x = InWindowX(x);
  points[4].y = InWindowY(y + 1.0/2);

  return points;
}

void OXPaintCanvas::_DrawHotSpot(int x, int y) {
  XFillPolygon(GetDisplay(), _id, _framingGC,
               _HotSpotShape(x, y), 5, Convex, CoordModeOrigin);
}

void OXPaintCanvas::_HighlightHotSpot(int x, int y) {
  XFillPolygon(GetDisplay(), _id, _highlightingGC,
               _HotSpotShape(x, y), 5, Convex, CoordModeOrigin);
}
               
void OXPaintCanvas::_HighlightSquare(int x, int y) {
  XFillRectangle(GetDisplay(), _id, _highlightingGC,
                 InWindowX(x), InWindowY(y),
                 _squareSize, _squareSize);
}

void OXPaintCanvas::_ClearSquare(int x, int y) {
  XSetForeground(GetDisplay(), _drawingGC, _clearPixel);
  XFillRectangle(GetDisplay(), _id, _drawingGC,
                 InWindowX(x), InWindowY(y),
                 _squareSize, _squareSize);
}                

void OXPaintCanvas::_DrawSquare(int x, int y, Pixel color) {
  if (color == _clearPixel) {
    _ClearSquare(x, y);
  } else if (color == TRANSPARENT(GetDisplay(), _screen)) {
    XFillRectangle(GetDisplay(), _id, _transparentGC,
                   InWindowX(x), InWindowY(y),
                   _squareSize, _squareSize);
  } else {
    XSetForeground(GetDisplay(), _drawingGC, color);
    XFillRectangle(GetDisplay(), _id, _drawingGC,
                   InWindowX(x), InWindowY(y),
                   _squareSize, _squareSize);
    XSetForeground(GetDisplay(), _drawingGC, _fgndPixel);
  }
}

void OXPaintCanvas::_DrawPoint(int x, int y, Pixel color, int mode) {
  if ((mode == NotLazy) || (_GetPxl(x, y) != color)) {
    _DrawSquare(x, y, color);
    _SetPxl(x, y, color);
    // now put back the status of the pixel
    RedrawGrid(x, y, x, y);
    if (WithinMark(x, y)) _HighlightSquare(x, y);
    if (IsHotSpot(x, y)) RedrawHotSpot();
  }
}

void OXPaintCanvas::_ClearPoint(int x, int y, int mode) {
  if ((mode == NotLazy) || (_GetPxl(x, y) != _clearPixel)) {
    _ClearSquare(x, y);
    _ClearPxl(x, y, _clearPixel);
    // now put back the status of the pixel 
    RedrawGrid(x, y, x, y);
    if (WithinMark(x, y)) _HighlightSquare(x, y);
    if (IsHotSpot(x, y)) RedrawHotSpot();
  }
}


//-------------------------------------------------------------------------

void OXPaintCanvas::_FloodLoop(int x, int y, Pixel value, Pixel toflood) {
  int save_x, save_y, x_left, x_right;

  if (value == toflood) return; // nothing to flood in this case
  
  if (QueryFlood(x, y, toflood)) Flood(x, y, value);

  save_x = x;
  save_y = y;

  x++;
  while (QueryFlood(x, y, toflood)) {
    Flood(x, y, value);
    x++;
  }
  x_right = --x;
  
  x = save_x;
  x--;
  while (QueryFlood(x, y, toflood)) {
    Flood(x, y, value);
    x--;
  }
  x_left = ++x;
  
  
  x = x_left;
  y = save_y;
  y++;
  
  while (x <= x_right) {
    int flag = False;
    int x_enter;

    while (QueryFlood(x, y, toflood) && (x <= x_right)) {
      flag = True;
      x++;
    }
    
    if (flag) {
      if ((x == x_right) && QueryFlood(x, y, toflood))
	_FloodLoop(x, y, value, toflood);
      else
	_FloodLoop(x - 1, y, value, toflood);
    }
    
    x_enter = x;
    
    while (!QueryFlood(x, y, toflood) && (x < x_right)) x++;
    
    if (x == x_enter) x++;
  }

  x = x_left;
  y = save_y;
  y--;
  
  while (x <= x_right) {
    int flag = False;
    int x_enter;
    
    while (QueryFlood(x, y, toflood) && (x <= x_right)) {
      flag = True;
      x++;
    }
    
    if (flag) {
      if ((x == x_right) && QueryFlood(x, y, toflood))
	_FloodLoop(x, y, value, toflood);
      else
	_FloodLoop(x - 1, y, value, toflood);
    }
    
    x_enter = x;
    
    while (!QueryFlood(x, y, toflood) && (x < x_right)) x++;
    
    if (x == x_enter) x++;
  }
}

void OXPaintCanvas::DrawImageData(XImage *image, XImage *mask_image,
                       int at_x, int at_y, int value, DrawingMode mode) {
  int   x, y;
  Pixel A, B;
  register Pixel Cl, Fg, val;
  
  Cl = _clearPixel;
  Fg = _fgndPixel;
  
  for (x=0; x<image->width; x++) 
    for (y=0; y<image->height; y++) {
      if (!QueryInPixmap(at_x + x, at_y + y)) break;
      A = _GetPxlFromImageAndMask(image, mask_image, x, y);
      B = _GetPxl(at_x + x, at_y + y);
      switch (value) {
	case CLEAR:
	  if ((mode == NotLazy) || (B != Cl))
            val = Cl;
	  else
            val = (Pixel) NOT_SET;
	  break;

	case SET:
	  if ((mode == NotLazy) || (B != A))
            val = A;
	  else
            val = (Pixel) NOT_SET;
	  break;

	case INVERT:
	  if (((mode == NotLazy) && (A != Cl)) || ((A != Cl) && (B != Cl)))
	    val = Cl;
	  else if (((mode == NotLazy) && (A == Cl)) || 
		   ((A == Cl) && (B != Fg)))
            val = Fg;
	  else
            val = (Pixel) NOT_SET;
	  break;

	case HIGHLIGHT:
	  if (A != B) DrawPoint(at_x + x, at_y + y, A);
	  break;

	}

      if ((val != NOT_SET) && (val != HIGHLIGHT))
	_DrawPoint(at_x + x, at_y + y, val, mode);
      else if (value == HIGHLIGHT)
        _HighlightSquare(x, y);
	
    }
}

typedef struct {
  int *x, *y, *width, *height;
} Table;

XImage *OXPaintCanvas::ScalePixmapImage(XImage *src, 
                         double scale_x, double scale_y) {
  XImage *dst;
  Table table;    
  int x, y, width, height, w, h;
  Pixel pixel;

  width = max(roundint(scale_x * src->width), 1);
  height = max(roundint(scale_y * src->height), 1);

  dst = CreatePixmapImage(width, height);
  
  // It would be nice to check if width or height < 1.0 and
  // average the skipped pixels. But, it is slow as it is now.

  if (scale_x == 1.0 && scale_y == 1.0) {
    CopyImageData(src, dst, 0, 0, width-1 , height-1, 0, 0);
  } else {
    table.x = new int [src->width];
    table.y = new int [src->height];
    table.width = new int [src->width];
    table.height = new int [src->height];
    
    for (x=0; x<src->width; x++) {
      table.x[x] = roundint(scale_x * x);
      table.width[x] = roundint(scale_x * (x + 1)) - roundint(scale_x * x);
    }

    for (y=0; y<src->height; y++) {
      table.y[y] = roundint(scale_y * y);
      table.height[y] = roundint(scale_y * (y + 1)) - roundint(scale_y * y);
    }
    
    for (x=0; x<src->width; x++)
      for (y=0; y<src->height; y++) {
	pixel = XGetPixel(src, x, y);
	for (w=0; w<table.width[x]; w++)
	  for (h=0; h<table.height[y]; h++)
	    if (pixel != _clearPixel)
              XPutPixel(dst, table.x[x] + w, table.y[y] + h, pixel);
      }
    
    delete[] table.x;
    delete[] table.y;
    delete[] table.width;
    delete[] table.height;
  }
  
  return dst;
}

XImage *OXPaintCanvas::ScaleMaskImage(XImage *src,
                             double scale_x, double scale_y) {
  XImage *dst;
  Table table;    
  int x, y, width, height, w, h;
  Pixel pixel;
  
  width = max(roundint(scale_x * src->width), 1);
  height = max(roundint(scale_y * src->height), 1);
  
  dst = CreateMaskImage(width, height);
  
  // It would be nice to check if width or height < 1.0 and
  // average the skipped pixels. But, it is slow as it is now.

  if (scale_x == 1.0 && scale_y == 1.0) {
    CopyImageData(src, dst, 0, 0, width-1 , height-1, 0, 0);
  } else {
    table.x = new int [src->width];
    table.y = new int [src->height];
    table.width = new int [src->width];
    table.height = new int [src->height];
    
    for (x=0; x<src->width; x++) {
      table.x[x] = roundint(scale_x * x);
      table.width[x] = roundint(scale_x * (x + 1)) - roundint(scale_x * x);
    }

    for (y=0; y<src->height; y++) {
      table.y[y] = roundint(scale_y * y);
      table.height[y] = roundint(scale_y * (y + 1)) - roundint(scale_y * y);
    }
    
    for (x=0; x<src->width; x++)
      for (y=0; y<src->height; y++) {
	pixel = XGetPixel(src, x, y);
	for (w=0; w<table.width[x]; w++)
	  for (h=0; h<table.height[y]; h++)
	    if (pixel == 0)
              XPutPixel(dst, table.x[x] + w, table.y[y] + h, pixel);
      }
    
    delete[] table.x;
    delete[] table.y;
    delete[] table.width;
    delete[] table.height;
  }

  return dst;
}


//-------------------------------------------------------------------------

// Interface functions

void OXPaintCanvas::RedrawHotSpot() {
  if (QuerySet(_hot.x, _hot.y)) _DrawHotSpot(_hot.x, _hot.y);
}

void OXPaintCanvas::ClearHotSpot() {
  _DrawHotSpot(_hot.x, _hot.y);
  _hot.x = _hot.y = NOT_SET;
}

void OXPaintCanvas::DrawHotSpot(int x, int y, int value) {
  if (QueryInPixmap(x, y)) {
    if (QuerySet(_hot.x, _hot.y) && ((_hot.x == x) && (_hot.y == y))) {
      if ((value == CLEAR) || (value == INVERT)) {
	ClearHotSpot();
      }
    } else if ((value == SET) || (value == INVERT)) {
      ClearHotSpot();
      _DrawHotSpot(x, y);
      _hot.x = x;
      _hot.y = y;
    }
	
    if (value == HIGHLIGHT) _HighlightHotSpot(x, y); 
  }
}

void OXPaintCanvas::SetHotSpot(int x, int y) {
  if (QuerySet(x, y))
    DrawHotSpot(x, y, SET);
  else
    ClearHotSpot();
}

void OXPaintCanvas::DrawPoint(int x, int y, int value) {
  if (QueryInPixmap(x, y)) {
    if (value == HIGHLIGHT) {
      _HighlightSquare(x, y);
    } else if ((value == CLEAR) || 
            ((value == INVERT) && 
             (_GetPxl(x, y) != _clearPixel))) {
      _ClearPoint(x, y, Lazy);
    } else { // value == SET || (value == INVERT && Pxl == clearPixel)
      _DrawPoint(x, y, _fgndPixel, Lazy);
    }
  }
}

void OXPaintCanvas::RedrawSquares(int from_x, int from_y,
                                  int to_x, int to_y) {
  register int x, y;

  QuerySwap(from_x, to_x);
  QuerySwap(from_y, to_y);
  from_x = max(0, from_x);
  from_y = max(0, from_y);
  to_x = min(_image->width - 1, to_x);
  to_y = min(_image->height - 1, to_y);
  
  if (_squareSize == 1) {
    XPutImage(GetDisplay(), _id, _drawingGC, _image, from_x, from_y,
                            InWindowX(from_x), InWindowY(from_y),
                            to_x - from_x + 1, to_y - from_y + 1);
    // create mask pixmap
    Pixmap mask = XCreatePixmap(GetDisplay(), _id,
                                _mask_image->width, _mask_image->height,
                                _mask_image->depth);

    XGCValues gcv;
    GC gc;

    gcv.function = GXcopyInverted;
    gc = XCreateGC(GetDisplay(), mask, GCFunction, &gcv);

    XPutImage(GetDisplay(), mask, gc, _mask_image, 0, 0, 1, 1,
              _mask_image->width, _mask_image->height);
    XSetClipMask(GetDisplay(), _transparentGC, mask);
    XFillRectangle(GetDisplay(), _id, _transparentGC,
                   InWindowX(from_x), InWindowY(from_y),
                   to_x - from_x + 1, to_y - from_y + 1);
    XSetClipMask(GetDisplay(), _transparentGC, None);

    XFreePixmap(GetDisplay(), mask);
    XFreeGC(GetDisplay(), gc);

  } else {
    for (x=from_x; x<=to_x; x++)
      for (y=from_y; y<=to_y; y++)
        _DrawSquare(x, y, _GetPxl(x, y));
  }
}

void OXPaintCanvas::RedrawPoints(int from_x, int from_y, 
		                 int to_x, int to_y) {
  register int x, y;
  
  QuerySwap(from_x, to_x);
  QuerySwap(from_y, to_y);
  from_x = max(0, from_x);
  from_y = max(0, from_y);
  to_x = min(_image->width - 1, to_x);
  to_y = min(_image->height - 1, to_y);
  
  for (x=from_x; x<=to_x; x++)
    for (y=from_y; y<=to_y; y++)
      _DrawPoint(x, y, _GetPxl(x, y), NotLazy);
}

void OXPaintCanvas::DrawGrid(int from_x, int from_y, 
		             int to_x, int to_y) {
  int i;

  QuerySwap(from_x, to_x);
  QuerySwap(from_y, to_y);
  from_x = max(0, from_x);
  from_y = max(0, from_y);
  to_x = min(_image->width - 1, to_x);
  to_y = min(_image->height - 1, to_y);

  for (i=from_x+(from_x == 0); i<=to_x; i++)
    XDrawLine(GetDisplay(), _id, _framingGC,
	      InWindowX(i), InWindowY(from_y),
	      InWindowX(i), InWindowY(to_y + 1));

  for (i=from_y+(from_y == 0); i<=to_y; i++)
    XDrawLine(GetDisplay(), _id, _framingGC,
	      InWindowX(from_x), InWindowY(i),
	      InWindowX(to_x + 1), InWindowY(i));
}


void OXPaintCanvas::RedrawGrid(int from_x, int from_y, 
		               int to_x, int to_y) {
  if (_grid && _squareSize >= _gridTolerance)
    DrawGrid(from_x, from_y, to_x, to_y);
}

void OXPaintCanvas::DrawLine(int from_x, int from_y, 
		             int to_x, int to_y, int value) {
  int i, e, dx, dy, sx, sy, x, y, xchg;

  x = from_x;
  y = from_y;
  dx = abs(to_x - from_x); sx = sign(to_x - from_x);
  dy = abs(to_y - from_y); sy = sign(to_y - from_y);
  if (dy > dx) {
    Swap(dx, dy);
    xchg = True;
  } else {
    xchg = False;
  }
  e = dy + dy - dx;
  for (i=0; i<=dx; ++i) {
    DrawPoint(x, y, value);
    if (e >= 0) {
      if (xchg) x += sx; else y += sy;
      e -= dx + dx;
    }
    if (xchg) y += sy; else x += sx;
    e += dy + dy;
  }
}

void OXPaintCanvas::DrawRectangle(int from_x, int from_y, 
		                  int to_x, int to_y, int value) {
  register int i;
  int delta, width, height;
  
  QuerySwap(from_x, to_x);
  QuerySwap(from_y, to_y);
  
  width = to_x - from_x;
  height = to_y - from_y;
  
  delta = max(width, height);
  
  if (!QueryZero(width, height)) {
    for (i=0; i<delta; i++) {
      if (i < width) {
	DrawPoint(from_x + i, from_y, value);
	DrawPoint(to_x - i, to_y, value);
      }
      if (i < height) {
	DrawPoint(from_x, to_y - i, value);
	DrawPoint(to_x, from_y + i, value);
      }
    }
  } else {
    DrawLine(from_x, from_y, to_x, to_y, value);
  }
}

void OXPaintCanvas::DrawFilledRectangle(int from_x, int from_y, 
			                int to_x, int to_y, int value) {
  register int x, y;
  
  QuerySwap(from_x, to_x);
  QuerySwap(from_y, to_y);
  
  for (x=from_x; x<=to_x; x++)
    for (y=from_y; y<=to_y; y++)
      DrawPoint(x, y, value);
}

void OXPaintCanvas::DrawCircle(int origin_x, int origin_y, 
		               int point_x, int point_y, int value) {
  int x, y, dx, dy, r, r2, x2, y2;
  double radius;

  dx = abs(point_x - origin_x);
  dy = abs(point_y - origin_y);
  radius = sqrt((double) (dx * dx + dy * dy));
  if (radius < 1.0) {
    DrawPoint(origin_x, origin_y, value);
  } else {
    r = (int) radius;
    r2 = r * r;
    x = r;
    x2 = r2 - x;
    y2 = y = 0;
    while (x+1 > y) {
      DrawPoint(origin_x+x, origin_y+y, value);
      DrawPoint(origin_x-x, origin_y+y, value);
      if (y) DrawPoint(origin_x+x, origin_y-y, value);
      if (y) DrawPoint(origin_x-x, origin_y-y, value);
      if (x != y) DrawPoint(origin_x+y, origin_y+x, value);
      if (y && (x != y)) DrawPoint(origin_x-y, origin_y+x, value);
      if (x != y) DrawPoint(origin_x+y, origin_y-x, value);
      if (x != y) if (y) DrawPoint(origin_x-y, origin_y-x, value);
      y2 += y + y + 1;
      ++y;
      if (x2 > r2 - y2) {
        x2 -= x + x - 1;
        --x;
      }
    }
  }
}

void OXPaintCanvas::DrawFilledCircle(int origin_x, int origin_y, 
			             int point_x, int point_y, int value) {
  int x, y, dx, dy, r, r2, x2, y2;
  double radius;

  dx = abs(point_x - origin_x);
  dy = abs(point_y - origin_y);
  radius = sqrt((double) (dx * dx + dy * dy));
  if (radius < 1.0) {
    DrawPoint(origin_x, origin_y, value);
  } else {
    r = (int) radius;
    r2 = r * r;
    x = r;
    x2 = r2 - x;
    y2 = y = 0;
    while (x+1 > y) {
      DrawLine(origin_x+x, origin_y+y, origin_x-x, origin_y+y, value);
      DrawLine(origin_x+x, origin_y-y, origin_x-x, origin_y-y, value);
      DrawLine(origin_x+y, origin_y+x, origin_x-y, origin_y+x, value);
      DrawLine(origin_x+y, origin_y-x, origin_x-y, origin_y-x, value);
      y2 += y + y + 1;
      ++y;
      if (x2 > r2 - y2) {
        x2 -= x + x - 1;
        --x;
      }
    }
  }
}

void OXPaintCanvas::FloodFill(int x, int y, int value) {
  Pixel pixel, foreground = _fgndPixel;
  Pixel clear = _clearPixel;
  
  pixel = _GetPxl(x, y);
  
  if (value == INVERT)
    _FloodLoop(x, y, ((pixel != clear) ? clear : foreground), pixel);
  else if (value == CLEAR)
    _FloodLoop(x, y, clear, pixel);
  else
    _FloodLoop(x, y, foreground, pixel); 
}

void OXPaintCanvas::ShiftUp() {
  register int x, y;
  Pixel first, up, down;
  int from_x, from_y, to_x, to_y;
  
  if (QueryMarked()) {
    from_x = _mark.from_x;
    from_y = _mark.from_y;
    to_x = _mark.to_x;
    to_y = _mark.to_y;
  } else {
    from_x = 0;
    from_y = 0;
    to_x = _width - 1;
    to_y = _height - 1;
  }
  
  if ((to_y - from_y) == 0) return;

  for (x=from_x; x<=to_x; x++) {
    first = up = _GetPxl(x, to_y);
    for (y=to_y-1; y>=from_y; y--) {
      down = _GetPxl(x, y);
      if (up != down) _DrawPoint(x, y, up, Lazy);
      up = down;
    }
    if (first != down) _DrawPoint(x, to_y, down, Lazy);
  }
  
  if (!QueryMarked()) {
    if (QuerySet(_hot.x, _hot.y)) {
      SetHotSpot(_hot.x, (_hot.y - 1 + _image->height) % _image->height);
    }
  }

  _changed = True;
}

void OXPaintCanvas::ShiftDown() {
  register int x, y;
  Pixel first, down, up;
  int from_x, from_y, to_x, to_y;
  
  if (QueryMarked()) {
    from_x = _mark.from_x;
    from_y = _mark.from_y;
    to_x = _mark.to_x;
    to_y = _mark.to_y;
  } else {
    from_x = 0;
    from_y = 0;
    to_x = _width - 1;
    to_y = _height - 1;
  }
  
  if ((to_y - from_y) == 0) return;
  
  for (x=from_x; x<=to_x; x++) {
    first = down = _GetPxl(x, from_y);
    for (y=from_y+1; y<=to_y; y++) {
      up = _GetPxl(x, y);
      if (down != up) _DrawPoint(x, y, down, Lazy);
      down = up;
    }
    if (first != up) _DrawPoint(x, from_y, up, Lazy);
  }
  
  if (!QueryMarked()) {
    if (QuerySet(_hot.x, _hot.y)) {
      SetHotSpot(_hot.x, (_hot.y + 1) % _image->height);
    }
  }

  _changed = True;
}


void OXPaintCanvas::ShiftLeft() {
  register int x, y;
  Pixel first, left, right;
  int from_x, from_y, to_x, to_y;
  
  if (QueryMarked()) {
    from_x = _mark.from_x;
    from_y = _mark.from_y;
    to_x = _mark.to_x;
    to_y = _mark.to_y;
  } else {
    from_x = 0;
    from_y = 0;
    to_x = _width - 1;
    to_y = _height - 1;
  }
  
  if ((to_x - from_x) == 0) return;
  
  for (y=from_y; y<=to_y; y++) {
    first = left = _GetPxl(to_x, y);
    for (x=to_x-1; x>=from_x; x--) {
      right = _GetPxl(x, y);
      if (left != right) _DrawPoint(x, y, left, Lazy);
      left = right;
    }
    if (first != right)	_DrawPoint(to_x, y, right, Lazy);
  }

  if (!QueryMarked()) {
    if (QuerySet(_hot.x, _hot.y)) {
      SetHotSpot((_hot.x - 1 + _image->width) % _image->width, _hot.y);
    }
  }

  _changed = True;
}

void OXPaintCanvas::ShiftRight() {
  register int x, y;
  Pixel first, right, left;
  int from_x, from_y, to_x, to_y;
  
  if (QueryMarked()) {
    from_x = _mark.from_x;
    from_y = _mark.from_y;
    to_x = _mark.to_x;
    to_y = _mark.to_y;
  } else {
    from_x = 0;
    from_y = 0;
    to_x = _width - 1;
    to_y = _height - 1;
  }
  
  if ((to_x - from_x) == 0) return;
  
  for (y=from_y; y<=to_y; y++) {
    first = right = _GetPxl(from_x, y);
    for (x=from_x+1; x<=to_x; x++) {
      left = _GetPxl(x, y);
      if (right != left) _DrawPoint(x, y, right, Lazy);
      right = left;
    }
    if (first != left) _DrawPoint(from_x, y, left, Lazy);
  }

  if (!QueryMarked()) {
    if (QuerySet(_hot.x, _hot.y)) {
      SetHotSpot((_hot.x + 1) % _image->width, _hot.y);
    }
  }

  _changed = True;
}

void OXPaintCanvas::RedrawMark() {
  if (QuerySet(_mark.from_x, _mark.from_y))
    XFillRectangle(GetDisplay(), _id, _highlightingGC,
		   InWindowX(_mark.from_x), InWindowY(_mark.from_y), 
		   InWindowX(_mark.to_x + 1) - InWindowX(_mark.from_x),
		   InWindowY(_mark.to_y + 1) - InWindowY(_mark.from_y));
}

void OXPaintCanvas::Fold() {
  int x, y, new_x, new_y, horiz, vert;
  XImage *storage, *mask_storage;
  Pixel color;
  
  storage = CreatePixmapImage(_image->width, _image->height);
  mask_storage = CreateMaskImage(_mask_image->width, _mask_image->height);

  TransferImageData(_image, storage);
  TransferImageData(_mask_image, mask_storage);
  
  _fold ^= True;
  horiz = (_image->width + _fold) / 2;
  vert = (_image->height + _fold) / 2;
  
  for (x=0; x<_image->width; x++)
    for (y=0; y<_image->height; y++) {
      new_x = (x + horiz) % _image->width;
      new_y = (y + vert) % _image->height;
      if (_GetPxl(new_x, new_y) != 
	  (color = _GetPxlFromImageAndMask(storage, mask_storage, x, y))) {
	  _DrawPoint(new_x, new_y, color, Lazy);
      }
    }

  DestroyPixmapImage(&storage);
  DestroyMaskImage(&mask_storage);

  if (QuerySet(_hot.x, _hot.y))
    SetHotSpot((_hot.x + horiz) % _image->width,
	       (_hot.y + vert) % _image->height);

  _changed = True;
}

void OXPaintCanvas::ClearAll() {
  register int x, y, from_x, from_y, to_x, to_y;

  if (QueryMarked()) {
    from_x = _mark.from_x;
    from_y = _mark.from_y;
    to_x = _mark.to_x;
    to_y = _mark.to_y;
  } else {
    from_x = 0;
    from_y = 0;
    to_x = _image->width - 1;
    to_y = _image->height - 1;
  }

  for (x=from_x; x<=to_x; x++)
    for (y=from_y; y<=to_y; y++)
      _ClearPoint(x, y, Lazy);
/* This way is too flashy
      ClearPxl(_image, x, y, _clearPixel);
  XClearArea(GetDisplay(), _id, 0, 0, _w, _h, True);
*/
}

void OXPaintCanvas::SetAll() {
  register int x, y, from_x, from_y, to_x, to_y;

  if (QueryMarked()) {
    from_x = _mark.from_x;
    from_y = _mark.from_y;
    to_x = _mark.to_x;
    to_y = _mark.to_y;
  } else {
    from_x = 0;
    from_y = 0;
    to_x = _image->width - 1;
    to_y = _image->height - 1;
  }

  for (x=from_x; x<=to_x; x++)
    for (y=from_y; y<=to_y; y++)
      _DrawPoint(x, y, _fgndPixel, Lazy);
}


// Translate a point by dx, dy wrapping around if the point goes off
// the pixmap

void OXPaintCanvas::TranslatePoint(int *x, int *y, int dx, int dy) {
  *x = (*x + dx + _image->width)  % _image->width;
  *y = (*y + dy + _image->height) % _image->height;
}


// Flip a given point guaranteeing that the flipped point will be
// in the pixmap.

void OXPaintCanvas::FlipPoint(int *x, int *y, FlipAxis axis) {
  if (axis == Vertical)
    *x = _image->width  - 1 - *x;
  else 
    *y = _image->height - 1 - *y;
}

// Rotate a single point left or right 90 degrees. Guarantee that
// result is in pixmap.

void OXPaintCanvas::RotatePoint(int *x, int *y, RotateDirection direction) {
  int from_x, from_y, to_x, to_y;
  int half_width, half_height, shift;
  int tmp;

  if (QueryMarked()) {
    from_x = _mark.from_x;
    from_y = _mark.from_y;
    to_x   = _mark.to_x;
    to_y   = _mark.to_y;
  } else {
    from_x = 0;
    from_y = 0;
    to_x   = _width - 1;
    to_y   = _height - 1;
  }

  half_width  = (int) floor((to_x - from_x) / 2.0 + 0.5);
  half_height = (int) floor((to_y - from_y ) / 2.0 + 0.5);
  shift       = min((int)(to_x - from_x), (int)(to_y - from_y )) % 2;

  *x -= half_width;
  *y -= half_height;
  if (*x >= 0) *x += shift;
  if (*y >= 0) *y += shift;
  tmp = *x;

  if (direction == Right) {
    *x = - *y;
    *y = tmp;
  } else {
    *x = *y;
    *y = - tmp;
  }

  if (*x > 0) *x -= shift;
  if (*y > 0) *y -= shift;

  *x += half_width;
  *y += half_height;
}


void OXPaintCanvas::FlipHoriz() {
  register int x, y;
  int from_x, from_y, to_x, to_y;
  float half;
  Pixel color1, color2;
  
  if (QueryMarked()) {
    from_x = _mark.from_x;
    from_y = _mark.from_y;
    to_x   = _mark.to_x;
    to_y   = _mark.to_y;
  } else {
    from_x = 0;
    from_y = 0;
    to_x   = _width - 1;
    to_y   = _height - 1;
  }

  half = (float) (to_y - from_y) / 2.0 + 0.5;

  if (half == 0.0) return;

  for (x=from_x; x<=to_x; x++) 
    for (y=0; y<half; y++)
      if ((color1 = _GetPxl(x, from_y + y)) != 
	  (color2 = _GetPxl(x, to_y - y))) {
	_DrawPoint(x, from_y + y, color2, Lazy);
	_DrawPoint(x, to_y - y, color1, Lazy);
      }

  if (!QueryMarked()) {
    if (QuerySet(_hot.x, _hot.y)) {
      int hx = _hot.x;
      int hy = _hot.y;
      FlipPoint(&hx, &hy, Horizontal); 
      SetHotSpot(hx, hy);
    }
  }

  _changed = True;
}

void OXPaintCanvas::FlipVert() {
  register int x, y;
  int from_x, from_y, to_x, to_y;
  float half;
  Pixel color1, color2;

  if (QueryMarked()) {
    from_x = _mark.from_x;
    from_y = _mark.from_y;
    to_x = _mark.to_x;
    to_y = _mark.to_y;
  } else {
    from_x = 0;
    from_y = 0;
    to_x = _width - 1;
    to_y = _height - 1;
  }

  half = (float) (to_x - from_x) / 2.0 + 0.5;

  if (half == 0) return;
  
  for (y=from_y; y<=to_y; y++)
    for (x=0; x<half; x++)
      if ((color1 = _GetPxl(from_x + x, y)) != 
	  (color2 = _GetPxl(to_x - x, y))) {
	_DrawPoint(from_x + x, y, color2, Lazy);
	_DrawPoint(to_x - x, y, color1, Lazy);
      }

  if (!QueryMarked()) {
    if (QuerySet(_hot.x, _hot.y)) {
      int hx = _hot.x;
      int hy = _hot.y;
      FlipPoint(&hx, &hy, Vertical); 
      SetHotSpot(hx, hy);
    }
  }

  _changed = True;
}

void OXPaintCanvas::RotateRight() {
  int x, y, delta, shift, from_x, from_y, to_x, to_y;
  int half_width, half_height;
  Pixel quad1, quad2, quad3, quad4;
  
  if (QueryMarked()) {
    from_x = _mark.from_x;
    from_y = _mark.from_y;
    to_x = _mark.to_x;
    to_y = _mark.to_y;
  } else {
    from_x = 0;
    from_y = 0;
    to_x = _width - 1;
    to_y = _height - 1;
  }
  
  half_width = (int) floor((to_x - from_x) / 2.0 + 0.5);
  half_height = (int) floor((to_y - from_y ) / 2.0 + 0.5);
  shift = min(to_x - from_x, to_y - from_y) % 2;
  delta = min(half_width, half_height) - shift;

  for (x=0; x<=delta; x++) {
    for (y=1-shift; y<=delta; y++) {
      quad1 = _GetPxl(from_x + half_width + x, 
		      from_y + half_height + y);
      quad2 = _GetPxl(from_x + half_width + y, 
		      from_y + half_height - shift - x);
      quad3 = _GetPxl(from_x + half_width - shift - x, 
		      from_y + half_height - shift - y);
      quad4 = _GetPxl(from_x + half_width - shift - y, 
		      from_y + half_height + x);

      if (quad1 != quad2)
        _DrawPoint(from_x + half_width + x, 
		   from_y + half_height + y, quad2, Lazy);

      if (quad2 != quad3)
        _DrawPoint(from_x + half_width + y, 
		   from_y + half_height - shift - x, quad3, Lazy);

      if (quad3 != quad4)
        _DrawPoint(from_x + half_width - shift - x,
	           from_y + half_height - shift - y, quad4, Lazy);

      if (quad4 != quad1)
        _DrawPoint(from_x + half_width - shift - y, 
		   from_y + half_height + x, quad1, Lazy);
    }
  }
  
  if (!QueryMarked()) {
    if (QuerySet(_hot.x, _hot.y)) {
      int hx = _hot.x;
      int hy = _hot.y;
      RotatePoint(&hx, &hy, Right);
      if (QueryInPixmap(hx, hy)) SetHotSpot(hx, hy);
    }
  }

  _changed = True;
}

void OXPaintCanvas::RotateLeft() {
  int x, y,delta, shift, from_x, from_y, to_x, to_y;
  int half_width, half_height;
  Pixel quad1, quad2, quad3, quad4;
  
  if (QueryMarked()) {
    from_x = _mark.from_x;
    from_y = _mark.from_y;
    to_x = _mark.to_x;
    to_y = _mark.to_y;
  } else {
    from_x = 0;
    from_y = 0;
    to_x = _width - 1;
    to_y = _height - 1;
  }

  half_width = (int) floor((to_x - from_x) / 2.0 + 0.5);
  half_height = (int) floor((to_y - from_y ) / 2.0 + 0.5);
  shift = min(to_x - from_x, to_y - from_y) % 2;
  delta = min(half_width, half_height) - shift;
  
  for (x=0; x<=delta; x++) {
    for (y=1-shift; y<=delta; y++) {
      quad1 = _GetPxl(from_x + half_width + x, 
		      from_y + half_height + y);
      quad2 = _GetPxl(from_x + half_width + y, 
		      from_y + half_height - shift - x);
      quad3 = _GetPxl(from_x + half_width - shift - x, 
		      from_y + half_height - shift - y);
      quad4 = _GetPxl(from_x + half_width - shift - y, 
		      from_y + half_height + x);

      if (quad1 != quad4)
	_DrawPoint(from_x + half_width + x, 
		   from_y + half_height + y, quad4, Lazy);

      if (quad2 != quad1)
	_DrawPoint(from_x + half_width + y, 
		   from_y + half_height - shift - x, quad1, Lazy);

      if (quad3 != quad2)
	_DrawPoint(from_x + half_width - shift - x,
		   from_y + half_height - shift - y, quad2, Lazy);

      if (quad4 != quad3)
	_DrawPoint(from_x + half_width - shift - y, 
		   from_y + half_height + x, quad3, Lazy);

    }
  }

  if (!QueryMarked()) {
    if (QuerySet(_hot.x, _hot.y)) {
      int hx = _hot.x;
      int hy = _hot.y;
      RotatePoint(&hx, &hy, Left); 
      if (QueryInPixmap(hx, hy)) SetHotSpot(hx, hy);
    }
  }

  _changed = True;
}


void OXPaintCanvas::Store() {
  int width, height;

  if (QuerySet(_mark.from_x, _mark.from_y)) {

    DestroyPixmapImage(&_storage);
    DestroyMaskImage(&_mask_storage);

    width = _mark.to_x - _mark.from_x + 1;
    height = _mark.to_y - _mark.from_y + 1;

    _storage = CreatePixmapImage(width, height);
    _mask_storage = CreateMaskImage(width, height);

    CopyImageData(_image, _storage,
		  _mark.from_x,  _mark.from_y,
		  _mark.to_x,  _mark.to_y,
		  0, 0);

    CopyImageData(_mask_image, _mask_storage,
		  _mark.from_x,  _mark.from_y,
		  _mark.to_x,  _mark.to_y,
		  0, 0);
  }
}

void OXPaintCanvas::ClearMarked() {
  if (QuerySet(_mark.from_x, _mark.from_y)) {
    int from_x = _mark.from_x, 
        from_y = _mark.from_y, to_x = _mark.to_x, 
        to_y = _mark.to_y;
    int x, y;

    QuerySwap(from_x, to_x);
    QuerySwap(from_y, to_y);
      
    for (x=from_x; x<=to_x; x++)
      for (y=from_y; y<=to_y; y++)
        _ClearPoint(x, y, Lazy);
  }
}


void OXPaintCanvas::DragMarked(int at_x, int at_y) {
  if (QuerySet(_mark.from_x, _mark.from_y))
    DrawRectangle(at_x, at_y, 
		  at_x + _mark.to_x - _mark.from_x,
		  at_y + _mark.to_y - _mark.from_y,
		  HIGHLIGHT);
}

void OXPaintCanvas::DragStored(int at_x, int at_y) {
  if (_storage)
    DrawRectangle(at_x, at_y,
		  at_x + _storage->width - 1,
		  at_y + _storage->height - 1,
		  HIGHLIGHT);
}

void OXPaintCanvas::Restore(int at_x, int at_y, int value) {
  if (_storage)
    DrawImageData(_storage, _mask_storage, at_x, at_y, value, Lazy);
}

void OXPaintCanvas::CopyRegion(int at_x, int at_y, int value) {
  XImage *storage, *mask_storage;
  int width, height;
  
  if (QuerySet(_mark.from_x, _mark.from_y)) {
    
    width = _mark.to_x - _mark.from_x + 1;
    height = _mark.to_y - _mark.from_y + 1;
    
    storage = CreatePixmapImage(width, height);
    mask_storage = CreateMaskImage(width, height);
    
    CopyImageData(_image, storage,
		  _mark.from_x,  _mark.from_y,
		  _mark.to_x,  _mark.to_y,
		  0, 0);

    CopyImageData(_mask_image, mask_storage,
		  _mark.from_x,  _mark.from_y,
		  _mark.to_x,  _mark.to_y,
		  0, 0);
    
    DrawImageData(storage, mask_storage, at_x, at_y, value, NotLazy);
    
    Mark(at_x, at_y,
	 at_x + _mark.to_x - _mark.from_x,
	 at_y + _mark.to_y - _mark.from_y); 

    DestroyPixmapImage(&storage);
    DestroyMaskImage(&mask_storage);
  }
}

void OXPaintCanvas::MoveRegion(int at_x, int at_y, int value) {
  XImage *storage, *mask_storage;
  int width, height;
  
  if (QuerySet(_mark.from_x, _mark.from_y)) {
    
    width = _mark.to_x - _mark.from_x + 1;
    height = _mark.to_y - _mark.from_y + 1;
    
    storage = CreatePixmapImage(width, height);
    mask_storage = CreateMaskImage(width, height);

    CopyImageData(_image, storage,
		  _mark.from_x,  _mark.from_y,
		  _mark.to_x,  _mark.to_y,
		  0, 0);
    
    CopyImageData(_mask_image, mask_storage,
		  _mark.from_x,  _mark.from_y,
		  _mark.to_x,  _mark.to_y,
		  0, 0);
    
    ClearMarked();

    DrawImageData(storage, mask_storage, at_x, at_y, value, NotLazy); 
    
    Mark(at_x, at_y,
	 at_x + _mark.to_x - _mark.from_x,
	 at_y + _mark.to_y - _mark.from_y);

    DestroyPixmapImage(&storage);
    DestroyMaskImage(&mask_storage);
  }
}

void OXPaintCanvas::StoreToBuffer() {

  // first check available space and resize if necessary
  if ((_image->width != _buffer->width) ||
      (_image->height != _buffer->height)) {
    XImage *buffer = CreatePixmapImage(_image->width, _image->height);
    XImage *mask_buffer = CreateMaskImage(_mask_image->width, 
					  _mask_image->height);
      
    DestroyPixmapImage(&_buffer);
    DestroyMaskImage(&_mask_buffer);
    _buffer = buffer;
    _mask_buffer = mask_buffer;
  }
  
  CopyImageData(_image, _buffer, 0, 0, 
		_image->width-1, _image->height-1, 0, 0);

  CopyImageData(_mask_image, _mask_buffer, 0, 0, 
		_mask_image->width-1, _mask_image->height-1, 0, 0);
  
  _buffer_hot  = _hot;
  _buffer_mark = _mark;

  // save extensions to undo buffer
  XpmFreeExtensions(_buffer_extensions, 
                    _buffer_nextensions);
  //CopyExtensions(&_buffer_extensions, 
  //               &_buffer_nextensions,
  //               _extensions, _nextensions);
}

void OXPaintCanvas::Unmark() {
  if (QuerySet(_mark.from_x, _mark.from_y)) {
    XFillRectangle(GetDisplay(), _id, _highlightingGC,
		   InWindowX(_mark.from_x), InWindowY(_mark.from_y), 
		   InWindowX(_mark.to_x + 1) - InWindowX(_mark.from_x),
		   InWindowY(_mark.to_y + 1) - InWindowY(_mark.from_y));
    
    _mark.from_x = _mark.from_y = NOT_SET;
    _mark.to_x = _mark.to_y = NOT_SET;
  }
}

void OXPaintCanvas::Mark(int from_x, int from_y, int to_x, int to_y) {
  
  Unmark();
  
  if (QuerySet(from_x, from_y)) {
    if ((from_x == to_x) && (from_y == to_y)) {
      /*
      _mark.from_x = 0;
      _mark.from_y = 0;
      _mark.to_x = _image->width - 1;
      _mark.to_y = _image->height - 1;
      */
      return;
    } else {
      QuerySwap(from_x, to_x);
      QuerySwap(from_y, to_y);
      from_x = max(0, from_x);
      from_y = max(0, from_y);
      to_x = min(_image->width - 1, to_x);
      to_y = min(_image->height - 1, to_y);
      
      _mark.from_x = from_x;
      _mark.from_y = from_y;
      _mark.to_x = to_x;
      _mark.to_y = to_y;
    }
    
    XFillRectangle(GetDisplay(), _id, _highlightingGC,
		   InWindowX(_mark.from_x), InWindowY(_mark.from_y), 
		   InWindowX(_mark.to_x + 1) - InWindowX(_mark.from_x),
		   InWindowY(_mark.to_y +1) - InWindowY(_mark.from_y));
  }
}

void OXPaintCanvas::Undo() {
  int x, y;
  int tmp_nextensions;
  XpmExtension *tmp_extensions;
  XPoint tmp_hot;
  XImage *tmp_image, *mask_tmp_image;
  OArea tmp_mark;
  Pixel color;

  tmp_image = _image;
  mask_tmp_image = _mask_image;
  _image = _buffer;
  _mask_image = _mask_buffer;
  _buffer = tmp_image;
  _mask_buffer = mask_tmp_image;

  tmp_hot = _hot;
  tmp_mark = _mark;
  _mark = _buffer_mark;
  _buffer_mark= tmp_mark;

  // erase extensions then save

  tmp_nextensions = _nextensions;
  tmp_extensions  = _extensions;
  _nextensions = _buffer_nextensions;
  _extensions  = _buffer_extensions;
  _buffer_nextensions = tmp_nextensions;
  _buffer_extensions  = tmp_extensions;

  if ((_image->width != _buffer->width) ||
      (_image->height != _buffer->height)) {
    _width = _image->width;
    _height = _image->height;

    Resize(_width * _squareSize + 2 * _distance,
           _height * _squareSize + 2 * _distance);

  } else {
    for (x=0; x<_image->width; x++)
      for (y=0; y<_image->height; y++)
        if (_GetPxlFromImageAndMask(_buffer, _mask_buffer, x, y) !=
            (color = _GetPxl(x, y))) {
          _DrawPoint(x, y, color, NotLazy);
        }
  
    // Now treating Undo copy seperatly:
    // when mark is set and is different from buffer one, redraw them
    if ((QuerySet(_mark.from_x, _mark.from_y)) &&
        (QuerySet(_buffer_mark.from_x, _buffer_mark.from_y))
	  && ((_mark.from_x != _buffer_mark.from_x) ||
	      (_mark.from_y != _buffer_mark.from_y) ||
	      (_mark.to_x != _buffer_mark.to_x) ||
	      (_mark.to_y != _buffer_mark.to_y))) {
      RedrawPoints(_mark.from_x, _mark.from_y, _mark.to_x, _mark.to_y);
      RedrawPoints(_buffer_mark.from_x, _buffer_mark.from_y, 
		   _buffer_mark.to_x, _buffer_mark.to_y);
    }
  }

  SetHotSpot(_buffer_hot.x, _buffer_hot.y);
  _buffer_hot = tmp_hot;

}

void OXPaintCanvas::RotateBufferAndImage() {
  XPoint tmp_hot;
  XImage *tmp_image, *mask_tmp_image;
  int set_hot_spot = False;

  if (QuerySet(_mark.from_x, _mark.from_y)) {
    // Rotate Image and marked area
    int width, height;
    XImage *storage, *mask_storage;

    width = _mark.to_x - _mark.from_x + 1;
    height = _mark.to_y - _mark.from_y + 1;
      
    storage = CreatePixmapImage(width, height);
    mask_storage = CreateMaskImage(width, height);
      
    CopyImageData(_image, storage,
                  _mark.from_x, _mark.from_y,
	          _mark.to_x,   _mark.to_y, 0, 0);
      
    CopyImageData(_mask_image, mask_storage,
		  _mark.from_x, _mark.from_y,
		  _mark.to_x,   _mark.to_y, 0, 0);

    tmp_image = _image;
    mask_tmp_image = _mask_image;
    tmp_hot = _hot;
    _image = storage;
    _mask_image = mask_storage;

    // reset all marked area as there are problems with undo
    _mark.from_x = _mark.to_x = NOT_SET;
    _mark.from_y = _mark.to_y = NOT_SET;
    _buffer_mark.from_x = _buffer_mark.to_x = NOT_SET;
    _buffer_mark.from_y = _buffer_mark.to_y = NOT_SET;

  } else {

    // Rotate Image and Cut/Insert buffer
    set_hot_spot = True;
    tmp_image = _image;
    mask_tmp_image = _mask_image;
    tmp_hot = _hot;
    _image = _storage;
    _mask_image = _mask_storage;
    _storage = tmp_image;
    _mask_storage = mask_tmp_image;

  }

  if ((_image->width != tmp_image->width) ||
      (_image->height != tmp_image->height)) {
    _width = _image->width;
    _height = _image->height;
    Resize(_width * _squareSize + 2 * _distance,
           _height * _squareSize + 2 * _distance);
  } else {
    //Redraw(); is just:
    XClearArea(GetDisplay(), _id, 0, 0, _w, _h, True);
  }

  if (set_hot_spot) {  // if storage taken from buffer
    SetHotSpot(_storage_hot.x, _storage_hot.y);
  } else {
    DestroyPixmapImage(&tmp_image);
    DestroyMaskImage(&mask_tmp_image);
  }

  _storage_hot = tmp_hot;
}


void OXPaintCanvas::SetDrawingForeground(Pixel color) {
  XSetForeground(GetDisplay(), _drawingGC, color);
  _fgndPixel = color;
}


// Drag a rectangle representing the extent of the text

void OXPaintCanvas::DragText(int at_x, int at_y, int value) {
/*
  int x, y, width, height;

  calculate text extent 
  translate from text origin to upper left of text extent 
  draw rectangle from upper left to lower right of text extent

  width  = _text_rbearing - _text_lbearing;
  height = _text_ascent + _text_descent;
  x = at_x + _text_lbearing;
  y = at_y - _text_ascent + 1;

  DrawRectangle(x, y, x + width, y + height, value);
*/

  DrawText(at_x, at_y, value);
}

void OXPaintCanvas::DrawText(int at_x, int at_y, int value) {
  Pixmap     pixmap;
  XImage     *image;
  GC         gc;
  XGCValues  gcv;
  int        x, y, i, j, width, height;

  if (!_text_string || ! _font_struct || strlen(_text_string) == 0)
    return;

  // To draw text: 
  // put text in tmp pixmap of depth 1 then get image 
  // foreach pixel in image, if set draw into pixmapwidget 

  // text extent, add 1 pixel pad because fonts vary is how ascent is defined
  width  = _text_rbearing - _text_lbearing + 2;
  height = _text_ascent + _text_descent + 2;

  // create white pixmap size of text extent with depth one
  pixmap = XCreatePixmap(GetDisplay(), _id, width, height, 1);
  gcv.font       = _font_struct->fid;
  gcv.function   = GXcopy;
  gcv.foreground = _whitePixel; //WhitePixel(dpy, _screen);
  gcv.background = _whitePixel; //WhitePixel(dpy, _screen);
  gc = XCreateGC(GetDisplay(), pixmap, 
                 GCFunction | GCFont | GCForeground | GCBackground, 
                 &gcv);

  // clear pixmap
  XFillRectangle(GetDisplay(), pixmap, gc, 0, 0, width, height);

  // draw text in pixmap
  x = (_text_lbearing < 0) ? - _text_lbearing : 0;
  y = _text_ascent /* - 1 */; // include pad
  XSetForeground(GetDisplay(), gc, _blackPixel /*BlackPixel(dpy, _screen)*/);
  XDrawString(GetDisplay(), pixmap, gc, x, y,
              _text_string, strlen(_text_string));

  // get image of text
  image = XGetImage(GetDisplay(), pixmap, 0, 0, width, height,
                    ~0 /*-1*/ /*AllPlanes*/, ZPixmap);

  // translate (at_x,at_y) from text origin to upper left of text extent
  x = at_x + _text_lbearing;
  y = at_y - _text_ascent + 1;

  // foreach set pixel in image, draw into pixmap widget
  for (i=0; i<width; i++) {
    for (j=0; j<height; j++) {
      if (!QueryInPixmap(x + i, y + j)) continue;
      if (XGetPixel(image, i, j) == _blackPixel/*BlackPixel(dpy, _screen)*/) {
        DrawPoint(x + i, y + j, value);
      }
    }
  }
  
  // clean up
  XFreePixmap(GetDisplay(), pixmap);
  XFreeGC(GetDisplay(), gc);
  XDestroyImage(image);
}


//-------------------------------------------------------------------------

//  Cut and Paste operations...

int OXPaintCanvas::HandleSelectionRequest(XSelectionRequestEvent *event) {
  Atom type;
  unsigned char *data = 0;
  unsigned long len = 0;
  int format;

  if (_ConvertSelection(event->selection, event->target,
                        &type, &data, &len, &format)) {
    if (event->property == None)  // obsolete client?
      event->property = event->target;

    XChangeProperty(GetDisplay(), event->requestor, event->property,
                    event->target, format, PropModeReplace, data, len);
  } else {
    event->property = None;
  }

  XEvent xevent;

  xevent.xselection.type      = SelectionNotify;
  xevent.xselection.property  = event->property;
  xevent.xselection.display   = event->display;
  xevent.xselection.requestor = event->requestor;
  xevent.xselection.selection = event->selection;
  xevent.xselection.target    = event->target;
  xevent.xselection.time      = event->time;     

  XSendEvent(GetDisplay(), event->requestor, False, NoEventMask, &xevent);

  if (data) delete data;

  return True;
}

int OXPaintCanvas::_ConvertSelection(Atom selection, Atom target,
                                Atom *type, unsigned char **value,
                                unsigned long *length, int *format) {
  Pixmap *pixmap;
  XImage *image;
  int width, height;
 
  switch (target) {
//    case XA_TARGETS:
//	*type = XA_ATOM;
//	*value = (XtPointer) pixmapClassRec.pixmap_class.targets;
//	*length = pixmapClassRec.pixmap_class.num_targets;
//	*format = 32;
//	return True;

    case XA_BITMAP:
    case XA_PIXMAP:
	if (!QueryMarked()) return False;
	width  = _mark.to_x - _mark.from_x + 1;
	height = _mark.to_y - _mark.from_y + 1;
	image = CreatePixmapImage(width, height);
	CopyImageData(_image, image, 
		      _mark.from_x, _mark.from_y,
		      _mark.to_x, _mark.to_y, 0, 0);
	pixmap = (Pixmap *) new unsigned char[sizeof(Pixmap)]; //(Pixmap *) XtMalloc(sizeof(Pixmap));
	*pixmap = GetPixmap(image);
	DestroyPixmapImage(&image);
	*type = XA_PIXMAP;
	*value = (unsigned char *) pixmap;
	*length = 1;
	*format = 32;
	return True;

    case XA_STRING:
	*type = XA_STRING;
	*value = (unsigned char *) StrDup("Hello world!\n");
	*length = strlen((char *) *value);
	*format = 8;
	return True;

    default:
	return False;
  }
}

int OXPaintCanvas::HandleSelectionClear(XSelectionClearEvent *event) {
  _selection.own = False;
  Unmark();
  return True;
}

// called on PropertyNotify after the requestor deleted the data

void OXPaintCanvas::_SelectionDone(Atom *selection, Atom *target) {
//  if (*target != XA_TARGETS)
//    XtFree(_value);
}

void OXPaintCanvas::_GrabSelection(Time time) {
  XSetSelectionOwner(GetDisplay(), XA_PRIMARY, _id, time);
  _selection.own = (XGetSelectionOwner(GetDisplay(), XA_PRIMARY) == _id);
}

// called by X server after XConvertSelection

int OXPaintCanvas::HandleSelection(XSelectionEvent *event) {
  Atom actual;
  int format;
  unsigned long count, remaining;
  unsigned char *data = 0;

  // get the type, value, length and format from event->property
  // the property is deleted after data is retrieved.

  if (XGetWindowProperty(GetDisplay(), _id, _XC_SELECTION_DATA, 0, 1L,
                         True, XA_PIXMAP, &actual, &format, &count,
                         &remaining, &data) == Success) {
    if (count > 0 && _PasteSelection(actual, data)) {
      _lastMode = _drawMode;
      SetMode(MOVE_REGION);
    }
  }

  if (data) XFree(data);
  _selection.limbo = False;

  return True;
}

int OXPaintCanvas::_PasteSelection(Atom type, unsigned char *value) {
  Pixmap *pixmap;

  switch (type) {
    case XA_BITMAP:
    case XA_PIXMAP:
	DestroyPixmapImage(&_storage);
	DestroyMaskImage(&_mask_storage);
	pixmap = (Pixmap *) value;
	_storage = GetImage(*pixmap);
	_mask_storage = CreateMaskImage(_mask_image->width,
			                _mask_image->height);

#if 1
	for (int y = 0; y < _storage->height; ++y) {
	  for (int x = 0; x < _storage->width; ++x) {
	    unsigned int pix = XGetPixel(_storage, x, y);
	    if (_colorTable->GetColor(pix) == NULL) {
	      _colorTable->UseColorInTable(pix, NULL, NULL, NULL, NULL,
	                                   NULL, NULL);
	    }
	  }
	}
#endif

// hmmm... pixmap should be rather freed by selection owner
	XFreePixmap(GetDisplay(), *pixmap);

        { // notify main application to update the palette frame
          OWidgetMessage msg(MSG_PAINTCANVAS, MSG_IMAGEPASTED, -1);
          SendMessage(_msgObject, &msg);
        }
	break;
	
    case XA_STRING:
	break;

    default:
//	XtAppWarning(XtWidgetToApplicationContext(w),
//		     " selection request failed.  PixmapWidget");
	return False;
  }

  return True;
}

void OXPaintCanvas::_RequestSelection(Time time) {

  XChangeProperty(GetDisplay(), _id, _XC_SELECTION_DATA, XA_PIXMAP, 32,
                  PropModeReplace, (unsigned char *) 0, 0);

  XConvertSelection(GetDisplay(), XA_PRIMARY, XA_PIXMAP, _XC_SELECTION_DATA,
                    _id, time);

  _selection.limbo = True;
}

void OXPaintCanvas::Paste() {
  _RequestSelection(CurrentTime);
}
