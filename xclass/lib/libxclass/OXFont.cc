/**************************************************************************

   This file maintains a database of fonts and provides several utility
   procedures for measuring and displaying text. The implementation is
   based on similar tk code.

   The tkFont.c file contains the following copyright notice:

     Copyright (c) 1990-1994 The Regents of the University of California.
     Copyright (c) 1994-1997 Sun Microsystems, Inc.

     See the file "tk.license.terms" for information on usage and
     redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.

     [ tkFont.c 1.74 97/10/10 14:34:11 ]

**************************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include <xclass/utils.h>
#include <xclass/OXClient.h>
#include <xclass/OString.h>
#include <xclass/OTextBuffer.h>
#include <xclass/OHashTable.h>
#include <xclass/OXFont.h>


#ifndef ABS
#define ABS(n)  (((n) < 0) ? -(n) : (n))
#endif
   
#ifndef UCHAR 
#define UCHAR(c) ((unsigned char) (c))
#endif


// The following data structure is used to keep track of the font attributes
// for each named font that has been defined. The named font is only deleted
// when the last reference to it goes away.

class ONamedFont {
public:
  int refCount;			// Number of users of named font.
  int deletePending;		// Non-zero if font should be deleted when
				// last reference goes away.
  OFontAttributes fa;		// Desired attributes for named font.
};


// The following structures are used as two-way maps between the values for
// the fields in the OFontAttributes structure and the strings used when
// parsing both option-value format and style-list format font name strings.

struct StateMap { int numKey; char *strKey; };

static StateMap weightMap[] = {
  { FONT_FW_NORMAL,  "normal" },
  { FONT_FW_BOLD,    "bold"   },
  { FONT_FW_UNKNOWN, NULL     }
};

static StateMap slantMap[] = {
  { FONT_FS_ROMAN,   "roman" },
  { FONT_FS_ITALIC,  "italic" },
  { FONT_FS_UNKNOWN, NULL     }
};

static StateMap underlineMap[] = {
  { 1, "underline" },
  { 0, NULL        }
};

static StateMap overstrikeMap[] = {
  { 1, "overstrike" },
  { 0, NULL         }
};

// The following structures are used when parsing XLFD's into a set of
// OFontAttributes.

static StateMap xlfdWeightMap[] = {
  { FONT_FW_NORMAL, "normal"   },
  { FONT_FW_NORMAL, "medium"   },
  { FONT_FW_NORMAL, "book"     },
  { FONT_FW_NORMAL, "light"    },
  { FONT_FW_BOLD,   "bold"     },
  { FONT_FW_BOLD,   "demi"     },
  { FONT_FW_BOLD,   "demibold" },
  { FONT_FW_NORMAL,  NULL      }	// Assume anything else is "normal".
};

static StateMap xlfdSlantMap[] = {
  { FONT_FS_ROMAN,   "r"  },
  { FONT_FS_ITALIC,  "i"  },
  { FONT_FS_OBLIQUE, "o"  },
  { FONT_FS_ROMAN,   NULL }		// Assume anything else is "roman".
};

static StateMap xlfdSetwidthMap[] = {
  { FONT_SW_NORMAL,   "normal"        },
  { FONT_SW_CONDENSE, "narrow"        },
  { FONT_SW_CONDENSE, "semicondensed" },
  { FONT_SW_CONDENSE, "condensed"     },
  { FONT_SW_UNKNOWN,  NULL            }
};

static StateMap xlfdCharsetMap[] = {
  { FONT_CS_NORMAL, "iso8859" },
  { FONT_CS_SYMBOL, "adobe"   },
  { FONT_CS_SYMBOL, "sun"     },
  { FONT_CS_OTHER,  NULL      }
};


// Possible values for entries in the "types" field in a OXFont structure,
// which classifies the types of all characters in the given font. This
// information is used when measuring and displaying characters.
//
// NORMAL:              Standard character.
// REPLACE:             This character doesn't print: instead of displaying
//                      character, display a replacement sequence like "\n"
//                      (for those characters where ANSI C defines such a
//                      sequence) or a sequence of the form "\xdd" where dd
//                      is the hex equivalent of the character.
// SKIP:                Don't display anything for this character. This is
//                      only used where the font doesn't contain all the
//                      characters needed to generate replacement sequences.

#define NORMAL          0
#define REPLACE         1
#define SKIP            2


// Characters used when displaying control sequences.

static char hexChars[] = "0123456789abcdefxtnvr\\";


// The following table maps some control characters to sequences like '\n'
// rather than '\x10'. A zero entry in the table means no such mapping
// exists, and the table only maps characters less than 0x10.

static char mapChars[] = {
  0, 0, 0, 0, 0, 0, 0, 'a', 'b', 't', 'n', 'v', 'f', 'r', 0
};

static int GetControlCharSubst(int c, char buf[4]);

//--------------------------------------------------------------------------

void OFontAttributes::Init() {
  family = NULL;
  pointsize = 0;
  weight = FONT_FW_NORMAL;
  slant = FONT_FS_ROMAN;
  underline = 0;
  overstrike = 0;
}


//--------------------------------------------------------------------------

OFontPool::~OFontPool() {
  OHashEntry *h;
  OHashSearch search;

  if (fontCache.NumEntries() != 0) {
    Debug(DBG_MISC, "Warning: deleting OFontPool with unfreed fonts.\n");

    for (h = fontCache.FirstHashEntry(&search);
         h != NULL; h = fontCache.NextHashEntry(&search)) {
      OXFont *font = (OXFont *) h->GetHashValue();
      fontCache.DeleteHashEntry(font->cacheHash);
      XFreeFont(_client->GetDisplay(), font->fontStruct);
      delete font;
    }
  }

  // delete fontCache; (if dyn alloc)

  for (h = namedTable.FirstHashEntry(&search);
       h != NULL; h = namedTable.NextHashEntry(&search)) {
    delete (ONamedFont *) h->GetHashValue();
  }
  // delete namedTable;
}


//-----------------------------------------------------------------------

// Given a string, this procedure returns a unique identifier for the
// string.
//
// This procedure returns a pointer to a new char string corresponding to
// the "string" argument. The new string has a value identical to string
// (strcmp will return 0), but it's guaranteed that any other calls to this
// procedure with a string equal to "string" will return exactly the same
// result (i.e. can compare pointer *values* directly, without having to
// call strcmp on what they point to).

const char *OFontPool::GetUid(const char *string) {
  int dummy;

  OHashEntry *h = uidTable.Create(string, &dummy);
  return (const char *) h->GetHashKey();
}


//-------------------------------------------------------------------------

// Return information about the font attributes as an array of strings.
//
// An array of FONT_NUMFIELDS strings is returned holding the value of the
// font attributes in the following order:
//   family size weight slant underline overstrike

char **OFontPool::GetAttributeInfo(const OFontAttributes *fa) {
  int i, num;
  const char *str;

  char **result = new char*[FONT_NUMFIELDS];

  for (i = 0; i < FONT_NUMFIELDS; ++i) {
    str = NULL;
    num = 0;

    switch (i) {
    case FONT_FAMILY:
      str = fa->family;
      if (str == NULL) str = "";
      break;

    case FONT_SIZE:
      num = fa->pointsize;
      break;

    case FONT_WEIGHT:
      str = FindStateString(weightMap, fa->weight);
      break;

    case FONT_SLANT:
      str = FindStateString(slantMap, fa->slant);
      break;

    case FONT_UNDERLINE:
      num = fa->underline;
      break;

    case FONT_OVERSTRIKE:
      num = fa->overstrike;
      break;
    }

    if (str) {
      result[i] = new char[strlen(str)+1];
      strcpy(result[i], str);
    } else {
      result[i] = new char[20];
      sprintf(result[i], "%d", num);
    }
  }

  return result;
}

void OFontPool::FreeAttributeInfo(char **info) {
  int i;

  if (info) {
    for (i = 0; i < FONT_NUMFIELDS; ++i) if (info[i]) delete[] info[i];
    delete[] info;
  }
}


//-------------------------------------------------------------------------

// The return value is token for the font, or NULL if an error prevented
// the font from being created.
//
// The font is added to an internal database with a reference count. For
// each call to this procedure, there should eventually be a call to
// FreeFont() so that the database is cleaned up when fonts aren't in
// use anymore.
//
// string -- String describing font, as: named font, native format, or
//           parseable string.

OXFont *OFontPool::GetFont(const char *string) {
  OHashEntry *cacheHash, *namedHash;
  OXFont *font;
  int inew, descent;
  ONamedFont *nf;

  cacheHash = fontCache.Create(string, &inew);

  if (inew == 0) {

    // We have already constructed a font with this description for
    // this display. Bump the reference count of the cached font.

    font = (OXFont *) cacheHash->GetHashValue();
    font->refCount++;
    return font;
  }

  namedHash = namedTable.Find(string);
  if (namedHash != NULL) {

    // Construct a font based on a named font.

    nf = (ONamedFont *) namedHash->GetHashValue();
    nf->refCount++;

    font = GetFontFromAttributes(&nf->fa, NULL);

  } else {

    // Native font?

    font = GetNativeFont(string);
    if (font == NULL) {
      OFontAttributes fa;

      if (ParseFontName(string, &fa) != 0) {
	fontCache.DeleteHashEntry(cacheHash); // delete cacheHash;
	return NULL;
      }

      // String contained the attributes inline.

      font = GetFontFromAttributes(&fa, NULL);
    }
  }
  cacheHash->SetHashValue(font);

  font->refCount = 1;
  font->cacheHash = cacheHash;
  font->namedHash = namedHash;

  font->MeasureChars("0", 1, 0, 0, &font->tabWidth);
  if (font->tabWidth == 0) font->tabWidth = font->fm.maxWidth;
  font->tabWidth *= 8;

  // Make sure the tab width isn't zero (some fonts may not have enough
  // information to set a reasonable tab width).

  if (font->tabWidth == 0) font->tabWidth = 1;

  // Get information used for drawing underlines in generic code on a
  // non-underlined font.

  descent = font->fm.descent;
  font->underlinePos = descent / 2;  // ==!== could be set by MakeFont()
  font->underlineHeight = font->fa.pointsize / 10;
  if (font->underlineHeight == 0) font->underlineHeight = 1;
  if (font->underlinePos + font->underlineHeight > descent) {

    // If this set of values would cause the bottom of the underline
    // bar to stick below the descent of the font, jack the underline
    // up a bit higher.

    font->underlineHeight = descent - font->underlinePos;
    if (font->underlineHeight == 0) {
      font->underlinePos--;
      font->underlineHeight = 1;
    }
  }

  return font;
}

OXFont *OFontPool::GetFont(const char *family, int ptsize, int weight,
                           int slant) {
  char *s, tmp[strlen(family)+100];

  sprintf(tmp, "%s %d", family, ptsize);
  s = FindStateString(weightMap, weight);
  if (s) { strcat(tmp, " "); strcat(tmp, s); }
  s = FindStateString(slantMap, slant);
  if (s) { strcat(tmp, " "); strcat(tmp, s); }
  return GetFont(tmp);
}

OXFont *OFontPool::GetFont(OXFont *font) {
  if (font) font->refCount++;
  return font;
}


//--------------------------------------------------------------------------

// Called to release a font allocated by GetFont().
//
// The reference count associated with font is decremented, and only
// deallocated when no one is using it.

void OFontPool::FreeFont(OXFont *font) {
  ONamedFont *nf;

  if (font == NULL) return;
  font->refCount--;
  if (font->refCount == 0) {
    if (font->namedHash != NULL) {

      // The font is being deleted. Determine if the associated named
      // font definition should and/or can be deleted too.

      nf = (ONamedFont *) font->namedHash->GetHashValue();
      nf->refCount--;
      if ((nf->refCount == 0) && (nf->deletePending != 0)) {
	namedTable.DeleteHashEntry(font->namedHash);
	delete nf;
      }
    }
    fontCache.DeleteHashEntry(font->cacheHash);
    XFreeFont(_client->GetDisplay(), font->fontStruct);
    delete font;
  }
}


//--------------------------------------------------------------------------

// Converts a string into a set of font attributes that can be used to
// construct a font.
//
// The string can be one of the following forms:
//        XLFD (see X documentation)
//        "Family [size [style] [style ...]]"
//
// The return value is an error code if the object was syntactically
// invalid. Otherwise, fills the font attribute buffer with the values
// parsed from the string and returns 0. The structure must already be
// properly initialized.

#define FONT_ERROR_NO_SUCH_FONT    -1
#define FONT_ERROR_UNKNOWN_STYLE   -2

static char *getToken(char *str) {
  static char *p = NULL;
  char *retp;

  if (str) p = str;

  if (!p) return NULL;
  if (!*p) return NULL;

  while (*p && ((*p == ' ') || (*p == '\t'))) ++p;  // skip spaces
  if (!*p) return NULL;

  if (*p == '"') {  // quoted string
    retp = ++p;
    if (!*p) return NULL;
    while (*p && (*p != '"')) ++p;
    if (*p == '"') *p++ = '\0';
  } else {
    retp = p;
    while (*p && (*p != ' ') && (*p != '\t')) ++p;
    if (*p) *p++ = '\0';
  }

  return retp;
}

int OFontPool::ParseFontName(const char *string, OFontAttributes *fa) {
  char *s;
  int n, result;
  OXLFDAttributes xa;

  char *str = new char[strlen(string)+1];
  strcpy(str, string);

  if (*str == '-' || *str == '*') {

    // This appears to be an XLFD.

    xa.fa = *fa;
    result = ParseXLFD(str, &xa);
    if (result == True) {
      *fa = xa.fa;
      delete[] str;
      return 0;
    }
  }

  // Wasn't an XLFD or "-option value" string. Try it as a
  // "font size style" list.

  s = getToken(str); // strtok(str, " ");
  if (s == NULL) {
    delete[] str;
    return FONT_ERROR_NO_SUCH_FONT;
  }
  fa->family = GetUid(s);
  s = getToken(NULL); // strtok(NULL, " ");
  if (s != NULL) {
    char *end;

    fa->pointsize = strtol(s, &end, 0);
    if (errno == ERANGE || end == s) return FONT_ERROR_NO_SUCH_FONT;
  }

  while ((s = getToken(NULL) /*strtok(NULL, " ")*/) != NULL) {
    n = FindStateNum(weightMap, s);
    if (n != FONT_FW_UNKNOWN) {
      fa->weight = n;
      continue;
    }
    n = FindStateNum(slantMap, s);
    if (n != FONT_FS_UNKNOWN) {
      fa->slant = n;
      continue;
    }
    n = FindStateNum(underlineMap, s);
    if (n != 0) {
      fa->underline = n;
      continue;
    }
    n = FindStateNum(overstrikeMap, s);
    if (n != 0) {
      fa->overstrike = n;
      continue;
    }

    // Unknown style.

    delete[] str;
    return FONT_ERROR_UNKNOWN_STYLE;
  }

  delete[] str;
  return 0;
}


