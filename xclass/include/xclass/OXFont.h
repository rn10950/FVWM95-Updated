/**************************************************************************

   The code here is based on the tk font object implementation.

   The tkFont.h file contains the following copyright notice:

     Copyright (c) 1996 Sun Microsystems, Inc.

     See the file "tk.license.terms" for information on usage and
     redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.

     [ tkFont.h 1.11 97/05/07 14:44:13 ]

**************************************************************************/

#ifndef __OXFONT_H
#define __OXFONT_H

#include <xclass/utils.h>
#include <xclass/OXObject.h>
#include <xclass/OHashTable.h>

class OXClient;
class OString;
class OTextBuffer;


//----------------------------------------------------------------------

// The following class keeps track of the attributes of a font. It can
// be used to keep track of either the desired attributes or the actual
// attributes gotten when the font was instantiated.

class OFontAttributes : public OBaseObject {
public:
  OFontAttributes() { Init(); }

  void Init();

  const char *family;	// Font family. The most important field.
  int pointsize;	// Pointsize of font, 0 for default size, or
			// negative number meaning pixel size.
  int weight;		// Weight flag; see below for def'n.
  int slant;		// Slant flag; see below for def'n.
  int underline;	// Non-zero for underline font.
  int overstrike;	// Non-zero for overstrike font.
};


// Possible values for the "weight" field in a OFontAttributes object.
// Weight is a subjective term and depends on what the company that created
// the font considers bold.

#define FONT_FW_NORMAL	0
#define FONT_FW_BOLD	1

#define FONT_FW_UNKNOWN	-1	// Unknown weight. This value is used for
				// error checking and is never actually stored
				// in the weight field.

// Possible values for the "slant" field in a OFontAttributes object.

#define FONT_FS_ROMAN	0
#define FONT_FS_ITALIC	1
#define FONT_FS_OBLIQUE	2	// This value is only used when parsing X
				// font names to determine the closest
				// match. It is only stored in the
				// XLFDAttributes structure, never in the
				// slant field of the OFontAttributes.

#define FONT_FS_UNKNOWN	-1	// Unknown slant. This value is used for
				// error checking and is never actually stored
				// in the slant field.


// The following structure keeps track of the metrics for an instantiated
// font. The metrics are the physical properties of the font itself.

class OFontMetrics : public OBaseObject {
public:
  int ascent;			// From baseline to top of font.
  int descent;			// From baseline to bottom of font.
  int linespace;		// The sum of the ascent and descent.
  int maxWidth;			// Width of widest character in font.
  int fixed;			// True if this is a fixed-width font
				// (monospaced), False otherwise.
};


// Flags passed to OXFont::MeasureChars
  
#define TEXT_WHOLE_WORDS	1
#define TEXT_AT_LEAST_ONE	2
#define TEXT_PARTIAL_OK		4
  

// Flags passed to OXFont::ComputeTextLayout
    
#define TEXT_IGNORE_TABS	8
#define TEXT_IGNORE_NEWLINES	16


// Text justification styles

#define TEXT_JUSTIFY_LEFT	1
#define TEXT_JUSTIFY_RIGHT	2
#define TEXT_JUSTIFY_CENTER	3


// The following two structures are used to keep track of string
// measurement information when using the text layout facilities.
//
// A OLayoutChunk represents a contiguous range of text that can be measured
// and displayed by low-level text calls. In general, chunks will be
// delimited by newlines and tabs. Low-level, platform-specific things
// like kerning and non-integer character widths may occur between the
// characters in a single chunk, but not between characters in different
// chunks.
//
// A OTextLayout is a collection of OLayoutChunks. It can be displayed with
// respect to any origin.

class OLayoutChunk : public OBaseObject {

  friend class OXFont;
  friend class OTextLayout;

  const char *start;		// Pointer to simple string to be displayed.
				// This is a pointer into the OTextLayout's
				// string.
  int numChars;			// The number of characters in this chunk.
  int numDisplayChars;		// The number of characters to display when
				// this chunk is displayed. Can be less than
				// numChars if extra space characters were
				// absorbed by the end of the chunk. This
				// will be < 0 if this is a chunk that is
				// holding a tab or newline.
  int x, y;			// The origin of the first character in this
				// chunk with respect to the upper-left hand
				// corner of the OTextLayout.
  int totalWidth;		// Width in pixels of this chunk. Used
				// when hit testing the invisible spaces at
				// the end of a chunk.
  int displayWidth;		// Width in pixels of the displayable
				// characters in this chunk. Can be less than
				// width if extra space characters were
				// absorbed by the end of the chunk.
};

