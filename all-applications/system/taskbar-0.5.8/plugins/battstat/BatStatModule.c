#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <sys/stat.h>

#include <X11/X.h>
#include <X11/xpm.h>

#include <fvwm/fvwmlib.h>

#include "GoodyLoadable.h"

#define PROCAPMFILE             "/proc/apm"

#define FULL_ICON               "apm-full.xpm"
#define HALF_ICON               "apm-half.xpm"
#define EMPTY_ICON              "apm-empty.xpm"
#define ALERT_ICON              "apm-alert.xpm"
#define UNKNOWN_ICON            "apm-unknown.xpm"
#define LOADING_ICON            "apm-loading.xpm"
#define ONLINE_ICON             "apm-online.xpm"

typedef struct IconDef {
	char *        Name;
	Pixmap        Pix;
	Pixmap        Mask;
	XpmAttributes Attr;
} IconDef_t;

typedef struct BatStatInfo BatStatInfo_t;
struct BatStatInfo {
	char *id;
	/* other stuff */

	int fd; /* Descriptor to read the battery information */

	char ACStatus;
	char BatteryStatus;
	char BatteryPercent;

        IconDef_t FullIcon;
        IconDef_t HalfIcon;
        IconDef_t EmptyIcon;
        IconDef_t AlertIcon;
        IconDef_t LoadingIcon;
        IconDef_t OnlineIcon;
        IconDef_t UnknownIcon;
        char * IconLocation;

	int offset;
	int visible;
	Time lastclick;
	int show; /* whether to show the icon */
	time_t lastchecked;
	char *lock;
};
 
static void * BatStatModule_Init(char *id);
static int    BatStatModule_ParseResource(
		     BatStatInfo_t *mif,
		     char *tline,
		     char *Module,
		     int Clength);
static void  BatStatModule_Load(
		     BatStatInfo_t *mif,
		     Display *dpy,
		     Drawable win,
		     int *goodies_width);
static void  BatStatModule_Draw(
		     BatStatInfo_t *mif,
		     Display *dpy,
		     Window win);
static int  BatStatModule_SeeMouse(
		     BatStatInfo_t *mif,
		     int x,
		     int y);
static void  BatStatModule_CreateIconTipWindow_(
		     BatStatInfo_t *mif);
static void  BatStatModule_IconClick(
		     BatStatInfo_t *mif,
		     XEvent event);

extern int win_width, stwin_width;
extern int RowHeight;
extern GC statusgc;

/* 
*
* Purely local functions 
*
*/

/* Might better be part of the FvwmTaskbar environment */

static int LoadIcon( Display * dpy, Drawable win,
                char* FullPath, char * FilePart, char * FileName,
                IconDef_t * Icon ) {

        int rv;

        strcpy( FilePart, FileName );

#ifdef __DEBUG__
	fprintf(stderr, "FvwmTaskBar. BatStatModule.loadicon(\"%s\")\n", 
	                FullPath);
	fflush( stderr);
#endif

        Icon->Name = strdup( FullPath );
	rv = XpmReadFileToPixmap(dpy, win, 
	                Icon->Name,
			&(Icon->Pix), 
			&(Icon->Mask),
			&(Icon->Attr) );
        if( rv != XpmSuccess){
	  fprintf(stderr, "Icon loading(): error loading icon %s\n", Icon->Name );
	  free( Icon->Name );
	  Icon->Name = NULL;
	}
	return rv == XpmSuccess;

}

/* Figure out what Icon to use with respect to the battery status */

static IconDef_t * GetActualIcon( BatStatInfo_t * mif ) {

        IconDef_t * TheIcon;

	if( mif->ACStatus == 0x01 ) {
	  /* online */
	  if( mif->BatteryStatus == 0x03 ) {
	    /* Charging */
	    TheIcon = &(mif->LoadingIcon);
	  } else {
	    /* Simply online */
	    TheIcon = &(mif->OnlineIcon);
	  }
	} else {
	  /* offline */
	  switch( mif->BatteryStatus ) {
	    case 0 : /* High */
	      TheIcon = &(mif->FullIcon);
	      break;
	    case 1 : /* Low */
	      TheIcon = &(mif->HalfIcon);
	      break;
	    case 2 : /* Critical */
	      TheIcon = &(mif->AlertIcon);
	      break;
	    default : /* All others */
	      TheIcon = &(mif->EmptyIcon);
	      break;
	  }
	}
	return TheIcon;
}

/* 
*
* Read information from OS about battery status 
* Current implementation (Linux) Directly reads /proc/apm
* Hence support for the apm is required
*
* returns 1 if successful, 0 if not
*
*/

