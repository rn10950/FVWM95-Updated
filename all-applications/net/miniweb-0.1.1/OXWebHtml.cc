#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#ifdef __FreeBSD__
#include <netinet/in.h>
#endif
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <xclass/utils.h>
#include <xclass/OGifImage.h>
#include <xclass/OXRadioButton.h>
#include <xclass/OXTextEntry.h>

#include "OXWebHtml.h"
#include "main.h"


//#define DEBUG_FORM
//#define DEBUG_CONNECT

//----------------------------------------------------------------------

OXWebHtml::OXWebHtml(const OXWindow *p, int w, int h, int id) :
  OXHtml(p, w, h, id) {
    nCachedImages = 0;
    imageCache = 0;
    visitedUrl.clear();
    docName = 0;
    docNameStart = -1;
}

OXWebHtml::~OXWebHtml() {
  Image *p, *next;

  p = imageCache;
  while (p) {
    next = p->pNext;
    delete[] p->zUrl;
    unlink(p->zLocal);
    delete[] p->zLocal;
    delete p;
    p = next;
  }

  for (int i = 0; i < visitedUrl.size(); ++i) delete[] visitedUrl[i];
  visitedUrl.clear();

  if (docName) delete[] docName;
}


//----------------------------------------------------------------------

// Make sure the given URL is loaded as a local file. Return the
// name of the local file.

char *OXWebHtml::HttpGetImage(const char *zUrl) {
  char local[100], cont[300];
  Image *p;

  for (p = imageCache; p; p = p->pNext) {
    if (strcmp(p->zUrl, zUrl) == 0) return p->zLocal;
  }
  sprintf(local, "/tmp/%s.image%d.tmp", APP_NAME, nCachedImages+1);
  try {
    HttpFetch(zUrl, local, 0, 0, cont);
  } catch (OConnectException &e) {
    // we could create a "broken image" icon here...
    return NULL;
  }

  p = new Image;
  p->zUrl = StrDup(zUrl);
  p->zLocal = StrDup(local);
  p->pNext = imageCache;
  imageCache = p;
  ++nCachedImages;

  return p->zLocal;
}

OImage *OXWebHtml::LoadImage(const char *url, int w, int h) {
  OImage *image = 0;

  OHtmlUri uri(url);
  if (uri.zScheme && strcmp(uri.zScheme, "http") == 0) {
    char *localfile = HttpGetImage(url);
    if (localfile) {
      image = new OGifImage(_client, localfile);
      if (((OGifImage *)image)->isInvalidGif()) {
        delete image;
        image = 0;
      }
    }
  } else {
    image = OXHtml::LoadImage(url, w, h);
  }

  return image;
}

void OXWebHtml::Clear() {
  Image *p, *next;

  OXHtml::Clear();
#if 0
  p = imageCache;
  while (p) {
    next = p->pNext;
    delete[] p->zUrl;
    unlink(p->zLocal);
    delete[] p->zLocal;
    delete p;
    p = next;
  }
  imageCache = 0;
  nCachedImages = 0;
#endif

  if (docName) delete[] docName;
  docName = 0;
  docNameStart = -1;
}

int OXWebHtml::IsVisited(const char *url) {
  int i;
  char *z = ResolveUri(url);

  if (!z) return False;

  for (i = 0; i < visitedUrl.size(); ++i) {
    if (strcmp(visitedUrl[i], z) == 0) {
      delete[] z;
      return True;
    }
  }

  delete[] z;
  return False;
}

void OXWebHtml::AddToVisited(const char *url) {
  char *z = ResolveUri(url);
  if (!z) return;
  if (!IsVisited(z)) visitedUrl.push_back(StrDup(z));
  delete[] z;
}

int OXWebHtml::ProcessToken(OHtmlElement *pElem, const char *name, int type) {
  if (type == Html_TITLE) {
    docNameStart = pElem->offs;
  } else if (type == Html_EndTITLE) {
    if (docNameStart > 0) {
      if (docName) delete[] docName;
      // StrNDup(...)
      int len = pElem->offs - docNameStart - strlen(name) - 2;
      docName = new char[len + 1];
      strncpy(docName, zText + docNameStart, len);
      docName[len] = '\0';
    }
  }
  return False;
}

