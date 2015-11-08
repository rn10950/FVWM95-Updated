#ifndef __Goodies__
#define __Goodies__

#define DEFAULT_MAIL_PATH  "/var/spool/mail/"
#define DEFAULT_BELL_VOLUME 20

/* Tip window types */
/* non-negative tip types are for icon buttons */
#define NO_TIP        (-1)
#define START_TIP     (-2)
#define GLOADABLE_TIP (-3)
/* loadable tip type numbers decrease from GLOADABLE_TIP */

#define DEFAULT_MAX_TIP_LINES 30

typedef struct {
  int  x, y, w, h, tw, th, open, type;
  char *text;
  Window win;
  char **lines;
  int nlines;
  int fFree;
} TipStruct;

void GoodiesParseConfig(char *tline, char *Module);
void InitGoodies();
void DrawGoodies();
void PopupTipWindow(int px, int py, char *text);
void CreateTipWindow(int x, int y, int w, int h);
void RedrawTipWindow();
void DestroyTipWindow();
void ShowTipWindow(int open);
/* void HandleMouseClick(XEvent event); */

#endif