class OTextLayout : public OBaseObject {
public:
  OTextLayout() { numChunks = 0; chunks = NULL; }
  ~OTextLayout();

  void DrawText(Display *dpy, Drawable dst, GC gc,
                int x, int y, int firstChar, int lastChar) const;
  void UnderlineChar(Display *dpy, Drawable dst, GC gc,
                     int x, int y, int underline) const;
  int  PointToChar(int x, int y) const;
  int  CharBbox(int index, int *x, int *y, int *w, int *h) const;
  int  DistanceToText(int x, int y) const;
  int  IntersectText(int x, int y, int w, int h) const;
  void ToPostscript(OString *dst) const;

  friend class OXFont;

protected:
  const OXFont *font;		// The font used when laying out the text.
  const char *string;		// The string that was layed out.
  int width;			// The maximum width of all lines in the
				// text layout.
  int numChunks;		// Number of chunks actually used in
				// following array.
  OLayoutChunk *chunks;		// Array of chunks. The actual size will
				// be maxChunks.
};


// The following class is used to keep track of the generic information
// about a font.

class OXFont : public OXObject {
protected:
  OXFont() {}
  virtual ~OXFont() {}

public:
  char *NameOfFont() const;
  void GetFontMetrics(OFontMetrics *m) const;
  void GetFontAttributes(OFontAttributes *a) const;
  int  PostscriptFontName(OString *dst) const;
  int  TextWidth(const char *string, int numChars = -1) const;
  int  XTextWidth(const char *string, int numChars = -1) const;
  int  TextHeight() const { return fm.linespace; }
  void UnderlineChars(Display *dpy, Drawable dst, GC gc,
                      const char *string, int x, int y,
                      int firstChar, int lastChar) const;
  OTextLayout *ComputeTextLayout(const char *string, int numChars,
                      int wrapLength, int justify, int flags,
                      int *width, int *height) const;
  int  MeasureChars(const char *source, int numChars, int maxLength,
                      int flags, int *length) const;
  void DrawCharsExp(Display *dpy, Drawable dst, GC gc,
                      const char *source, int numChars, int x, int y) const;
  void DrawChars(Display *dpy, Drawable dst, GC gc,
                      const char *source, int numChars, int x, int y) const;

  friend class OFontPool;
  friend class OTextLayout;

protected:
  void SetId(XID id) { _id = id; }
  OLayoutChunk *NewChunk(OTextLayout *layout, int *maxPtr,
                         const char *start, int numChars,
                         int curX, int newX, int y) const;

  int refCount;			// Number of users of the OXFont.
  OHashEntry *cacheHash;	// Entry in font cache for this structure,
				// used when deleting it.
  OHashEntry *namedHash;	// Pointer to hash table entry that
				// corresponds to the named font that the
				// OXFont object was based on, or NULL if
				// the font object was not based on a named
				// font.
  int tabWidth;			// Width of tabs in this font (pixels).
  int underlinePos;		// Offset from baseline to origin of
				// underline bar (used for drawing underlines
				// on a non-underlined font).
  int underlineHeight;		// Height of underline bar (used for drawing
				// underlines on a non-underlined font).
  OFontAttributes fa;		// Actual font attributes obtained when the
				// the font was created, as opposed to the
				// desired attributes passed in to
				// GetFontFromAttributes(). The desired
				// metrics can be determined from the string
				// that was used to create this font.
  OFontMetrics fm;		// Font metrics determined when font was
				// created.

  // Fields that represent Unix's implementation of a font

  Display *display;             // The display to which font belongs.
  XFontStruct *fontStruct;      // X information about font.
  char types[256];              // Array giving types of all characters in
                                // the font, used when displaying control
                                // characters. See OXFont.cc for definition.
  int widths[256];              // Array giving widths of all possible
                                // characters in the font.
/*
  int underlinePos;             // Offset from baseline to origin of
                                // underline bar (used for simulating a
                                // native underlined font).
*/
  int barHeight;                // Height of underline or overstrike bar
                                // (used for simulating a native underlined
                                // or strikeout font).
};


