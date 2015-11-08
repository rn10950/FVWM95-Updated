
#ifndef __OXPAINTCANVAS_H
#define __OXPAINTCANVAS_H


#include <X11/X.h>
#include <X11/Xproto.h>
#include <X11/xpm.h>

#include <xclass/OXFrame.h>

#include "OColorTable.h"


#define MSG_PAINTCANVAS  (MSG_USERMSG+60)
#define MSG_IMAGEPASTED  1


//--- Pixmap pixel defines

#define NOT_SET    -1
#define CLEAR       0
#define SET         1
#define INVERT      2
#define HIGHLIGHT   3

#define NoColorName "None"
#define TRANSPARENT(dpy, scr_num) (1<<DisplayPlanes(dpy, scr_num))


//--- Operation modes

#define DRAW_POINT             0
#define DRAW_LINE              1
#define DRAW_CURVE             2
#define DRAW_RECTANGLE         3
#define DRAW_FILLED_RECTANGLE  4
#define DRAW_CIRCLE            5
#define DRAW_FILLED_CIRCLE     6
#define DRAW_TEXT              7
#define FLOOD_FILL             8
#define SELECT_REGION          9
#define MOVE_REGION           10
#define COPY_REGION           11


typedef enum { Lazy, NotLazy }        DrawingMode;
typedef enum { Right, Left }          RotateDirection;
typedef enum { Horizontal, Vertical } FlipAxis;


//-------------------------------------------------------------------------

#ifndef sign
#define sign(x) ((x) ? (((x) > 0) ? 1 : -1) : 0)
#endif

#define InPixmapX(x)\
        (int) (min((int)((max(_horizOffset, x) -\
                         _horizOffset) / _squareSize), _width - 1))

#define InPixmapY(y)\
        (int) (min((int)((max(_vertOffset, y) -\
                         _vertOffset) / _squareSize), _height - 1))

#define InWindowX(x)\
        (int) (_horizOffset + ((x) * _squareSize))

#define InWindowY(y)\
        (int) (_vertOffset + ((y) * _squareSize))
   
#define QuerySet(x, y) (((x) != NOT_SET) && ((y) != NOT_SET))

#define QueryZero(x, y) (((x) == 0) || ((y) == 0))
                         
#define Swap(x, y) { int t; t = x; x = y; y = t; }
  
#define QuerySwap(x, y) if(x > y) Swap(x, y)

#define QueryInPixmap(x, y)\
  (((x) >= 0) && ((x) < _image->width) &&\
   ((y) >= 0) && ((y) < _image->height))

#define roundint(x)  ((int) floor((x) + 0.5))

#define WithinMark(xi, yi) \
  ((xi <= max(_mark.from_x, _mark.to_x)) &&\
   (xi >= min(_mark.from_x, _mark.to_x)) &&\
   (yi <= max(_mark.from_y, _mark.to_y)) &&\
   (yi >= min(_mark.from_y, _mark.to_y)))

#define HotInMark() WithinMark(_hot.x, _hot.y)
                         
#define IsHotSpot(xi, yi) \
  ((xi == _hot.x) && (yi == _hot.y))

#define QueryFlood(x, y, value)\
  (QueryInPixmap(x, y)\
     && (_GetPxl(x, y) == value))

#define Flood(x, y, value) _DrawPoint(x, y, value, NotLazy)


//-------------------------------------------------------------------------

class OArea {
public:
  int from_x, from_y, to_x, to_y;
};

class OSelection {
public:
  int own, limbo;
};

class OXPaintCanvas : public OXFrame {
protected:
  static Atom _XC_SELECTION_DATA;

public:
  OXPaintCanvas(const OXWindow *p, int w, int h,
                unsigned int options = 0,
                unsigned long back = _defaultFrameBackground);
  virtual ~OXPaintCanvas();

  virtual ODimension GetDefaultSize() const;
  virtual int HandleExpose(XExposeEvent *event);
  virtual int HandleButton(XButtonEvent *event);
  virtual int HandleMotion(XMotionEvent *event);

  virtual int HandleSelection(XSelectionEvent *event);
  virtual int HandleSelectionRequest(XSelectionRequestEvent *event);
  virtual int HandleSelectionClear(XSelectionClearEvent *event);