int OXWebHtml::FormAction(OHtmlForm *form, int id) {
  OString str("");

  if (!form) {
#ifdef DEBUG_FORM
    fprintf(stderr, "No form!\n");
#endif
    return False;
  } else {
    OHtmlInput *p;
#ifdef DEBUG_FORM
    printf("method = %s\n", form->MarkupArg("method", 0));
    printf("action = %s\n", form->MarkupArg("action", 0));
#endif
    for (p = firstInput; p; p = p->iNext) {
      if (p->pForm == form) {
        switch (p->itype) {
          case INPUT_TYPE_Hidden: {
            const char *name = p->MarkupArg("name", 0);
            const char *value = p->MarkupArg("value", "");
            if (name && *name) {
              if (str.GetLength() > 0) str.Append("&");
              EncodeText(&str, name);
              str.Append("=");
              EncodeText(&str, value);
            }
            break;
          }

          case INPUT_TYPE_Radio: {
            OXRadioButton *r = (OXRadioButton *) p->frame;
            if (r && r->GetState() == BUTTON_DOWN) {
              const char *name = p->MarkupArg("name", 0);
              const char *value = p->MarkupArg("value", "");
              if (name && *name) {
                if (str.GetLength() > 0) str.Append("&");
                EncodeText(&str, name);
                str.Append("=");
                EncodeText(&str, value);
              }
            }
            break;
          }

          case INPUT_TYPE_Submit:
            if (p->cnt == id) {
              const char *name = p->MarkupArg("name", 0);
              const char *value = p->MarkupArg("value", "");
              if (name && *name) {
                if (str.GetLength() > 0) str.Append("&");
                EncodeText(&str, name);
                str.Append("=");
                EncodeText(&str, value);
              }
            }
            break;

          case INPUT_TYPE_Text:
          case INPUT_TYPE_Password: {
            OXTextEntry *te = (OXTextEntry *) p->frame;
            if (te) {
              const char *name = p->MarkupArg("name", 0);
              const char *value = te->GetString();
              if (name && *name) {
                if (str.GetLength() > 0) str.Append("&");
                EncodeText(&str, name);
                str.Append("=");
                EncodeText(&str, value);
              }
            }
            break;
          }

          default:
            break;
        }
      }
    }
    const char *method = form->MarkupArg("method", "GET");
    if (strcasecmp(method, "GET") == 0) {
      str.Prepend("?");
      str.Prepend(form->MarkupArg("action", ""));
      char *uri = ResolveUri(str.GetString());
      if (uri) {
#ifdef DEBUG_FORM
        printf("Submiting form: %s\n", uri);
#endif
        OHtmlMessage msg(MSG_HTML, MSG_CLICK, _widgetID, uri, 0, 0);
        SendMessage(_msgObject, &msg);
        delete[] uri;
      }
    } else if (strcasecmp(method, "POST") == 0) {
#ifdef DEBUG_FORM
      printf("POST method not implemented\n");
#endif
    } else {
#ifdef DEBUG_FORM
      printf("Unknown form method: %s\n", method);
#endif
    }
  }
  return True;
}

// Get a URL using HTTP.  Return the result code.  If a Location: field
// appears in the header, write it into zLocation[].  Location[] should
// be at least 200 characters in size.

