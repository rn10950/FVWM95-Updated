#ifndef __FVWMDEFS_H
#define __FVWMDEFS_H

/* 
 *  The following section is taken from the fvwm.h file.
 *  Always ensure that these values match the ones in your particular
 *  fvwm distribution!
 */

/***************************************************************************
 * window flags definitions 
 ***************************************************************************/

/* The first 13 items are mapped directly from the style structure's
 * flag value, so they MUST correspond to the first 13 entries in misc.h */
#define STARTICONIC             (1<<0)
#define ONTOP                   (1<<1) /* does window stay on top */
#define STICKY                  (1<<2) /* Does window stick to glass? */
#define WINDOWLISTSKIP          (1<<3)
#define SUPPRESSICON            (1<<4)
#define NOICON_TITLE            (1<<5)
#define Lenience                (1<<6)
#define StickyIcon              (1<<7)
#define CirculateSkipIcon       (1<<8)
#define CirculateSkip           (1<<9)
#define ClickToFocus            (1<<10)
#define SloppyFocus             (1<<11)
#define SHOW_ON_MAP    (1<<12) /* switch to desk when it gets mapped? */
#define ALL_COMMON_FLAGS (STARTICONIC|ONTOP|STICKY|WINDOWLISTSKIP| \
			  SUPPRESSICON|NOICON_TITLE|Lenience|StickyIcon| \
			  CirculateSkipIcon|CirculateSkip|ClickToFocus| \
			  SloppyFocus|SHOW_ON_MAP) 

#define BORDER         (1<<13) /* Is this decorated with border*/
#define TITLE          (1<<14) /* Is this decorated with title */
#define MAPPED         (1<<15) /* is it mapped? */
#define ICONIFIED      (1<<16) /* is it an icon now? */
#define TRANSIENT      (1<<17) /* is it a transient window? */
#define RAISED         (1<<18) /* if its a sticky window, needs raising? */
#define VISIBLE        (1<<19) /* is the window fully visible */
#define ICON_OURS      (1<<20) /* is the icon window supplied by the app? */
#define PIXMAP_OURS    (1<<21)/* is the icon pixmap ours to free? */
#define SHAPED_ICON    (1<<22)/* is the icon shaped? */
#define MAXIMIZED      (1<<23)/* is the window maximized? */
#define DoesWmTakeFocus		(1<<24)
#define DoesWmDeleteWindow	(1<<25)
/* has the icon been moved by the user? */
#define ICON_MOVED              (1<<26)
/* was the icon unmapped, even though the window is still iconified
 * (Transients) */
#define ICON_UNMAPPED           (1<<27) 
/* Sent an XMapWindow, but didn't receive a MapNotify yet.*/
#define MAP_PENDING             (1<<28)
#define HintOverride            (1<<29)
/* not used in fvwm-95
#define MWMButtons              (1<<30)
#define MWMBorders              (1<<31)
*/

#endif  // __FVWMDEFS_H