//--------------------------------------------------------------------------

// Break up a fully specified XLFD into a set of font attributes.
//
// Return value is False if string was not a fully specified XLFD.
// Otherwise, fills font attribute buffer with the values parsed from
// the XLFD and returns True.
//
// string -- Parseable font description string.
// xa     -- XLFD attributes structure whose fields are to be modified.
//           Structure must already be properly initialized.

int OFontPool::ParseXLFD(const char *string, OXLFDAttributes *xa) {
  char *src;
  const char *str;
  int i, j;
  char *field[XLFD_NUMFIELDS + 2];
  OString ds("");

  memset(field, '\0', sizeof (field));

  str = string;
  if (*str == '-') str++;

  ds.Append((char *) str);
  src = (char *) ds.GetString();

  field[0] = src;
  for (i = 0; *src != '\0'; src++) {
    if (isupper(UCHAR(*src))) {
      *src = tolower(UCHAR(*src));
    }
    if (*src == '-') {
      i++;
      if (i > XLFD_NUMFIELDS) {
	break;
      }
      *src = '\0';
      field[i] = src + 1;
    }
  }

  // An XLFD of the form -adobe-times-medium-r-*-12-*-* is pretty common, 
  // but it is (strictly) malformed, because the first * is eliding both
  // the Setwidth and the Addstyle fields. If the Addstyle field is a
  // number, then assume the above incorrect form was used and shift all
  // the rest of the fields up by one, so the number gets interpreted
  // as a pixelsize.

  if ((i > XLFD_ADD_STYLE) && (FieldSpecified(field[XLFD_ADD_STYLE]))) {
    if (atoi(field[XLFD_ADD_STYLE]) != 0) {
      for (j = XLFD_NUMFIELDS - 1; j >= XLFD_ADD_STYLE; j--) {
	field[j + 1] = field[j];
      }
      field[XLFD_ADD_STYLE] = NULL;
      i++;
    }
  }

  // Bail if we don't have enough of the fields (up to pointsize).

  if (i < XLFD_FAMILY) {
    return False;
  }
  if (FieldSpecified(field[XLFD_FOUNDRY])) {
    xa->foundry = GetUid(field[XLFD_FOUNDRY]);
  }
  if (FieldSpecified(field[XLFD_FAMILY])) {
    xa->fa.family = GetUid(field[XLFD_FAMILY]);
  }
  if (FieldSpecified(field[XLFD_WEIGHT])) {
    xa->fa.weight = FindStateNum(xlfdWeightMap, field[XLFD_WEIGHT]);
  }
  if (FieldSpecified(field[XLFD_SLANT])) {
    xa->slant = FindStateNum(xlfdSlantMap, field[XLFD_SLANT]);
    if (xa->slant == FONT_FS_ROMAN) {
      xa->fa.slant = FONT_FS_ROMAN;
    } else {
      xa->fa.slant = FONT_FS_ITALIC;
    }
  }
  if (FieldSpecified(field[XLFD_SETWIDTH])) {
    xa->setwidth = FindStateNum(xlfdSetwidthMap, field[XLFD_SETWIDTH]);
  }
  // XLFD_ADD_STYLE ignored.

  // Pointsize in tenths of a point, but treat it as tenths of a pixel.

  if (FieldSpecified(field[XLFD_POINT_SIZE])) {
    if (field[XLFD_POINT_SIZE][0] == '[') {

      // Some X fonts have the point size specified as follows:
      //
      //      [ N1 N2 N3 N4 ]
      //
      // where N1 is the point size (in points, not decipoints!), and
      // N2, N3, and N4 are some additional numbers that I don't know
      // the purpose of, so I ignore them.

      xa->fa.pointsize = atoi(field[XLFD_POINT_SIZE] + 1);
    } else {
      char *end;

      xa->fa.pointsize = strtol(field[XLFD_POINT_SIZE], &end, 0);
      if (errno == ERANGE || end == field[XLFD_POINT_SIZE]) return False;
      xa->fa.pointsize /= 10;
    }
  }

  // Pixel height of font. If specified, overrides pointsize.

  if (FieldSpecified(field[XLFD_PIXEL_SIZE])) {
    if (field[XLFD_PIXEL_SIZE][0] == '[') {

      // Some X fonts have the pixel size specified as follows:
      //
      //      [ N1 N2 N3 N4 ]
      //
      // where N1 is the pixel size, and where N2, N3, and N4 
      // are some additional numbers that I don't know
      // the purpose of, so I ignore them.

      xa->fa.pointsize = atoi(field[XLFD_PIXEL_SIZE] + 1);
    } else {
      char *end;

      xa->fa.pointsize = strtol(field[XLFD_PIXEL_SIZE], &end, 0);
      if (errno == ERANGE || end == field[XLFD_PIXEL_SIZE]) return False;
    }
  }
  xa->fa.pointsize = -xa->fa.pointsize;

  // XLFD_RESOLUTION_X ignored.

  // XLFD_RESOLUTION_Y ignored.

  // XLFD_SPACING ignored.

  // XLFD_AVERAGE_WIDTH ignored.

  if (FieldSpecified(field[XLFD_REGISTRY])) {
    xa->charset = FindStateNum(xlfdCharsetMap, field[XLFD_REGISTRY]);
  }
  if (FieldSpecified(field[XLFD_ENCODING])) {
    xa->encoding = atoi(field[XLFD_ENCODING]);
  }

  return True;
}