int OXWebHtml::HttpTryOnce(const char *zUrl, const char *zLocalFile,
                           char *zLocation, char *zContent) {
  int i, j;
  int nErr = 0;			// Number of errors
  const char *zPath;		// Pathname to send as second argument to GET
  int iPort;			// TCP port for the server
  struct hostent *pHost;	// Name information
  int s;			// The main communications socket
  int c;			// A character read from the remote side
  int n;			// Number of characters in header
  int rc = 200;			// The result code
  FILE *sock;			// FILE corresponding to file descriptor s
  FILE *out;			// Write output here
  int last_was_nl;		// TRUE if last character received was '\n'
  struct sockaddr_in addr;	// The address structure
  int nByte = 0;
  char zIpAddr[400];		// The IP address of the server to print
  static char zMsg[1000];	// Space to hold error messages
  char zLine[1000];		// A single line of the header

  out = fopen(zLocalFile, "w");
  if (out == 0) {
    sprintf(zMsg, "Can't open output file \"%s\": %s", zLocalFile,
                  strerror(errno));
    throw OConnectException(zMsg);
  }

  i = 0;
  if (strncasecmp(zUrl, "http:", 5) == 0) i = 5;

  while (zUrl[i] == '/') i++;

  j = 0;
  while (zUrl[i] && zUrl[i] != ':' && zUrl[i] != '/') {
    if (j < sizeof(zIpAddr) - 1) zIpAddr[j++] = zUrl[i];
    i++;
  }
  zIpAddr[j] = 0;
  if (zUrl[i] == ':') {
    iPort = 0;
    i++;
    while (isdigit(zUrl[i])) {
      iPort = iPort * 10 + zUrl[i] - '0';
      i++;
    }
  } else {
    iPort = 80;
  }
  zPath = &zUrl[i];

  addr.sin_family = AF_INET;
  addr.sin_port = htons(iPort);
  addr.sin_addr.s_addr = inet_addr(zIpAddr);
  if (addr.sin_addr.s_addr == -1) {
    pHost = gethostbyname(zIpAddr);
    if (pHost == 0) {
      sprintf(zMsg, "Can't resolve host name: %s", zIpAddr);
      throw OConnectException(zMsg);
    }
    memcpy(&addr.sin_addr, pHost->h_addr_list[0], pHost->h_length);
#ifdef DEBUG_CONNECT
    fprintf(stderr, "Address resolution: %s -> %d.%d.%d.%d\n", zIpAddr,
                    pHost->h_addr_list[0][0] & 0xff,
                    pHost->h_addr_list[0][1] & 0xff,
                    pHost->h_addr_list[0][2] & 0xff,
                    pHost->h_addr_list[0][3] & 0xff);
#endif
  }
  s = socket(AF_INET, SOCK_STREAM, 0);
  if (s < 0) {
    sprintf(zMsg, "Can't open socket to %s: %s", zIpAddr, strerror(errno));
    fclose(out);
    throw OConnectException(zMsg);
  }
  if (connect(s, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
    sprintf(zMsg, "Can't connect to host %s: %s", zIpAddr, strerror(errno));
    fclose(out);
    throw OConnectException(zMsg);
  }
  sock = fdopen(s, "r+");
  if (*zPath == 0) zPath = "/";

  fprintf(sock, "GET %s HTTP/1.0\r\n", zPath);
  fprintf(sock, "User-Agent: Mozilla/2.0 (X11; U; Linux 0.99p17 i486)\r\n");
  if (iPort != 80) {
    fprintf(sock, "Host: %s:%d\r\n", zIpAddr, iPort);
  } else {
    fprintf(sock, "Host: %s\r\n", zIpAddr);
  }
  fprintf(sock, "Accept: image/gif, image/x-xbitmap, image/jpeg, */*\r\n");
  fprintf(sock, "\r\n");
  fflush(sock);
  n = 0;
  rc = 0;
  while ((c = getc(sock)) != EOF && (c != '\n' || !last_was_nl)) {
    if (c == '\r') continue;
    last_was_nl = (c == '\n');
    if (last_was_nl) {
      zLine[n] = 0;
      if (rc == 0) {
        sscanf(zLine, "HTTP/%*d.%*d %d ", &rc);
      } else if (strncmp(zLine, "Location:", 9) == 0 && zLocation) {
        const char *src = zLine + 9;
        while (isspace(*src)) ++src;
        char *dst = zLocation;
        int len = 0;
        while (*src && !isspace(*src) && (len < 199)) {
          *dst++ = *src++;
          ++len;
        }
        *dst = 0;
      } else if (strncasecmp(zLine, "Content-Type:", 13) == 0 && zContent) {
        const char *src = zLine + 13;
        while (isspace(*src)) ++src;
        char *dst = zContent;
        int len = 0;
        while (*src && !isspace(*src) && (len < 199)) {
          *dst++ = *src++;
          ++len;
        }
        *dst = 0;
      }
    }
    if (n < sizeof(zLine) - 1) zLine[n++] = c;
    if (last_was_nl) n = 0;
  }
  if (rc == 0) rc = 200;
  while ((c = getc(sock)) != EOF) {
    nByte++;
    putc(c, out);
  }
  fclose(sock);
  fclose(out);

  return rc;
}


// Get the file.  Take up to 7 redirects.
//
// zUrl       -- fetch this URL
// zLocalFile -- write to this file
// nActual    -- size of zActual[]
// zActual    -- write actual URL retrieved here
// zContent   -- write content type here (must be at least 200 chars in size)

int OXWebHtml::HttpFetch(const char *zUrl, const char *zLocalFile,
                         int nActual, char *zActual, char *zContent) {
  int i;
  int rc;
  char *zOriginalUrl = (char *) zUrl;
  char zLocation[300];

  for (i = 0; i < 7; i++) {
    rc = HttpTryOnce(zUrl, zLocalFile, zLocation, zContent);
    if (rc == 301 || rc == 302) {
      char *z;
      z = ResolveUri(zLocation);
      if (zUrl != zOriginalUrl) delete[] zUrl;
      zUrl = z;
    } else {
      break;
    }
  }
  if (nActual > 0 && zActual != 0) {
    sprintf(zActual, "%.*s", nActual, zUrl);
  }
  if (zUrl != zOriginalUrl) delete[] zUrl;

  return rc;
}
