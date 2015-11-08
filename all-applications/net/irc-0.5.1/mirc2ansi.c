/*----------------------------------------------------------------------

    Simple filter program to convert IRC video attributes and
    mIRC color codes to ANSI sequences. Version 0.01 (Hector Peraza)
    
    Compilation:
    
      cc -o mirc2ansi mirc2ansi.c
      
    Usage(s):

      mirc2ansi logfile
      
    or
    
      cat logfile | mirc2ansi
      
    etc...

----------------------------------------------------------------------*/    

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define ATTRIB_NORMAL     0x00
#define ATTRIB_BOLD       0x01
#define ATTRIB_REVERSE    0x02
#define ATTRIB_UNDERLINE  0x04
#define ATTRIB_FGCOLOR    0x08
#define ATTRIB_BGCOLOR    0x10


/*----------------------------------------------------------------------

  These are the mIRC colors and their approximate ANSI codes:
  
  index	  color		X color		ANSI	  fg  bg
  -----	  -----		-------		----	  --  --
    0	  white		white		white	  37  47
    1	  black		black		gray30	  30  40
    2	  dark blue	royalblue	blue	  34  44
    3	  dark green	darkgreen	green	  32  42
    4	  red		red3		red	  31  41
    5	  brown		brown		yellow	  33  43
    6	  dark purple	mediumpurple	magenta	  35  45
    7	  orange	orange		red
    8	  yellow	yellow		yellow
    9	  light green	lightgreen	green
   10	  cyan		cyan		cyan	  36  46
   11	  light cyan	lightcyan	cyan
   12	  light blue	lightblue	blue
   13	  pink		pink		magenta
   14	  grey		darkslategray	gray30
   15	  light grey	lightgray	white

----------------------------------------------------------------------*/

int mirc2ansi[16] = { 7, 0, 4, 2, 1, 3, 5, 11, 13, 12, 6, 16, 14, 15, 10, 7 };

int set_attrib(int ca, int fg, int bg) {
  char a[20];
  
  a[0] = '\0';
  if (ca & ATTRIB_BOLD)      { if (*a) strcat(a, ";"); strcat(a, "1"); }
  if (ca & ATTRIB_REVERSE)   { if (*a) strcat(a, ";"); strcat(a, "7"); }
  if (ca & ATTRIB_UNDERLINE) { if (*a) strcat(a, ";"); strcat(a, "4"); }
  if (!*a) strcpy(a, "0");
  if (ca & ATTRIB_FGCOLOR) {
    fg = mirc2ansi[fg & 0x0F]; if (fg >= 10) fg -= 10;
    if (*a) strcat(a, ";");
    sprintf(a, "%s%d", a, fg+30);
  }
  if (ca & ATTRIB_BGCOLOR) {
    bg = mirc2ansi[bg & 0x0F]; if (bg >= 10) bg -= 10;
    if (*a) strcat(a, ";");
    sprintf(a, "%s%d", a, bg+40);
  }
  printf("\033[%sm", a);
}

int main(int argc, char *argv[]) {
  FILE *f;
  int  c, ca, fg, bg;
  
  if (argc > 1) {
    f = fopen(argv[1], "r");
    if (!f) {
      fprintf(stderr, "File not found: %s\n", argv[1]); 
      return 1;
    }
  } else {
    f = stdin;
  }

  ca = ATTRIB_NORMAL;
  fg = bg = 0;

  c = fgetc(f);
  while (c != EOF) {
    if (c == 0x02) { 		/* toggle bold */
      if (ca & ATTRIB_BOLD)
        ca &= ~ATTRIB_BOLD;
      else
        ca |= ATTRIB_BOLD;
      set_attrib(ca, fg, bg);
    } else if (c == 0x16) { 	/* toggle reverse video */
      if (ca & ATTRIB_REVERSE)
        ca &= ~ATTRIB_REVERSE;
      else
        ca |= ATTRIB_REVERSE;
      set_attrib(ca, fg, bg);
    } else if (c == 0x1F) {	/* toggle underline */
      if (ca & ATTRIB_UNDERLINE)
        ca &= ~ATTRIB_UNDERLINE;
      else
        ca |= ATTRIB_UNDERLINE;
      set_attrib(ca, fg, bg);
    } else if (c == 0x03) {	/* change color attrib */
      c = fgetc(f);
      if (!isdigit(c)) {
        ca &= ~(ATTRIB_FGCOLOR | ATTRIB_BGCOLOR);
        set_attrib(ca, fg, bg);
        continue;
      } else {
        fg = c - '0';
        c = fgetc(f);
        if (isdigit(c)) {
          fg = fg*10 + (c - '0');
          c = fgetc(f);
        }
        ca |= ATTRIB_FGCOLOR;
        set_attrib(ca, fg, bg);
        if (c == ',') {
          c = fgetc(f);
          if (isdigit(c)) {
            bg = c - '0';
            c = fgetc(f);
            if (isdigit(c)) {
              bg = bg*10 + (c - '0');
              c = fgetc(f);
            }
            ca |= ATTRIB_BGCOLOR;
            set_attrib(ca, fg, bg);
          }
        }
        continue;
      }
    } else {
      if (c == '\n') {
        ca = ATTRIB_NORMAL;
        set_attrib(ca, fg, bg);
      }
      putchar(c);
    }
    c = fgetc(f);
  }
  
  if (f != stdin) fclose(f);
  return 0;
}