//--------------------------------------------------------------------------

// Given a lookup table, map a string to a number in the table.
//
// If strKey was equal to the string keys of one of the elements in the
// table, returns the numeric key of that element. Returns the numKey
// associated with the last element (the NULL string one) in the table
// if strKey was not equal to any of the string keys in the table.

int OFontPool::FindStateNum(const StateMap *map, const char *strKey) {
  const StateMap *m;

  if (map->strKey == NULL) {
    FatalError("OFontPool::FindStateNum(): no choices in lookup table");
  }

  for (m = map; m->strKey != NULL; m++) {
    if (strcasecmp(strKey, m->strKey) == 0) return m->numKey;
  }
  return m->numKey;
}


//--------------------------------------------------------------------------

// Given a lookup table, map a number to a string in the table.
//
// If numKey was equal to the numeric key of one of the elements in the
// table, returns the string key of that element. Returns NULL if numKey
// was not equal to any of the numeric keys in the table.

char *OFontPool::FindStateString(const StateMap *map, int numKey) {
  for ( ; map->strKey != NULL; map++) {
    if (numKey == map->numKey) return map->strKey;
  }
  return NULL;
}


//-------------------------------------------------------------------------

// Helper function for ParseXLFD(). Determines if a field in the XLFD was
// set to a non-null, non-don't-care value.
//
// The return value is False if the field in the XLFD was not set and
// should be ignored, True otherwise.
//
// field -- The field of the XLFD to check. Strictly speaking, only when
//          the string is "*" does it mean don't-care. However, an
//          unspecified or question mark is also interpreted as don't-care.

int OFontPool::FieldSpecified(const char *field) {
  char ch;

  if (field == NULL) return False;
  ch = field[0];
  return (ch != '*' && ch != '?');
}


//-------------------------------------------------------------------------

// Given a font, return a textual string identifying it.
//
// The return value is the description that was passed to GetFont()
// to create the font. The storage for the returned string is only
// guaranteed to persist until the font is deleted. The caller should
// not modify this string.

char *OFontPool::NameOfFont(OXFont *font) {
  return font->NameOfFont();
}

char *OXFont::NameOfFont() const {
  return (char *) cacheHash->GetHashKey();
}


//--------------------------------------------------------------------------

// Returns overall ascent and descent metrics for the given font. These
// values can be used to space multiple lines of text and to align the
// baselines of text in different fonts.
//
// fm->linespace is filled with the overall height of the font, which is
// the sum of the ascent and descent. fm->ascent and fm->descent are filled
// with the ascent and descent information for the font, respectively.

void OXFont::GetFontMetrics(OFontMetrics *m) const {
  m->ascent = fm.ascent;
  m->descent = fm.descent;
  m->linespace = fm.ascent + fm.descent;
  m->maxWidth = fm.maxWidth;
  m->fixed = fm.fixed;
}

void OXFont::GetFontAttributes(OFontAttributes *a) const {
  *a = fa;
}


//--------------------------------------------------------------------------

// Return the name of the corresponding Postscript font for this OXFont.
//
// The return value is the pointsize of the OXFont. The name of the
// Postscript font is appended to ds.
//
// If the font does not exist on the printer, the print job will fail at
// print time. Given a "reasonable" Postscript printer, the following
// OXFont font families should print correctly:
//
//     Avant Garde, Arial, Bookman, Courier, Courier New, Geneva,
//     Helvetica, Monaco, New Century Schoolbook, New York,
//     Palatino, Symbol, Times, Times New Roman, Zapf Chancery,
//     and Zapf Dingbats.
//
// Any other OXFont font families may not print correctly because the
// computed Postscript font name may be incorrect.
//
// dst -- Pointer to an initialized OString object to which the name of the
//        Postscript font that corresponds to the font will be appended.

int OXFont::PostscriptFontName(OString *dst) const {
  const char *family;
  char *weightString, *slantString;
  char *src, *dest;
  int upper, len;

  len = dst->GetLength();

  // Convert the case-insensitive OXFont family name to the
  // case-sensitive Postscript family name. Take out any spaces and
  // capitalize the first letter of each word.

  family = fa.family;
  if (strncasecmp(family, "itc ", 4) == 0) {
    family = family + 4;
  }
  if ((strcasecmp(family, "Arial") == 0)
      || (strcasecmp(family, "Geneva") == 0)) {
    family = "Helvetica";
  } else if ((strcasecmp(family, "Times New Roman") == 0)
	     || (strcasecmp(family, "New York") == 0)) {
    family = "Times";
  } else if ((strcasecmp(family, "Courier New") == 0)
	     || (strcasecmp(family, "Monaco") == 0)) {
    family = "Courier";
  } else if (strcasecmp(family, "AvantGarde") == 0) {
    family = "AvantGarde";
  } else if (strcasecmp(family, "ZapfChancery") == 0) {
    family = "ZapfChancery";
  } else if (strcasecmp(family, "ZapfDingbats") == 0) {
    family = "ZapfDingbats";
  } else {

    // Inline, capitalize the first letter of each word, lowercase the
    // rest of the letters in each word, and then take out the spaces
    // between the words. This may make the OString shorter, which is
    // safe to do.

    dst->Append(family);

    src = dest = (char *) dst->GetString() + len;
    upper = 1;
    for (; *src != '\0'; src++, dest++) {
      while (isspace(UCHAR(*src))) {
	src++;
	upper = 1;
      }
      *dest = *src;
      if ((upper != 0) && (islower(UCHAR(*src)))) {
	*dest = toupper(UCHAR(*src));
      }
      upper = 0;
    }
    *dest = '\0';
    dst->SetLength(dest - dst->GetString()); // dst->ResetLength(); may be better
    family = (char *) dst->GetString() + len;
  }
  if (family != (char *) dst->GetString() + len) {
    dst->Append(family);
    family = (char *) dst->GetString() + len;
  }
  if (strcasecmp(family, "NewCenturySchoolbook") == 0) {
    dst->SetLength(len);
    dst->Append("NewCenturySchlbk");
    family = (char *) dst->GetString() + len;
  }

  // Get the string to use for the weight.

  weightString = NULL;
  if (fa.weight == FONT_FW_NORMAL) {
    if (strcmp(family, "Bookman") == 0) {
      weightString = "Light";
    } else if (strcmp(family, "AvantGarde") == 0) {
      weightString = "Book";
    } else if (strcmp(family, "ZapfChancery") == 0) {
      weightString = "Medium";
    }
  } else {
    if ((strcmp(family, "Bookman") == 0)
	|| (strcmp(family, "AvantGarde") == 0)) {
      weightString = "Demi";
    } else {
      weightString = "Bold";
    }
  }

  // Get the string to use for the slant.

  slantString = NULL;
  if (fa.slant == FONT_FS_ROMAN) {
    ;
  } else {
    if ((strcmp(family, "Helvetica") == 0)
	|| (strcmp(family, "Courier") == 0)
	|| (strcmp(family, "AvantGarde") == 0)) {
      slantString = "Oblique";
    } else {
      slantString = "Italic";
    }
  }

  // The string "Roman" needs to be added to some fonts that are not bold
  // and not italic.

  if ((slantString == NULL) && (weightString == NULL)) {
    if ((strcmp(family, "Times") == 0)
	|| (strcmp(family, "NewCenturySchlbk") == 0)
	|| (strcmp(family, "Palatino") == 0)) {
      dst->Append("-Roman");
    }
  } else {
    dst->Append("-");
    if (weightString != NULL) dst->Append(weightString);
    if (slantString != NULL) dst->Append(slantString);
  }

  return fa.pointsize;
}


//---------------------------------------------------------------------------

// A wrapper function for the more complicated interface of MeasureChars.
// Computes how much space the given simple string needs.
//
// The return value is the width (in pixels) of the given string.
//
// string   -- String whose width will be computed.
// numChars -- Number of characters to consider from string, or < 0 for
//             strlen().

int OXFont::TextWidth(const char *string, int numChars) const {
  int width;

  if (numChars < 0) numChars = strlen(string);
  MeasureChars(string, numChars, 0, 0, &width);

  return width;
}