// The following structure is used to return attributes when parsing an
// XLFD. The extra information is used to find the closest matching font.

class OXLFDAttributes : public OBaseObject {

  friend class OFontPool;

  OFontAttributes fa;		// Standard set of font attributes.
  const char *foundry;		// The foundry of the font.
  int slant;			// The tristate value for the slant, which
				// is significant under X.
  int setwidth;			// The proportionate width, see below for
				// definition.
  int charset;			// The character set encoding (the glyph
				// family), see below for definition.
  int encoding;			// Variations within a charset for the
				// glyphs above character 127.
};


// Possible values for the "setwidth" field in an OXLFDAttributes object.
// The setwidth is whether characters are considered wider or narrower than
// normal.

#define FONT_SW_NORMAL	 0
#define FONT_SW_CONDENSE 1
#define FONT_SW_EXPAND	 2
#define FONT_SW_UNKNOWN	 3	// Unknown setwidth. This value may be
				// stored in the setwidth field.


// Possible values for the "charset" field in an OXLFDAttributes object.
// The charset is the set of glyphs that are used in the font.

#define FONT_CS_NORMAL	0
#define FONT_CS_SYMBOL	1
#define FONT_CS_OTHER	2


// The following defines specify the meaning of the fields in a fully
// qualified XLFD.

#define XLFD_FOUNDRY	    0
#define XLFD_FAMILY	    1
#define XLFD_WEIGHT	    2
#define XLFD_SLANT	    3
#define XLFD_SETWIDTH	    4
#define XLFD_ADD_STYLE	    5
#define XLFD_PIXEL_SIZE	    6
#define XLFD_POINT_SIZE	    7
#define XLFD_RESOLUTION_X   8
#define XLFD_RESOLUTION_Y   9
#define XLFD_SPACING	    10
#define XLFD_AVERAGE_WIDTH  11
#define XLFD_REGISTRY	    12
#define XLFD_ENCODING	    13
#define XLFD_NUMFIELDS	    14	// Number of fields in XLFD.


// The following class is used to keep track of all the fonts that
// exist in the current application.

struct StateMap;

class OFontPool : public OBaseObject {
public:
  OFontPool(const OXClient *client) { _client = client; }
  ~OFontPool();

  OXFont *GetFont(const char *string);
  OXFont *GetFont(const char *family, int ptsize, int weight, int slant);
  OXFont *GetFont(OXFont *font);
  void FreeFont(OXFont *font);
  char *NameOfFont(OXFont *font);
  char **GetFontFamilies();
  void FreeFontFamilies(char **f);
  char **GetAttributeInfo(const OFontAttributes *fa);
  void FreeAttributeInfo(char **info);
  int  ParseFontName(const char *string, OFontAttributes *fa);

protected:
  const char *GetUid(const char *string);
  int ParseXLFD(const char *string, OXLFDAttributes *xa);
  int FindStateNum(const StateMap *map, const char *strKey);
  char *FindStateString(const StateMap *map, int numKey);
  int FieldSpecified(const char *field);
  OXFont *GetNativeFont(const char *name);
  OXFont *GetFontFromAttributes(OFontAttributes *fa, OXFont *fontPtr);
  OXFont *MakeFont(OXFont *font, XFontStruct *fontStruct,
                   const char *fontName);

  OStringHashTable fontCache;	// Map a string to an existing OXFont.
				// Keys are CachedFontKey structs, values
				// are OXFont objects.
  OIntHashTable namedTable;	// Map a name to a set of attributes for a
				// font, used when constructing an OXFont
				// from a named font description. Keys are
				// strings (uids), values are ONamedFonts.
  OStringHashTable uidTable;	// Hash table for some used string values
  				// like family names, etc.
  const OXClient *_client;
};


#define FONT_FAMILY     0
#define FONT_SIZE       1
#define FONT_WEIGHT     2
#define FONT_SLANT      3
#define FONT_UNDERLINE  4
#define FONT_OVERSTRIKE 5
#define FONT_NUMFIELDS  6


#endif  // __OXFONT_H