static int  ReadBatteryInfo(BatStatInfo_t *mif) {

        char Buffer[100];

	if (mif == NULL) return;
	if (mif->fd == -1) {
	  mif->fd = open( PROCAPMFILE, O_RDONLY );
	} else {
	  lseek( mif->fd, 0, SEEK_SET );
	}
	if (mif->fd == -1)
	  return 0;

        { char * p1;
          int ACStatus;
          int BatStatus;
          long BatPercent;

          /* Information about the layout and meaning of bits 
             can be found in the apm_bios.c source file in the kernel
          */

	  read( mif->fd, Buffer, 100 );

	  { int arg;

	    ACStatus = BatStatus = BatPercent = 0;
	    /* skip driver version 
	       skip apm-bios version 
	       skip apm-bios flags
	    */
	    arg = sscanf( Buffer, "%*s %*s %*s %x %x %*s %ld",
	          &ACStatus, &BatStatus, &BatPercent );
	    if( arg < 3 ) 
	      fprintf( stderr, "Bad conversion %d\n", arg );
          }

	  mif->ACStatus = ACStatus;
	  mif->BatteryStatus = BatStatus;
	  mif->BatteryPercent = BatPercent;
	}
	return 1;
}

/*
*
*       Goody Module interface functions
*
*/

static void * BatStatModule_Init(char *id) {

	BatStatInfo_t *mif;

#ifdef __DEBUG__
	fprintf(stderr, "FvwmTaskBar. BatStatModule.Init(\"%s\")\n", id);
	fflush( stderr);
#endif

	mif = NULL;
	(void *) mif = calloc(1, sizeof(BatStatInfo_t));
	if(mif == NULL) {
	  perror("FvwmTaskBar. BatStatModule.Init()");
	  return NULL;
	}

	mif->id = id;

	mif->fd = -1;

	/* Call to test if apm is available */
	if( ReadBatteryInfo( mif ) ) {
	  /* Initialize to Unknown status */
	  mif->ACStatus = -1;
	  mif->BatteryStatus = -1;
	  mif->BatteryPercent = -1;

	  mif->lastclick = 0;
	  mif->show = 1; /* show up by default */
	  mif->lastchecked = 0;
	  mif->lock = NULL;
	  mif->IconLocation = NULL;
	} else {
	  /* No apm */
	  free( mif );
	  mif = NULL;
	}
	return mif;
}

static int  BatStatModule_ParseResource(BatStatInfo_t *mif, char *tline,
                                 char *Module, int Clength) {
	char *s;

#ifdef __DEBUG__
	fprintf(stderr, 
	        "FvwmTaskBar. BatStatModule.ParseResource(\"%s\",\"%s\",*)\n",
	       mif->id, tline);
	fflush(stderr);
#endif

	if (mif == NULL) return 0;

	if(strncasecmp(tline,CatString3(Module, 
		      "ModuleIconPath",mif->id),
		      Clength+14+strlen(mif->id))==0) {
          char * Runner = &tline[Clength+15+strlen(mif->id)];

          while( isspace( *Runner ) && *Runner )
            Runner ++;

	  if( *Runner ) {
	    char * EOLPtr = Runner;

	    while( ! isspace( *EOLPtr ) && *EOLPtr ) 
	      EOLPtr ++;

            *EOLPtr = '\0'; /* Could already be '\0' */

	    /* Location of icons */
	    mif->IconLocation = strdup( Runner );
#ifdef __DEBUG__
	    fprintf( stderr, "Icon Location %s\n", mif->IconLocation );
	    fflush(stderr);
#endif
	  }
	  return(1);
	} else 
	  return 0;
}