int OXFont::XTextWidth(const char *string, int numChars) const {
  int width;

  if (numChars < 0) numChars = strlen(string);
  width = ::XTextWidth(fontStruct, string, numChars);

  return width;
}

//--------------------------------------------------------------------------

// This procedure draws an underline for a given range of characters in a
// given string. It doesn't draw the characters (which are assumed to have
// been displayed previously); it just draws the underline. This procedure
// would mainly be used to quickly underline a few characters without having
// to construct an underlined font. To produce properly underlined text, the
// appropriate underlined font should be constructed and used. 
//
// dpy       -- Display on which to draw.
// dst       -- Window or pixmap in which to draw.
// gc        -- Graphics context for actually drawing line.
// string    -- String containing characters to be underlined or overstruck.
// x, y      -- Coordinates at which first character of string is drawn.
// firstChar -- Index of first character.
// lastChar  -- Index of one after the last character.

void OXFont::UnderlineChars(Display *dpy, Drawable dst, GC gc,
                            const char *string, int x, int y,
                            int firstChar, int lastChar) const {
  int startX, endX;

  MeasureChars(string, firstChar, 0, 0, &startX);
  MeasureChars(string, lastChar, 0, 0, &endX);

  XFillRectangle(dpy, dst, gc, x + startX, y + underlinePos,
                 (unsigned int) (endX - startX),
		 (unsigned int) underlineHeight);
}


//--------------------------------------------------------------------------

// Computes the amount of screen space needed to display a multi-line,
// justified string of text. Records all the measurements that were done
// to determine to size and positioning of the individual lines of text;
// this information can be used by the OTextLayout::DrawText() procedure
// to display the text quickly (without remeasuring it).
//
// This procedure is useful for simple widgets that want to display
// single-font, multi-line text and want OXFont to handle the details.
//
// The return value is a OTextLayout token that holds the measurement
// information for the given string. The token is only valid for the given
// string. If the string is freed, the token is no longer valid and must
// also be deleted.
// 
// The dimensions of the screen area needed to display the text are stored
// in *width and *height.
//
// string     -- String whose dimensions are to be computed.
// numChars   -- Number of characters to consider from string, or < 0 for
//               strlen().
// wrapLength -- Longest permissible line length, in pixels. <= 0 means no
//               automatic wrapping: just let lines get as long as needed.
// justify    -- How to justify lines.
// flags      -- Flag bits OR-ed together. TEXT_IGNORE_TABS means that tab
//               characters should not be expanded. TEXT_IGNORE_NEWLINES
//               means that newline characters should not cause a line break.
// width      -- Filled with width of string.
// height     -- Filled with height of string.

OTextLayout *OXFont::ComputeTextLayout(const char *string, int numChars,
                       int wrapLength, int justify, int flags,
		       int *width, int *height) const {
  const char *start, *end, *special;
  int n, y, charsThisChunk, maxChunks;
  int baseline, h, curX, newX, maxWidth;
  OTextLayout *layout;
  OLayoutChunk *chunk;

#define MAX_LINES 50
  int staticLineLengths[MAX_LINES];
  int *lineLengths;
  int maxLines, curLine, layoutHeight;

  lineLengths = staticLineLengths;
  maxLines = MAX_LINES;

  h = fm.ascent + fm.descent;

  if (numChars < 0) numChars = strlen(string);
  maxChunks = 0;

  layout = new OTextLayout;
  layout->font = this;
  layout->string = string;
  layout->numChunks = 0;
  layout->chunks = NULL;

  baseline = fm.ascent;
  maxWidth = 0;

  // Divide the string up into simple strings and measure each string.

  curX = 0;

  end = string + numChars;
  special = string;

  flags &= TEXT_IGNORE_TABS | TEXT_IGNORE_NEWLINES;
  flags |= TEXT_WHOLE_WORDS | TEXT_AT_LEAST_ONE;
  curLine = 0;
  for (start = string; start < end;) {
    if (start >= special) {

      // Find the next special character in the string.

      for (special = start; special < end; special++) {
	if (!(flags & TEXT_IGNORE_NEWLINES)) {
	  if ((*special == '\n') || (*special == '\r')) {
	    break;
	  }
	}
	if (!(flags & TEXT_IGNORE_TABS)) {
	  if (*special == '\t') {
	    break;
	  }
	}
      }
    }

    // Special points at the next special character (or the end of the
    // string). Process characters between start and special.

    chunk = NULL;
    if (start < special) {
      charsThisChunk = MeasureChars(start, special - start,
				    wrapLength - curX, flags, &newX);
      newX += curX;
      flags &= ~TEXT_AT_LEAST_ONE;
      if (charsThisChunk > 0) {
	chunk = NewChunk(layout, &maxChunks, start,
			 charsThisChunk, curX, newX, baseline);

	start += charsThisChunk;
	curX = newX;
      }
    }
    if ((start == special) && (special < end)) {

      // Handle the special character.

      chunk = NULL;
      if (*special == '\t') {
	newX = curX + tabWidth;
	newX -= newX % tabWidth;
	NewChunk(layout, &maxChunks, start, 1, curX, newX,
		 baseline)->numDisplayChars = -1;
	start++;
	if ((start < end) &&
	    ((wrapLength <= 0) || (newX <= wrapLength))) {

	  // More chars can still fit on this line.

	  curX = newX;
	  flags &= ~TEXT_AT_LEAST_ONE;
	  continue;
	}
      } else {
	NewChunk(layout, &maxChunks, start, 1, curX, 1000000000,
		 baseline)->numDisplayChars = -1;
	start++;
	goto wrapLine;
      }
    }

    // No more characters are going to go on this line, either because
    // no more characters can fit or there are no more characters left.
    // Consume all extra spaces at end of line.  

    while ((start < end) && isspace(UCHAR(*start))) {
      if (!(flags & TEXT_IGNORE_NEWLINES)) {
	if ((*start == '\n') || (*start == '\r')) {
	  break;
	}
      }
      if (!(flags & TEXT_IGNORE_TABS)) {
	if (*start == '\t') {
	  break;
	}
      }
      start++;
    }
    if (chunk != NULL) {

      // Append all the extra spaces on this line to the end of the
      // last text chunk.

      charsThisChunk = start - (chunk->start + chunk->numChars);
      if (charsThisChunk > 0) {
	chunk->numChars += MeasureChars(chunk->start + chunk->numChars,
                                        charsThisChunk,
				        0, 0, &chunk->totalWidth);
	chunk->totalWidth += curX;
      }
    }
  wrapLine:
    flags |= TEXT_AT_LEAST_ONE;

    // Save current line length, then move current position to start of
    // next line.

    if (curX > maxWidth) maxWidth = curX;

    // Remember width of this line, so that all chunks on this line
    // can be centered or right justified, if necessary.

    if (curLine >= maxLines) {
      int *newLengths;

      newLengths = new int[2 * maxLines];
      memcpy((void *) newLengths, lineLengths, maxLines * sizeof (int));

      if (lineLengths != staticLineLengths) {
	delete[] lineLengths;
      }
      lineLengths = newLengths;
      maxLines *= 2;
    }
    lineLengths[curLine] = curX;
    curLine++;

    curX = 0;
    baseline += h;
  }

  // If last line ends with a newline, then we need to make a 0 width
  // chunk on the next line. Otherwise "Hello" and "Hello\n" are the
  // same height.

  if ((layout->numChunks > 0) && ((flags & TEXT_IGNORE_NEWLINES) == 0)) {
    if (layout->chunks[layout->numChunks - 1].start[0] == '\n') {
      chunk = NewChunk(layout, &maxChunks, start, 0, curX,
		       1000000000, baseline);
      chunk->numDisplayChars = -1;
      baseline += h;
    }
  }

  // Using maximum line length, shift all the chunks so that the lines are
  // all justified correctly.

  curLine = 0;
  chunk = layout->chunks;
  if (chunk) y = chunk->y;
  for (n = 0; n < layout->numChunks; n++) {
    int extra;

    if (chunk->y != y) {
      curLine++;
      y = chunk->y;
    }
    extra = maxWidth - lineLengths[curLine];
    if (justify == TEXT_JUSTIFY_CENTER) {
      chunk->x += extra / 2;
    } else if (justify == TEXT_JUSTIFY_RIGHT) {
      chunk->x += extra;
    }
    ++chunk;
  }

  layout->width = maxWidth;
  layoutHeight = baseline - fm.ascent;
  if (layout->numChunks == 0) {
    layoutHeight = h;

    // This fake chunk is used by the other procedures so that they can
    // pretend that there is a chunk with no chars in it, which makes
    // the coding simpler.

    layout->numChunks = 1;
    layout->chunks = new OLayoutChunk[1];
    layout->chunks[0].start = string;
    layout->chunks[0].numChars = 0;
    layout->chunks[0].numDisplayChars = -1;
    layout->chunks[0].x = 0;
    layout->chunks[0].y = fm.ascent;
    layout->chunks[0].totalWidth = 0;
    layout->chunks[0].displayWidth = 0;
  }
  if (width != NULL) *width = layout->width;
  if (height != NULL) *height = layoutHeight;

  if (lineLengths != staticLineLengths) {
    delete[] lineLengths;
  }

  return layout;
}


//--------------------------------------------------------------------------

OTextLayout::~OTextLayout() {
  if (chunks) delete[] chunks;
}


//--------------------------------------------------------------------------