  void    SetMode(int mode);

  Pixmap  GetPixmap(XImage *image);
  XImage  *GetImage(Pixmap pixmap);
  XImage  *CreatePixmapImage(int width, int height);
  XImage  *CreateMaskImage(int width, int height);
  void    DestroyPixmapImage(XImage **image);
  void    DestroyMaskImage(XImage **mask) { DestroyPixmapImage(mask); }
  void    Select(int from_x, int from_y, int to_x, int to_y, Time time);  
  void    SetTransparentPixels(XImage *image, XImage *mask_image);
  int     StoreFile(char *filename);
  void    ChangeFilename(char *str);
  void    Comments(char **hints_cmt, char **colors_cmt, char **pixels_cmt);
  OColorTable *GetColorTable() const { return _colorTable; } 
  int     ReadFile(char *filename);
  void    SetImage(XImage *image, XImage *mask_image);
  void    BuildXpmImageAndAttributes(XImage *image, XImage *mask_image,
                   XImage **xpm_image_return, XpmInfo **infos_return);
  int     WriteFile(char *filename);
  const char *GetFilename() const { return _filename; }
  char    *GetFilepath(char **str);
  void    SetZoom(int zoom);
  void    RescaleImage(int width, int height);
  void    ResizeImage(int width, int height);
  void    Clip(int from_x, int from_y, int to_x, int to_y);
  void    Unclip();
  void    Refresh(int x, int y, int width, int height);
  void    SetGrid(int state);
  int     GridState() const { return _grid; }
  void    SetFont(OXFont *font);
  void    SetText(char *s);
  const char *GetText() const { return _text_string; }
  int     IsStored() const { return (_storage != NULL); }
  void    SetChanged() { _changed = True; }
  void    ClearChanged() { _changed = False; }
  int     HasChanged() const { return _changed; }
  int     QueryMarked() const { return QuerySet(_mark.from_x, _mark.from_y); }
  void    CopyImageData(XImage *source, XImage *destination,
                        int from_x, int from_y,
                        int to_x, int to_y, int at_x, int at_y);
  void    TransferImageData(XImage *source, XImage *destination);
  Pixel   GetPxl(int x, int y);
  void    DrawImageData(XImage *image, XImage *mask_image,
                        int at_x, int at_y, int value, DrawingMode mode);
  XImage  *ScalePixmapImage(XImage *src,
                            double scale_x, double scale_y);
  XImage  *ScaleMaskImage(XImage *src,
                          double scale_x, double scale_y);
  void    RedrawHotSpot();
  void    ClearHotSpot();
  void    DrawHotSpot(int x, int y, int value);
  void    SetHotSpot(int x, int y);
  void    DrawPoint(int x, int y, int value);
  void    RedrawSquares(int from_x, int from_y, int to_x, int to_y);
  void    RedrawPoints(int from_x, int from_y, int to_x, int to_y);
  void    DrawGrid(int from_x, int from_y, int to_x, int to_y);
  void    RedrawGrid(int from_x, int from_y, int to_x, int to_y);
  void    DrawLine(int from_x, int from_y, int to_x, int to_y, int value);
  void    DrawRectangle(int from_x, int from_y, int to_x, int to_y, int value);
  void    DrawFilledRectangle(int from_x, int from_y, int to_x, int to_y, 
                              int value);
  void    DrawCircle(int from_x, int from_y, int to_x, int to_y, int value);
  void    DrawFilledCircle(int from_x, int from_y, int to_x, int to_y, 
                           int value);
  void    FloodFill(int x, int y, int value);
  void    ShiftUp();
  void    ShiftDown();
  void    ShiftLeft();
  void    ShiftRight();
  void    RedrawMark();
  void    Fold();
  void    ClearAll();
  void    SetAll();
  void    TranslatePoint(int *x, int *y, int dx, int dy);
  void    FlipPoint(int *x, int *y, FlipAxis axis);
  void    RotatePoint(int *x, int *y, RotateDirection direction);
  void    FlipHoriz();
  void    FlipVert();
  void    RotateRight();
  void    RotateLeft();
  void    Store();
  void    ClearMarked();
  void    DragMarked(int at_x, int at_y);
  void    DragStored(int at_x, int at_y);
  void    Restore(int at_x, int at_y, int value);
  void    CopyRegion(int at_x, int at_y, int value);
  void    MoveRegion(int at_x, int at_y, int value);
  void    StoreToBuffer();
  void    Unmark();
  void    Mark(int from_x, int from_y, int to_x, int to_y);
  void    Undo();
  void    RotateBufferAndImage();
  void    SetDrawingForeground(Pixel color);
  void    DragText(int at_x, int at_y, int value);
  void    DrawText(int at_x, int at_y, int value);

