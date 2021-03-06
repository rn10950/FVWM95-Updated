/**************************************************************************

    HTML widget for xclass. Based on tkhtml 1.28
    Copyright (C) 1997-2000 D. Richard Hipp <drh@acm.org>
    Copyright (C) 2002-2003 Hector Peraza.

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

#include <xclass/OXHtml.h>


// NOTE: this list was generated automatically. If you make any
// modifications to it, you'll have to modify also the OHtmlTokens.h
// file accordingly.

SHtmlTokenMap HtmlMarkupMap[] = {
  { "a",            Html_A,                 O_HtmlAnchor,        },
  { "/a",           Html_EndA,              O_HtmlRef,           },
  { "address",      Html_ADDRESS,           O_HtmlMarkupElement  },
  { "/address",     Html_EndADDRESS,        O_HtmlMarkupElement  },
  { "applet",       Html_APPLET,            O_HtmlInput,         },
  { "/applet",      Html_EndAPPLET,         O_HtmlMarkupElement  },
  { "area",         Html_AREA,              O_HtmlMapArea        },
  { "b",            Html_B,                 O_HtmlMarkupElement  },
  { "/b",           Html_EndB,              O_HtmlMarkupElement  },
  { "base",         Html_BASE,              O_HtmlMarkupElement  },
  { "basefont",     Html_BASEFONT,          O_HtmlMarkupElement  },
  { "/basefont",    Html_EndBASEFONT,       O_HtmlMarkupElement  },
  { "bgsound",      Html_BGSOUND,           O_HtmlMarkupElement  },
  { "big",          Html_BIG,               O_HtmlMarkupElement  },
  { "/big",         Html_EndBIG,            O_HtmlMarkupElement  },
  { "blockquote",   Html_BLOCKQUOTE,        O_HtmlMarkupElement  },
  { "/blockquote",  Html_EndBLOCKQUOTE,     O_HtmlMarkupElement  },
  { "body",         Html_BODY,              O_HtmlMarkupElement  },
  { "/body",        Html_EndBODY,           O_HtmlMarkupElement  },
  { "br",           Html_BR,                O_HtmlMarkupElement  },
  { "caption",      Html_CAPTION,           O_HtmlMarkupElement  },
  { "/caption",     Html_EndCAPTION,        O_HtmlMarkupElement  },
  { "center",       Html_CENTER,            O_HtmlMarkupElement  },
  { "/center",      Html_EndCENTER,         O_HtmlMarkupElement  },
  { "cite",         Html_CITE,              O_HtmlMarkupElement  },
  { "/cite",        Html_EndCITE,           O_HtmlMarkupElement  },
  { "code",         Html_CODE,              O_HtmlMarkupElement  },
  { "/code",        Html_EndCODE,           O_HtmlMarkupElement  },
  { "comment",      Html_COMMENT,           O_HtmlMarkupElement  }, // Text!
  { "/comment",     Html_EndCOMMENT,        O_HtmlMarkupElement  },
  { "dd",           Html_DD,                O_HtmlRef,           },
  { "/dd",          Html_EndDD,             O_HtmlMarkupElement  },
  { "dfn",          Html_DFN,               O_HtmlMarkupElement  },
  { "/dfn",         Html_EndDFN,            O_HtmlMarkupElement  },
  { "dir",          Html_DIR,               O_HtmlListStart,     },
  { "/dir",         Html_EndDIR,            O_HtmlRef,           },
  { "div",          Html_DIV,               O_HtmlMarkupElement  },
  { "/div",         Html_EndDIV,            O_HtmlMarkupElement  },
  { "dl",           Html_DL,                O_HtmlListStart,     },
  { "/dl",          Html_EndDL,             O_HtmlRef,           },
  { "dt",           Html_DT,                O_HtmlRef,           },
  { "/dt",          Html_EndDT,             O_HtmlMarkupElement  },
  { "em",           Html_EM,                O_HtmlMarkupElement  },
  { "/em",          Html_EndEM,             O_HtmlMarkupElement  },
  { "embed",        Html_EMBED,             O_HtmlInput,         },
  { "font",         Html_FONT,              O_HtmlMarkupElement  },
  { "/font",        Html_EndFONT,           O_HtmlMarkupElement  },
  { "form",         Html_FORM,              O_HtmlForm,          },
  { "/form",        Html_EndFORM,           O_HtmlRef,           },
  { "frame",        Html_FRAME,             O_HtmlMarkupElement  },
  { "/frame",       Html_EndFRAME,          O_HtmlMarkupElement  },
  { "frameset",     Html_FRAMESET,          O_HtmlMarkupElement  },
  { "/frameset",    Html_EndFRAMESET,       O_HtmlMarkupElement  },
  { "h1",           Html_H1,                O_HtmlMarkupElement  },
  { "/h1",          Html_EndH1,             O_HtmlMarkupElement  },
  { "h2",           Html_H2,                O_HtmlMarkupElement  },
  { "/h2",          Html_EndH2,             O_HtmlMarkupElement  },
  { "h3",           Html_H3,                O_HtmlMarkupElement  },
  { "/h3",          Html_EndH3,             O_HtmlMarkupElement  },
  { "h4",           Html_H4,                O_HtmlMarkupElement  },
  { "/h4",          Html_EndH4,             O_HtmlMarkupElement  },
  { "h5",           Html_H5,                O_HtmlMarkupElement  },
  { "/h5",          Html_EndH5,             O_HtmlMarkupElement  },
  { "h6",           Html_H6,                O_HtmlMarkupElement  },
  { "/h6",          Html_EndH6,             O_HtmlMarkupElement  },
  { "hr",           Html_HR,                O_HtmlHr,            },
  { "html",         Html_HTML,              O_HtmlMarkupElement  },
  { "/html",        Html_EndHTML,           O_HtmlMarkupElement  },
  { "i",            Html_I,                 O_HtmlMarkupElement  },
  { "/i",           Html_EndI,              O_HtmlMarkupElement  },
  { "iframe",       Html_IFRAME,            O_HtmlMarkupElement  },
  { "img",          Html_IMG,               O_HtmlImageMarkup,   },
  { "input",        Html_INPUT,             O_HtmlInput,         },
  { "isindex",      Html_ISINDEX,           O_HtmlMarkupElement  },
  { "kbd",          Html_KBD,               O_HtmlMarkupElement  },
  { "/kbd",         Html_EndKBD,            O_HtmlMarkupElement  },
  { "li",           Html_LI,                O_HtmlLi,            },
  { "/li",          Html_EndLI,             O_HtmlMarkupElement  },
  { "link",         Html_LINK,              O_HtmlMarkupElement  },
  { "listing",      Html_LISTING,           O_HtmlMarkupElement  },
  { "/listing",     Html_EndLISTING,        O_HtmlMarkupElement  },
  { "map",          Html_MAP,               O_HtmlMarkupElement  },
  { "/map",         Html_EndMAP,            O_HtmlMarkupElement  },
  { "marquee",      Html_MARQUEE,           O_HtmlMarkupElement  },
  { "/marquee",     Html_EndMARQUEE,        O_HtmlMarkupElement  },
  { "menu",         Html_MENU,              O_HtmlListStart,     },
  { "/menu",        Html_EndMENU,           O_HtmlRef,           },
  { "meta",         Html_META,              O_HtmlMarkupElement  },
  { "nextid",       Html_NEXTID,            O_HtmlMarkupElement  },
  { "nobr",         Html_NOBR,              O_HtmlMarkupElement  },
  { "/nobr",        Html_EndNOBR,           O_HtmlMarkupElement  },
  { "noembed",      Html_NOEMBED,           O_HtmlMarkupElement  },
  { "/noembed",     Html_EndNOEMBED,        O_HtmlMarkupElement  },
  { "noframe",      Html_NOFRAMES,          O_HtmlMarkupElement  },
  { "/noframe",     Html_EndNOFRAMES,       O_HtmlMarkupElement  },
  { "noscript",     Html_NOSCRIPT,          O_HtmlMarkupElement  },
  { "/noscript",    Html_EndNOSCRIPT,       O_HtmlMarkupElement  },
  { "ol",           Html_OL,                O_HtmlListStart,     },
  { "/ol",          Html_EndOL,             O_HtmlRef,           },
  { "option",       Html_OPTION,            O_HtmlMarkupElement  },
  { "/option",      Html_EndOPTION,         O_HtmlMarkupElement  },
  { "p",            Html_P,                 O_HtmlMarkupElement  },
  { "/p",           Html_EndP,              O_HtmlMarkupElement  },
  { "param",        Html_PARAM,             O_HtmlMarkupElement  },
  { "/param",       Html_EndPARAM,          O_HtmlMarkupElement  },
  { "plaintext",    Html_PLAINTEXT,         O_HtmlMarkupElement  },
  { "pre",          Html_PRE,               O_HtmlMarkupElement  },
  { "/pre",         Html_EndPRE,            O_HtmlMarkupElement  },
  { "s",            Html_S,                 O_HtmlMarkupElement  },
  { "/s",           Html_EndS,              O_HtmlMarkupElement  },
  { "samp",         Html_SAMP,              O_HtmlMarkupElement  },
  { "/samp",        Html_EndSAMP,           O_HtmlMarkupElement  },
  { "script",       Html_SCRIPT,            O_HtmlScript,        },
  { "select",       Html_SELECT,            O_HtmlInput,         },
  { "/select",      Html_EndSELECT,         O_HtmlRef,           },
  { "small",        Html_SMALL,             O_HtmlMarkupElement  },
  { "/small",       Html_EndSMALL,          O_HtmlMarkupElement  },
  { "strike",       Html_STRIKE,            O_HtmlMarkupElement  },
  { "/strike",      Html_EndSTRIKE,         O_HtmlMarkupElement  },
  { "strong",       Html_STRONG,            O_HtmlMarkupElement  },
  { "/strong",      Html_EndSTRONG,         O_HtmlMarkupElement  },
  { "style",        Html_STYLE,             O_HtmlScript,        },
  { "sub",          Html_SUB,               O_HtmlMarkupElement  },
  { "/sub",         Html_EndSUB,            O_HtmlMarkupElement  },
  { "sup",          Html_SUP,               O_HtmlMarkupElement  },
  { "/sup",         Html_EndSUP,            O_HtmlMarkupElement  },
  { "table",        Html_TABLE,             O_HtmlTable,         },
  { "/table",       Html_EndTABLE,          O_HtmlRef,           },
  { "td",           Html_TD,                O_HtmlCell,          },
  { "/td",          Html_EndTD,             O_HtmlRef,           },
  { "textarea",     Html_TEXTAREA,          O_HtmlInput,         },
  { "/textarea",    Html_EndTEXTAREA,       O_HtmlRef,           },
  { "th",           Html_TH,                O_HtmlCell,          },
  { "/th",          Html_EndTH,             O_HtmlRef,           },
  { "title",        Html_TITLE,             O_HtmlMarkupElement  },
  { "/title",       Html_EndTITLE,          O_HtmlMarkupElement  },
  { "tr",           Html_TR,                O_HtmlRef,           },
  { "/tr",          Html_EndTR,             O_HtmlRef,           },
  { "tt",           Html_TT,                O_HtmlMarkupElement  },
  { "/tt",          Html_EndTT,             O_HtmlMarkupElement  },
  { "u",            Html_U,                 O_HtmlMarkupElement  },
  { "/u",           Html_EndU,              O_HtmlMarkupElement  },
  { "ul",           Html_UL,                O_HtmlListStart,     },
  { "/ul",          Html_EndUL,             O_HtmlRef,           },
  { "var",          Html_VAR,               O_HtmlMarkupElement  },
  { "/var",         Html_EndVAR,            O_HtmlMarkupElement  },
  { "wbr",          Html_WBR,               O_HtmlMarkupElement  },
  { "xmp",          Html_XMP,               O_HtmlMarkupElement  },
  { "/xmp",         Html_EndXMP,            O_HtmlMarkupElement  },
  { 0,              0,                      0                    }
};