// Use the information in the OTextLayout object to display a multi-line,
// justified string of text.
//
// This procedure is useful for simple widgets that need to display
// single-font, multi-line text and want OXFont to handle the details.
//
// dpy       -- Display on which to draw.
// dst       -- Window or pixmap in which to draw.
// gc        -- Graphics context to use for drawing text.
// x, y      -- Upper-left hand corner of rectangle in which to draw
//              (pixels).
// firstChar -- The index of the first character to draw from the given
//              text item. 0 specfies the beginning.
// lastChar  -- The index just after the last character to draw from the
//              given text item. A number < 0 means to draw all characters.

void OTextLayout::DrawText(Display *dpy, Drawable dst, GC gc,
                           int x, int y, int firstChar, int lastChar) const {
  int i, numDisplayChars, drawX;
  OLayoutChunk *chunk;

  if (lastChar < 0) lastChar = 100000000;
  chunk = chunks;
  for (i = 0; i < numChunks; i++) {
    numDisplayChars = chunk->numDisplayChars;
    if ((numDisplayChars > 0) && (firstChar < numDisplayChars)) {
      if (firstChar <= 0) {
	drawX = 0;
	firstChar = 0;
      } else {
	font->MeasureChars(chunk->start, firstChar, 0, 0, &drawX);
      }
      if (lastChar < numDisplayChars) numDisplayChars = lastChar;
      font->DrawChars(dpy, dst, gc,
		chunk->start + firstChar, numDisplayChars - firstChar,
		x + chunk->x + drawX, y + chunk->y);
    }
    firstChar -= chunk->numChars;
    lastChar -= chunk->numChars;
    if (lastChar <= 0) break;
    chunk++;
  }
}


//-------------------------------------------------------------------------

// Use the information in the OTextLayout object to display an underline
// below an individual character. This procedure does not draw the text,
// just the underline.
//
// This procedure is useful for simple widgets that need to display
// single-font, multi-line text with an individual character underlined
// and want OXFont to handle the details. To display larger amounts of
// underlined text, construct and use an underlined font.
//
// dpy       -- Display on which to draw.
// dst       -- Window or pixmap in which to draw.
// gc        -- Graphics context to use for drawing text.
// x, y      -- Upper-left hand corner of rectangle in which to draw
//              (pixels).
// underline -- Index of the single character to underline, or -1 for
//              no underline.

void OTextLayout::UnderlineChar(Display *dpy, Drawable dst, GC gc,
                                int x, int y, int underline) const {
  int xx, yy, width, height;

  if ((CharBbox(underline, &xx, &yy, &width, &height) != 0)
      && (width != 0)) {
    XFillRectangle(dpy, dst, gc, x + xx,
		   y + yy + font->fm.ascent + font->underlinePos,
	  (unsigned int) width, (unsigned int) font->underlineHeight);
  }
}


//-------------------------------------------------------------------------

// Use the information in the OTextLayout token to determine the character
// closest to the given point. The point must be specified with respect to
// the upper-left hand corner of the text layout, which is considered to be
// located at (0, 0).
//
// Any point whose y-value is less that 0 will be considered closest to the
// first character in the text layout; any point whose y-value is greater
// than the height of the text layout will be considered closest to the last
// character in the text layout.
//
// Any point whose x-value is less than 0 will be considered closest to the
// first character on that line; any point whose x-value is greater than the
// width of the text layout will be considered closest to the last character
// on that line.
//
// The return value is the index of the character that was closest to the
// point. Given a text layout with no characters, the value 0 will always
// be returned, referring to a hypothetical zero-width placeholder character.

int OTextLayout::PointToChar(int x, int y) const {
  OLayoutChunk *chunk, *last;
  int i, n, dummy, baseline, pos;

  if (y < 0) {

    // Point lies above any line in this layout. Return the index of
    // the first char.

    return 0;
  }

  // Find which line contains the point.

  last = chunk = chunks;
  for (i = 0; i < numChunks; i++) {
    baseline = chunk->y;
    if (y < baseline + font->fm.descent) {
      if (x < chunk->x) {

	// Point is to the left of all chunks on this line. Return
	// the index of the first character on this line.

	return chunk->start - string;
      }
      if (x >= width) {

	// If point lies off right side of the text layout, return
	// the last char in the last chunk on this line. Without
	// this, it might return the index of the first char that
	// was located outside of the text layout.

	x = INT_MAX;
      }

      // Examine all chunks on this line to see which one contains
      // the specified point.

      last = chunk;
      while ((i < numChunks) && (chunk->y == baseline)) {
	if (x < chunk->x + chunk->totalWidth) {

	  // Point falls on one of the characters in this chunk.

	  if (chunk->numDisplayChars < 0) {

	    // This is a special chunk that encapsulates a single
	    // tab or newline char.

	    return chunk->start - string;
	  }
	  n = font->MeasureChars(chunk->start, chunk->numChars,
                                 x + 1 - chunk->x, TEXT_PARTIAL_OK, &dummy);
	  return (chunk->start + n - 1) - string;
	}
	last = chunk;
	chunk++;
	i++;
      }

      // Point is to the right of all chars in all the chunks on this
      // line. Return the index just past the last char in the last
      // chunk on this line.

      pos = (last->start + last->numChars) - string;
      if (i < numChunks) pos--;
      return pos;
    }
    last = chunk;
    chunk++;
  }

  // Point lies below any line in this text layout. Return the index
  // just past the last char.

  return (last->start + last->numChars) - string;
}


//--------------------------------------------------------------------------

// Use the information in the OTextLayout token to return the bounding box
// for the character specified by index.  
//
// The width of the bounding box is the advance width of the character, and
// does not include and left- or right-bearing. Any character that extends
// partially outside of the text layout is considered to be truncated at the
// edge. Any character which is located completely outside of the text
// layout is considered to be zero-width and pegged against the edge.
//
// The height of the bounding box is the line height for this font,
// extending from the top of the ascent to the bottom of the descent.
// Information about the actual height of the individual letter is not
// available.
//
// A text layout that contains no characters is considered to contain a
// single zero-width placeholder character.
//
// The return value is 0 if the index did not specify a character in the
// text layout, or non-zero otherwise. In that case, *bbox is filled with
// the bounding box of the character.
//
// layout -- Layout information, from a previous call to ComputeTextLayout().
// index  -- The index of the character whose bbox is desired.
// x, y   -- Filled with the upper-left hand corner, in pixels, of the
//           bounding box for the character specified by index, if non-NULL.
// w, h   -- Filled with the width and height of the bounding box for the
//           character specified by index, if non-NULL.

int OTextLayout::CharBbox(int index, int *x, int *y, int *w, int *h) const {
  OLayoutChunk *chunk;
  int i, xx, ww;

  if (index < 0) return 0;

  chunk = chunks;

  for (i = 0; i < numChunks; i++) {
    if (chunk->numDisplayChars < 0) {
      if (index == 0) {
	xx = chunk->x;
	ww = chunk->totalWidth;
	goto check;
      }
    } else if (index < chunk->numChars) {
      if (x != NULL) {
	font->MeasureChars(chunk->start, index, 0, 0, &xx);
	xx += chunk->x;
      }
      if (w != NULL) {
	font->MeasureChars(chunk->start + index, 1, 0, 0, &ww);
      }
      goto check;
    }
    index -= chunk->numChars;
    chunk++;
  }
  if (index == 0) {

    // Special case to get location just past last char in layout.

    chunk--;
    xx = chunk->x + chunk->totalWidth;
    ww = 0;
  } else {
    return 0;
  }

  // Ensure that the bbox lies within the text layout. This forces all
  // chars that extend off the right edge of the text layout to have
  // truncated widths, and all chars that are completely off the right
  // edge of the text layout to peg to the edge and have 0 width.

check:
  if (y != NULL) {
    *y = chunk->y - font->fm.ascent;
  }
  if (h != NULL) {
    *h = font->fm.ascent + font->fm.descent;
  }
  if (xx > width) {
    xx = width;
  }
  if (x != NULL) {
    *x = xx;
  }
  if (w != NULL) {
    if (xx + ww > width) {
      ww = width - xx;
    }
    *w = ww;
  }
  return 1;
}


//--------------------------------------------------------------------------

// Computes the distance in pixels from the given point to the given
// text layout. Non-displaying space characters that occur at the end of
// individual lines in the text layout are ignored for hit detection
// purposes.
//
// The return value is 0 if the point (x, y) is inside the text layout.
// If the point isn't inside the text layout then the return value is the
// distance in pixels from the point to the text item.
//
// x, y -- Coordinates of point to check, with respect to the upper-left
//         corner of the text layout (in pixels).

#include <math.h>

int OTextLayout::DistanceToText(int x, int y) const {
  int i, x1, x2, y1, y2, xDiff, yDiff, dist, minDist, ascent, descent;
  OLayoutChunk *chunk;

  ascent = font->fm.ascent;
  descent = font->fm.descent;

  minDist = 0;
  chunk = chunks;
  for (i = 0; i < numChunks; i++) {
    if (chunk->start[0] == '\n') {

      // Newline characters are not counted when computing distance
      // (but tab characters would still be considered).

      chunk++;
      continue;
    }
    x1 = chunk->x;
    y1 = chunk->y - ascent;
    x2 = chunk->x + chunk->displayWidth;
    y2 = chunk->y + descent;

    if (x < x1) {
      xDiff = x1 - x;
    } else if (x >= x2) {
      xDiff = x - x2 + 1;
    } else {
      xDiff = 0;
    }

    if (y < y1) {
      yDiff = y1 - y;
    } else if (y >= y2) {
      yDiff = y - y2 + 1;
    } else {
      yDiff = 0;
    }
    if ((xDiff == 0) && (yDiff == 0)) {
      return 0;
    }
    dist = (int) hypot((double) xDiff, (double) yDiff);
    if ((dist < minDist) || (minDist == 0)) {
      minDist = dist;
    }
    chunk++;
  }
  return minDist;
}