static void  BatStatModule_Load(
                BatStatInfo_t *mif, Display *dpy, Drawable win,
                int *goodies_width) {

	int k;
	char * Buffer, * BasePos;

#ifdef __DEBUG__
	fprintf(stderr, "FvwmTaskBar. BatStatModule.Load()\n");
	fflush( stderr );
#endif

        if( mif->IconLocation == NULL ) {
          mif->visible = False;
	  fprintf(stderr, "FvwmTaskBar. BatStatModule.Load(): "
	                "No icon path set\n" );
          return;
        }

        /* Load icons */

        BasePos = Buffer = malloc( strlen( mif->IconLocation ) + 100 );
        BasePos += sprintf( Buffer, "%s/", mif->IconLocation );

        if( ! LoadIcon( dpy, win, Buffer, BasePos, FULL_ICON,
                      &(mif->FullIcon) ) ) { 
	  mif->visible = False;
	  return;
        }
        if( ! LoadIcon( dpy, win, Buffer, BasePos, HALF_ICON, 
                      &(mif->HalfIcon) ) ) {
	  mif->visible = False;
	  return;
        }
        if( ! LoadIcon( dpy, win, Buffer, BasePos, EMPTY_ICON, 
                      &(mif->EmptyIcon) ) ) {
	  mif->visible = False;
	  return;
        }
        if( ! LoadIcon( dpy, win, Buffer, BasePos, ALERT_ICON, 
                      &(mif->AlertIcon) ) ) {
	  mif->visible = False;
	  return;
        }
        if( ! LoadIcon( dpy, win, Buffer, BasePos, LOADING_ICON, 
                      &(mif->LoadingIcon) ) ) {
	  mif->visible = False;
	  return;
        }
        if( ! LoadIcon( dpy, win, Buffer, BasePos, ONLINE_ICON, 
                      &(mif->OnlineIcon) ) ) {
	  mif->visible = False;
	  return;
        }
        if( ! LoadIcon( dpy, win, Buffer, BasePos, UNKNOWN_ICON, 
                      &(mif->UnknownIcon) ) ) {
	  mif->visible = False;
	  return;
        }

	/* Icons loaded sucessfully */
	mif->visible = True;
	if ((mif->LoadingIcon.Attr.valuemask & XpmSize) == 0) {
	  mif->LoadingIcon.Attr.width = 16;
	  mif->LoadingIcon.Attr.height = 16;
	}

	*goodies_width += mif->LoadingIcon.Attr.width+2;    
	mif->offset = icons_offset;
	icons_offset += mif->LoadingIcon.Attr.width+2;
}

static void  BatStatModule_Draw(BatStatInfo_t *mif, Display *dpy, Window win) {

	XGCValues gcv;
	GC copy;
	time_t now;
	IconDef_t * TheIcon;
	unsigned long  gcm = 
	        GCClipMask | GCClipXOrigin | GCClipYOrigin;

	if (mif == NULL) return;

	now = time(NULL);

	if (mif->visible && mif->show) {

	  if (now-mif->lastchecked > 2) {
	    mif->lastchecked = now;
	    ReadBatteryInfo( mif );
	  }
			     
	  TheIcon = GetActualIcon( mif );

	  /* Whipe icon */
	  XClearArea( dpy, win, 
	              win_width - stwin_width+icons_offset+3, 1,
		      TheIcon->Attr.width, 
		      RowHeight-2, False);
	  gcv.clip_mask = TheIcon->Mask;
	  gcv.clip_x_origin = (win_width-stwin_width) + icons_offset+3;
	  gcv.clip_y_origin = ((RowHeight - TheIcon->Attr.height) >> 1);

	  XChangeGC(dpy, statusgc, gcm, &gcv);      
	  XCopyArea(dpy, TheIcon->Pix, win, statusgc, 0, 0,
		    TheIcon->Attr.width, 
		    TheIcon->Attr.height,
		    gcv.clip_x_origin,
		    gcv.clip_y_origin);

	  mif->offset = icons_offset;
	  icons_offset += TheIcon->Attr.width+2;
	}
}
         
static int  BatStatModule_SeeMouse(BatStatInfo_t *mif, int x, int y) {

	int xl, xr;
	IconDef_t * TheIcon;

	if (mif == NULL) return 0;
	if (mif->show == 0) return 0;

	TheIcon = GetActualIcon( mif );

	/* Mouse in icon area ? */
	xl = win_width - stwin_width + mif->offset;
	xr = win_width - stwin_width + mif->offset + TheIcon->Attr.width;

	return (x>=xl && x<xr && y>1 && y<RowHeight-2);
}


static void  BatStatModule_CreateIconTipWindow_(BatStatInfo_t *mif) {

        char Buffer[100];

	if (mif == NULL) return;

	if( mif->ACStatus == 0 ) {
	  sprintf( Buffer, "State : %ld%%", mif->BatteryPercent );
	} else {
	  if( mif->BatteryStatus == 0x03 ) {
	    /* Charging */
	    sprintf( Buffer, "Online Loading : %ld%%", mif->BatteryPercent );
	  } else {
	    /* Simply online */
	    sprintf( Buffer, "Online" );
	  }
	}

	PopupTipWindow(
	      win_width - stwin_width + mif->offset, 
	      0, 
	      Buffer);

}

static void  BatStatModule_IconClick(BatStatInfo_t *mif, XEvent event) {
        return;
}

/* Produces warnings because of the static attribute */

struct GoodyLoadable  BatStatModuleSymbol = {
        BatStatModule_Init,
        BatStatModule_ParseResource,
        BatStatModule_Load,
        BatStatModule_Draw,
        BatStatModule_SeeMouse,
        BatStatModule_CreateIconTipWindow_,
        BatStatModule_IconClick
};