  Pixel   GetTransparentPixel() const { return _transparentPixel; }
  int     IsStippled() const { return _stippled; }
  Pixmap  GetStipplePixmap() const { return (_stippled ? _stipple : None); }

  GC GetDrawingGC() const { return _drawingGC; }
  GC GetHighlightingGC() const { return _highlightingGC; }
  GC GetFramingGC() const { return _framingGC; }

  int GetImageWidth() const { return _width; }
  int GetImageHeight() const { return _height; }
  
  void Paste();

protected:
  void    _InitializeXpmAttributes(XpmAttributes *attribs);
  void    _InitializeXpmInfo(XpmInfo *infos);
  void    _InitializeXpmImage(XpmImage *xpm_image);
  int     _readFile(char *filename);
  void    _FixMark();
  void    _FixHotSpot();
  void    _Resize();
  Pixel   _GetPxl(int x, int y);
  void    _SetPxl(int x, int y, Pixel color);
  void    _ClearPxl(int x, int y, Pixel white) { _SetPxl(x, y, white); }
  Pixel   _GetPxlFromImageAndMask(XImage *image,
                                  XImage *mask_image, int x, int y);
  XPoint  *_HotSpotShape(int x, int y);
  void    _DrawHotSpot(int x, int y);
  void    _HighlightHotSpot(int x, int y);
  void    _HighlightSquare(int x, int y);
  void    _ClearSquare(int x, int y);
  void    _DrawSquare(int x, int y, Pixel color);
  void    _DrawPoint(int x, int y, Pixel color, int mode);
  void    _ClearPoint(int x, int y, int mode);
  void    _FloodLoop(int x, int y, Pixel value, Pixel toflood);

  void    _GrabSelection(Time time);
  int     _ConvertSelection(Atom selection, Atom target, Atom *type,
                            unsigned char **value, unsigned long *length,
                            int *format);
  void    _SelectionDone(Atom *selection, Atom *target);
  int     _PasteSelection(Atom type, unsigned char *value);
  void    _RequestSelection(Time time);


  int     _pressed, _x0, _y0, _x1, _y1, _drawMode, _lastMode;

  //--- resources
  Cursor  _drawCursor, _selCursor, _moveCursor;
  Pixel   _fgndPixel, _hilitePixel, _framingPixel, _transparentPixel;
  Pixel   _clearPixel;
  Pixmap  _stipple;
  int     _stippled, _grid;
  int     _gridTolerance, _distance, _squareSize, _width, _height;
  char    *_filename;

private:
  char    *_NewString(char *str);

  int          _nextensions, _buffer_nextensions;
  XpmExtension *_extensions, *_buffer_extensions;

  OXFont      *_font;
  XFontStruct *_font_struct;
  char        *_text_string; 
  int         _text_lbearing, _text_rbearing;
  int         _text_ascent, _text_descent; 

  XPoint      _hot, _buffer_hot, _storage_hot;
  int         _horizOffset, _vertOffset;
  XImage      *_image, *_mask_image;
  XImage      *_buffer, *_mask_buffer;
  XImage      *_storage, *_mask_storage;
  OArea       _mark, _buffer_mark;
  GC          _drawingGC, _highlightingGC, _framingGC, _transparentGC;
  int         _changed, _fold;
  OSelection  _selection;
  OColorTable *_colorTable;
  char        *_hints_cmt, *_colors_cmt, *_pixels_cmt;

  unsigned int _screen, _depth;
  Colormap     _colormap;
};


#endif  // __OXPAINTCANVAS_H