//--------------------------------------------------------------------------

// Determines whether a text layout lies entirely inside, entirely outside,
// or overlaps a given rectangle. Non-displaying space characters that occur
// at the end of individual lines in the text layout are ignored for
// intersection calculations.
//
// The return value is -1 if the text layout is entirely outside of the
// rectangle, 0 if it overlaps, and 1 if it is entirely inside of the
// rectangle.
//
// x, y -- Upper-left hand corner, in pixels, of rectangular area to compare
//         with text layout. Coordinates are with respect to the upper-left
//         hand corner of the text layout itself.
// w, h -- The width and height of the above rectangular area, in pixels.

int OTextLayout::IntersectText(int x, int y, int w, int h) const {
  int result, i, x1, y1, x2, y2;
  OLayoutChunk *chunk;
  int left, top, right, bottom;

  // Scan the chunks one at a time, seeing whether each is entirely in,
  // entirely out, or overlapping the rectangle.  If an overlap is
  // detected, return immediately; otherwise wait until all chunks have
  // been processed and see if they were all inside or all outside.

  chunk = chunks;

  left = x;
  top = y;
  right = x + w;
  bottom = y + h;

  result = 0;
  for (i = 0; i < numChunks; i++) {
    if (chunk->start[0] == '\n') {

      // Newline characters are not counted when computing area
      // intersection (but tab characters would still be considered).

      chunk++;
      continue;
    }
    x1 = chunk->x;
    y1 = chunk->y - font->fm.ascent;
    x2 = chunk->x + chunk->displayWidth;
    y2 = chunk->y + font->fm.descent;

    if ((right < x1) || (left >= x2)
	|| (bottom < y1) || (top >= y2)) {
      if (result == 1) {
	return 0;
      }
      result = -1;
    } else if ((x1 < left) || (x2 >= right)
	       || (y1 < top) || (y2 >= bottom)) {
      return 0;
    } else if (result == -1) {
      return 0;
    } else {
      result = 1;
    }
    chunk++;
  }
  return result;
}


//--------------------------------------------------------------------------

// Outputs the contents of a text layout in Postscript format. The set of
// lines in the text layout will be rendered by the user supplied Postscript
// function. The function should be of the form:
//
//     justify x y string  function  --
//
// Justify is -1, 0, or 1, depending on whether the following string should
// be left, center, or right justified, x and y is the location for the
// origin of the string, string is the sequence of characters to be printed,
// and function is the name of the caller-provided function; the function
// should leave nothing on the stack.
//
// The meaning of the origin of the string (x and y) depends on the
// justification. For left justification, x is where the left edge of the
// string should appear. For center justification, x is where the center of
// the string should appear. And for right justification, x is where the
// right edge of the string should appear. This behavior is necessary
// because, for example, right justified text on the screen is justified
// with screen metrics. The same string needs to be justified with printer
// metrics on the printer to appear in the correct place with respect to
// other similarly justified strings. In all circumstances, y is the
// location of the baseline for the string.
//
// result is modified to hold the Postscript code that will render the text
// layout.

void OTextLayout::ToPostscript(OString *result) const {
#define MAXUSE 128
  char buf[MAXUSE + 10];
  OLayoutChunk *chunk;
  int i, j, used, c, baseline;

  chunk = chunks;
  baseline = chunk->y;
  used = 0;
  buf[used++] = '(';
  for (i = 0; i < numChunks; i++) {
    if (baseline != chunk->y) {
      buf[used++] = ')';
      buf[used++] = '\n';
      buf[used++] = '(';
      baseline = chunk->y;
    }
    if (chunk->numDisplayChars <= 0) {
      if (chunk->start[0] == '\t') {
	buf[used++] = '\\';
	buf[used++] = 't';
      }
    } else {
      for (j = 0; j < chunk->numDisplayChars; j++) {
	c = UCHAR(chunk->start[j]);
	if ((c == '(') || (c == ')') || (c == '\\') || (c < 0x20)
	    || (c >= UCHAR(0x7f))) {

	  // Tricky point: the "03" is necessary in the sprintf
	  // below, so that a full three digits of octal are
	  // always generated. Without the "03", a number
	  // following this sequence could be interpreted by
	  // Postscript as part of this sequence.

	  sprintf(buf + used, "\\%03o", c);
	  used += 4;
	} else {
	  buf[used++] = c;
	}
	if (used >= MAXUSE) {
	  buf[used] = '\0';
	  result->Append(buf);
	  used = 0;
	}
      }
    }
    if (used >= MAXUSE) {

      // If there are a whole bunch of returns or tabs in a row,
      // then buf[] could get filled up.

      buf[used] = '\0';
      result->Append(buf);
      used = 0;
    }
    chunk++;
  }
  buf[used++] = ')';
  buf[used++] = '\n';
  buf[used] = '\0';
  result->Append(buf);
}


//--------------------------------------------------------------------------

// Helper function for ComputeTextLayout(). Encapsulates a measured set of
// characters in a chunk that can be quickly drawn.
//
// Returns a pointer to the new chunk in the text layout. The text layout is
// reallocated to hold more chunks as necessary.
//
// Currently, ComputeTextLayout() stores contiguous ranges of "normal"
// characters in a chunk, along with individual tab and newline chars in
// their own chunks. All characters in the text layout are accounted for.

OLayoutChunk *OXFont::NewChunk(OTextLayout *layout, int *maxPtr,
                               const char *start, int numChars,
                               int curX, int newX, int y) const {
  OLayoutChunk *chunk;
  int i, maxChunks;

  maxChunks = *maxPtr;
  if (layout->numChunks == maxChunks) {
    if (maxChunks == 0) maxChunks = 1; else maxChunks *= 2;
    chunk = new OLayoutChunk[maxChunks];
    if (layout->numChunks > 0) {
      for (i=0; i<layout->numChunks; ++i) chunk[i] = layout->chunks[i];
      delete[] layout->chunks;
    }
    layout->chunks = chunk;
    *maxPtr = maxChunks;
  }
  chunk = &layout->chunks[layout->numChunks];
  chunk->start = start;
  chunk->numChars = numChars;
  chunk->numDisplayChars = numChars;
  chunk->x = curX;
  chunk->y = y;
  chunk->totalWidth = newX - curX;
  chunk->displayWidth = newX - curX;
  layout->numChunks++;

  return chunk;
}


//--------------------------------------------------------------------------

// Map a native font name to a OXFont.
//
// The return value is a pointer to an OXFont object that represents the
// native font. If a native font by the given name could not be found,
// the return value is NULL.  
//
// Every call to this procedure returns a new OXFont object, even if the
// name has already been seen before. The caller should call XFreeFont
// when the font is no longer needed.

OXFont *OFontPool::GetNativeFont(const char *name) {
  XFontStruct *fontStruct;

  fontStruct = XLoadQueryFont(_client->GetDisplay(), name);
  if (fontStruct == NULL) return NULL;

  return MakeFont(NULL, fontStruct, name);
}


//---------------------------------------------------------------------------

// Given a desired set of attributes for a font, find a font with the
// closest matching attributes and create a new OXFont object. If a font
// with the desired attributes could not be constructed, some other font
// will be substituted automatically.

// Given a desired set of attributes for a font, find a font with the
// closest matching attributes.
//
// The return value is a pointer to a OXFont object that represents the
// font with the desired attributes. If a font with the desired attributes
// could not be constructed, some other font will be substituted
// automatically.
//
// Every call to this procedure returns a new OXFont object, even if the
// specified attributes have already been seen before.

OXFont *OFontPool::GetFontFromAttributes(OFontAttributes *fa,
                                         OXFont *fontPtr) {
  int numNames, score, i, scaleable, pixelsize, xaPixelsize;
  int bestIdx, bestScore, bestScaleableIdx, bestScaleableScore;
  OXLFDAttributes xa;
  char buf[256];
  char **nameList;
  OXFont *font;
  XFontStruct *fontStruct;
  const char *fmt, *family;
  double d;

  family = fa->family;
  if (family == NULL) family = "*";
  pixelsize = -fa->pointsize;
  if (pixelsize < 0) {
    d = -pixelsize * 25.4 / 72;
    d *= WidthOfScreen(DefaultScreenOfDisplay(_client->GetDisplay()));
    d /= WidthMMOfScreen(DefaultScreenOfDisplay(_client->GetDisplay()));
    d += 0.5;
    pixelsize = (int) d;
  }

  // Replace any standard Windows and Mac family names with the names that
  // X likes.

#if 0
  if ((strcasecmp("Times New Roman", family) == 0)
      || (strcasecmp("New York", family) == 0)) {
    family = "Times";
  } else if ((strcasecmp("Courier New", family) == 0)
	     || (strcasecmp("Monaco", family) == 0)) {
    family = "Courier";
  } else if ((strcasecmp("Arial", family) == 0)
	     || (strcasecmp("Geneva", family) == 0)) {
    family = "Helvetica";
  }
#endif

  // First try for the Q&D exact match.  

#if 0
  sprintf(buf, "-*-%.200s-%s-%c-normal-*-*-%d-*-*-*-*-iso8859-1", family,
	  ((fa->weight > FONT_FW_NORMAL) ? "bold" : "medium"),
	  ((fa->slant == FONT_FS_ROMAN) ? 'r' :
	   (fa->slant == FONT_FS_ITALIC) ? 'i' : 'o'),
	  fa->pointsize * 10);
  fontStruct = XLoadQueryFont(_client->GetDisplay(), buf);
#else
  fontStruct = NULL;
#endif

  if (fontStruct != NULL) {
    goto end;
  }

  // Couldn't find exact match. Now fall back to other available
  // physical fonts.  

  fmt = "-*-%.240s-*-*-*-*-*-*-*-*-*-*-*-*";
  sprintf(buf, fmt, family);
  nameList = XListFonts(_client->GetDisplay(), buf, 10000, &numNames);
  if (numNames == 0) {

    // Try getting some system font.

    sprintf(buf, fmt, "fixed");
    nameList = XListFonts(_client->GetDisplay(), buf, 10000, &numNames);
    if (numNames == 0) {
    getsystem:
      fontStruct = XLoadQueryFont(_client->GetDisplay(), "fixed");
      if (fontStruct == NULL) {
	fontStruct = XLoadQueryFont(_client->GetDisplay(), "*");
	if (fontStruct == NULL) {
	  FatalError("OFontPool: cannot get any font");
	}
      }
      goto end;
    }
  }

  // Inspect each of the XLFDs and pick the one that most closely
  // matches the desired attributes.

  bestIdx = 0;
  bestScore = INT_MAX;
  bestScaleableIdx = 0;
  bestScaleableScore = INT_MAX;

  for (i = 0; i < numNames; i++) {
    score = 0;
    scaleable = 0;
    if (!ParseXLFD(nameList[i], &xa)) continue;
    xaPixelsize = -xa.fa.pointsize;

    // Since most people used to use -adobe-* in their XLFDs,
    // preserve the preference for "adobe" foundry. Otherwise
    // some applications looks may change slightly if another foundry
    // is chosen.

    if (strcasecmp(xa.foundry, "adobe") != 0) {
      score += 3000;
    }
    if (xa.fa.pointsize == 0) {

      // A scaleable font is almost always acceptable, but the
      // corresponding bitmapped font would be better.

      score += 10;
      scaleable = 1;
    } else {

      // A font that is too small is better than one that is too big.

      if (xaPixelsize > pixelsize) {
	score += (xaPixelsize - pixelsize) * 120;
      } else {
	score += (pixelsize - xaPixelsize) * 100;
      }
    }

    score += ABS(xa.fa.weight - fa->weight) * 30;
    score += ABS(xa.fa.slant - fa->slant) * 25;
    if (xa.slant == FONT_FS_OBLIQUE) {

      // Italic fonts are preferred over oblique.

      //score += 4;
    }
    if (xa.setwidth != FONT_SW_NORMAL) {

      // The normal setwidth is highly preferred.

      score += 2000;
    }
    if (xa.charset == FONT_CS_OTHER) {

      // The standard character set is highly preferred over
      // foreign languages charsets (because we don't support
      // other languages yet).

      score += 11000;
    }
    if ((xa.charset == FONT_CS_NORMAL) && (xa.encoding != 1)) {

      // The '1' encoding for the characters above 0x7f is highly
      // preferred over the other encodings.

      score += 8000;
    }
    if (scaleable) {
      if (score < bestScaleableScore) {
	bestScaleableIdx = i;
	bestScaleableScore = score;
      }
    } else {
      if (score < bestScore) {
	bestIdx = i;
	bestScore = score;
      }
    }
    if (score == 0) {
      break;
    }
  }

  // Now we know which is the closest matching scaleable font and the
  // closest matching bitmapped font. If the scaleable font was a
  // better match, try getting the scaleable font; however, if the
  // scalable font was not actually available in the desired pointsize,
  // fall back to the closest bitmapped font.

  fontStruct = NULL;
  if (bestScaleableScore < bestScore) {
    char *str, *rest;

    // Fill in the desired pointsize info for this font.

  tryscale:
    str = nameList[bestScaleableIdx];
    for (i = 0; i < XLFD_PIXEL_SIZE - 1; i++) {
      str = strchr(str + 1, '-');
    }
    rest = str;
    for (i = XLFD_PIXEL_SIZE - 1; i < XLFD_REGISTRY; i++) {
      rest = strchr(rest + 1, '-');
    }
    *str = '\0';
    sprintf(buf, "%.240s-*-%d-*-*-*-*-*%s", nameList[bestScaleableIdx],
	    pixelsize, rest);
    *str = '-';
    fontStruct = XLoadQueryFont(_client->GetDisplay(), buf);
    bestScaleableScore = INT_MAX;
  }
  if (fontStruct == NULL) {
    strcpy(buf, nameList[bestIdx]);
    fontStruct = XLoadQueryFont(_client->GetDisplay(), buf);
    if (fontStruct == NULL) {

      // This shouldn't happen because the font name is one of the
      // names that X gave us to use, but it does anyhow.

      if (bestScaleableScore < INT_MAX) {
	goto tryscale;
      } else {
	XFreeFontNames(nameList);
	goto getsystem;
      }
    }
  }
  XFreeFontNames(nameList);

end:
  font = MakeFont(fontPtr, fontStruct, buf);
  font->fa.underline = fa->underline;
  font->fa.overstrike = fa->overstrike;

  return font;
}


//---------------------------------------------------------------------------

// Return information about the font families that are available on the
// current display.
//
// An array of strings is returned holding a list of all the available font
// families. The array is terminated with a NULL pointer.

char **OFontPool::GetFontFamilies() {
  int i, inew, numNames;
  char *family, *end, *p;
  OStringHashTable familyTable;
  OHashEntry *h;
  OHashSearch search;
  char **nameList;
  char **dst;

  nameList = XListFonts(_client->GetDisplay(), "*", 10000, &numNames);
  for (i = 0; i < numNames; i++) {
    if (nameList[i][0] != '-') continue;
    family = strchr(nameList[i] + 1, '-');
    if (family == NULL) continue;
    family++;
    end = strchr(family, '-');
    if (end == NULL) continue;
    *end = '\0';
    for (p = family; *p != '\0'; p++) {
      if (isupper(UCHAR(*p))) {
	*p = tolower(UCHAR(*p));
      }
    }
    familyTable.Create(family, &inew);
  }

  dst = new char*[familyTable.NumEntries()+1];

  for (i = 0, h = familyTable.FirstHashEntry(&search);
       h != NULL; h = familyTable.NextHashEntry(&search), ++i) {
    dst[i] = new char[strlen(h->GetHashKey())+1];
    strcpy(dst[i], h->GetHashKey());
  }
  dst[i] = NULL;

  XFreeFontNames(nameList);
  return dst;
}

void OFontPool::FreeFontFamilies(char **f) {
  int i;

  if (f == NULL) return;
  for (i = 0; f[i] != NULL; ++i) delete[] f[i];
  delete[] f;
}


//--------------------------------------------------------------------------

// Determine the number of characters from the string that will fit in the
// given horizontal span. The measurement is done under the assumption that
// DrawChars() will be used to actually display the characters.
//
// The return value is the number of characters from source that fit into
// the span that extends from 0 to maxLength. *length is filled with the
// x-coordinate of the right edge of the last character that did fit.
//
// source    -- Characters to be displayed. Need not be '\0' terminated.
// numChars  -- Maximum number of characters to consider from source string.
// maxLength -- If > 0, maxLength specifies the longest permissible line
//              length; don't consider any character that would cross this
//              x-position. If <= 0, then line length is unbounded and the
//              flags argument is ignored.
// flags     -- Various flag bits OR-ed together:
//              TEXT_PARTIAL_OK means include the last char which only
//              partially fit on this line.
//              TEXT_WHOLE_WORDS means stop on a word boundary, if possible.
//              TEXT_AT_LEAST_ONE means return at least one character even
//              if no characters fit.
// *length   -- Filled with x-location just after the terminating character.

int OXFont::MeasureChars(const char *source, int numChars, int maxLength,
                         int flags, int *length) const {
  const char *p;		// Current character.
  const char *term;		// Pointer to most recent character that
				// may legally be a terminating character.
  int termX;			// X-position just after term.
  int curX;			// X-position corresponding to p.
  int newX;			// X-position corresponding to p+1.
  int c, sawNonSpace;

  if (numChars == 0) {
    *length = 0;
    return 0;
  }
  if (maxLength <= 0) {
    maxLength = INT_MAX;
  }
  newX = curX = termX = 0;
  p = term = source;
  sawNonSpace = !isspace(UCHAR(*p));

  // Scan the input string one character at a time, calculating width.

  for (c = UCHAR(*p);;) {
    newX += widths[c];
    if (newX > maxLength) {
      break;
    }
    curX = newX;
    numChars--;
    p++;
    if (numChars == 0) {
      term = p;
      termX = curX;
      break;
    }
    c = UCHAR(*p);
    if (isspace(c)) {
      if (sawNonSpace) {
	term = p;
	termX = curX;
	sawNonSpace = 0;
      }
    } else {
      sawNonSpace = 1;
    }
  }

  // P points to the first character that doesn't fit in the desired
  // span. Use the flags to figure out what to return.

  if ((flags & TEXT_PARTIAL_OK) && (numChars > 0) && (curX < maxLength)) {

    // Include the first character that didn't quite fit in the desired
    // span. The width returned will include the width of that extra
    // character.

    numChars--;
    curX = newX;
    p++;
  }
  if ((flags & TEXT_AT_LEAST_ONE) && (term == source) && (numChars > 0)) {
    term = p;
    termX = curX;
    if (term == source) {
      term++;
      termX = newX;
    }
  } else if ((numChars == 0) || !(flags & TEXT_WHOLE_WORDS)) {
    term = p;
    termX = curX;
  }
  *length = termX;

  return term - source;
}


//-------------------------------------------------------------------------

// Draw a string of characters on the screen. DrawCharsExp() expands
// control characters that occur in the string to \X or \xXX sequences.
// DrawChars() just draws the strings.
//
// dpy      -- Display on which to draw.
// dst      -- Window or pixmap in which to draw.
// gc       -- Graphics context for drawing characters.
// source   -- Characters to be displayed. Need not be'\0' terminated.
//             For DrawChars(), all meta-characters (tabs, control
//             characters, and newlines) should be stripped out of the
//             string that is passed to this function. If they are not
//             stripped out, they will be displayed as regular printing
//             characters.
// numChars -- Number of characters in string.
// x, y     -- Coordinates at which to place origin of string when drawing.

void OXFont::DrawCharsExp(Display *dpy, Drawable dst, GC gc,
                          const char *source, int numChars,
                          int x, int y) const {
  const char *p;
  int i, type;
  char buf[4];

  p = source;
  for (i = 0; i < numChars; i++) {
    type = types[UCHAR(*p)];
    if (type != NORMAL) {
      DrawChars(dpy, dst, gc, source, p - source, x, y);
      x += ::XTextWidth(fontStruct, source, p - source);
      if (type == REPLACE) {
	DrawChars(dpy, dst, gc, buf,
		  GetControlCharSubst(UCHAR(*p), buf), x, y);
	x += widths[UCHAR(*p)];
      }
      source = p + 1;
    }
    p++;
  }

  DrawChars(dpy, dst, gc, source, p - source, x, y);
}

void OXFont::DrawChars(Display *dpy, Drawable dst, GC gc,
                       const char *source, int numChars,
                       int x, int y) const {

  // Perform a quick sanity check to ensure we won't overflow the X
  // coordinate space.

  if ((x + (fontStruct->max_bounds.width * numChars) > 0x7fff)) {
    int length;

    // The string we are being asked to draw is too big and would overflow
    // the X coordinate space. Unfortunatley X servers aren't too bright
    // and so they won't deal with this case cleanly. We need to truncate
    // the string before sending it to X.

    numChars = MeasureChars(source, numChars, 0x7fff - x, 0, &length);
  }
  XDrawString(dpy, dst, gc, x, y, source, numChars);

  if (fa.underline != 0) {
    XFillRectangle(dpy, dst, gc, x,  y + underlinePos,
	    (unsigned) ::XTextWidth(fontStruct, source, numChars),
	    (unsigned) barHeight);
  }
  if (fa.overstrike != 0) {
    y -= fm.descent + fm.ascent / 10;
    XFillRectangle(dpy, dst, gc, x, y,
	    (unsigned) ::XTextWidth(fontStruct, source, numChars),
	    (unsigned) barHeight);
  }
}


//--------------------------------------------------------------------------
//
// Helper for GetNativeFont() and GetFontFromAttributes(). Creates and
// intializes a new OXFont object.
//
// font       -- If non-NULL, store the information in this existing OXFont
//               object, rather than creating a new one; the existing
//               contents of the font will be released. If NULL, a new
//               OXFont object is created.
// fontStruct -- X information about font.
// fontName   -- The string passed to XLoadQueryFont() to construct the
//               fontStruct.

OXFont *OFontPool::MakeFont(OXFont *font, XFontStruct *fontStruct,
                            const char *fontName) {
  OXFont *newFont;
  unsigned long value;
  int i, width, firstChar, lastChar, n, replaceOK;
  char *name, *p;
  char buf[4];
  OXLFDAttributes xa;
  double d;

  if (font != NULL) {
    XFreeFont(_client->GetDisplay(), font->fontStruct);
    newFont = font;
  } else {
    newFont = new OXFont();
  }

  // Encapsulate the generic stuff in the OXFont. 

  newFont->SetId(fontStruct->fid);

  if (XGetFontProperty(fontStruct, XA_FONT, &value) && (value != 0)) {
    name = XGetAtomName(_client->GetDisplay(), (Atom) value);
    xa.fa.Init();
    if (ParseXLFD(name, &xa) == True) {
      XFree(name);
      goto ok;
    }
  }
  xa.fa.Init();
  if (ParseXLFD(fontName, &xa) != True) {
    newFont->fa.Init();
    newFont->fa.family = GetUid(fontName);
  } else {
  ok:
    newFont->fa = xa.fa;
  }

  if (newFont->fa.pointsize < 0) {
    d = -newFont->fa.pointsize * 72 / 25.4;
    d *= WidthMMOfScreen(DefaultScreenOfDisplay(_client->GetDisplay()));
    d /= WidthOfScreen(DefaultScreenOfDisplay(_client->GetDisplay()));
    d += 0.5;
    newFont->fa.pointsize = (int) d;
  }
  newFont->fm.ascent = fontStruct->max_bounds.ascent; //->ascent;
  newFont->fm.descent = fontStruct->max_bounds.descent; //->descent;
  newFont->fm.linespace = newFont->fm.ascent + newFont->fm.descent;
  newFont->fm.maxWidth = fontStruct->max_bounds.width;
  newFont->fm.fixed = True;
  newFont->fontStruct = fontStruct;

  // Classify the characters.

  firstChar = fontStruct->min_char_or_byte2;
  lastChar = fontStruct->max_char_or_byte2;
  for (i = 0; i < 256; i++) {
    if ((i == 0177) || (i < firstChar) || (i > lastChar)) {
      newFont->types[i] = REPLACE;
    } else {
      newFont->types[i] = NORMAL;
    }
  }

  // Compute the widths for all the normal characters. Any other
  // characters are given an initial width of 0. Also, this determines
  // if this is a fixed or variable width font, by comparing the widths
  // of all the normal characters.

  width = 0;
  for (i = 0; i < 256; i++) {
    if (newFont->types[i] != NORMAL) {
      n = 0;
    } else if (fontStruct->per_char == NULL) {
      n = fontStruct->max_bounds.width;
    } else {
      n = fontStruct->per_char[i - firstChar].width;
    }
    newFont->widths[i] = n;
    if (n != 0) {
      if (width == 0) {
	width = n;
      } else if (width != n) {
	newFont->fm.fixed = False;
      }
    }
  }

  // Compute the widths of the characters that should be replaced with
  // control character expansions. If the appropriate chars are not
  // available in this font, then control character expansions will not
  // be used; control chars will be invisible & zero-width.

  replaceOK = 1;
  for (p = hexChars; *p != '\0'; p++) {
    if ((UCHAR(*p) < firstChar) || (UCHAR(*p) > lastChar)) {
      replaceOK = 0;
      break;
    }
  }
  for (i = 0; i < 256; i++) {
    if (newFont->types[i] == REPLACE) {
      if (replaceOK) {
	n = GetControlCharSubst(i, buf);
	for (; --n >= 0;) {
	  newFont->widths[i] += newFont->widths[UCHAR(buf[n])];
	}
      } else {
	newFont->types[i] = SKIP;
      }
    }
  }

  if (XGetFontProperty(fontStruct, XA_UNDERLINE_POSITION, &value)) {
    newFont->underlinePos = value;
  } else {

    // If the XA_UNDERLINE_POSITION property does not exist, the X
    // manual recommends using the following value:

    newFont->underlinePos = fontStruct->descent / 2;
  }
  newFont->barHeight = 0;
  if (XGetFontProperty(fontStruct, XA_UNDERLINE_THICKNESS, &value)) {

    // Sometimes this is 0 even though it shouldn't be.

    newFont->barHeight = value;
  }
  if (newFont->barHeight == 0) {

    // If the XA_UNDERLINE_THICKNESS property does not exist, the X
    // manual recommends using the width of the stem on a capital
    // letter. I don't know of a way to get the stem width of a letter,
    // so guess and use 1/3 the width of a capital I.

    newFont->barHeight = newFont->widths['I'] / 3;
    if (newFont->barHeight == 0) {
      newFont->barHeight = 1;
    }
  }
  if (newFont->underlinePos + newFont->barHeight > fontStruct->descent) {

    // If this set of cobbled together values would cause the bottom of
    // the underline bar to stick below the descent of the font, jack
    // the underline up a bit higher.

    newFont->barHeight = fontStruct->descent - newFont->underlinePos;
    if (newFont->barHeight == 0) {
      newFont->underlinePos--;
      newFont->barHeight = 1;
    }
  }
  return newFont;
}


//--------------------------------------------------------------------------

// When displaying text in a widget, a backslashed escape sequence is
// substituted for control characters that occur in the text. Given a
// control character, fill in a buffer with the replacement string that
// should be displayed.
//
// The return value is the length of the substitute string, buf is
// filled with the substitute string; it is not '\0' terminated.
//
// c   -- The control character to be replaced.
// buf -- Buffer that gets replacement string. It only needs to be
//        4 characters long.

static int GetControlCharSubst(int c, char buf[4]) {
  buf[0] = '\\';
  if ((c < sizeof (mapChars)) && (mapChars[c] != 0)) {
    buf[1] = mapChars[c];
    return 2;
  } else {
    buf[1] = 'x';
    buf[2] = hexChars[(c >> 4) & 0xf];
    buf[3] = hexChars[c & 0xf];
    return 4;
  }
}
