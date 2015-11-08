#ifndef __OXWEBHTML_H
#define __OXWEBHTML_H

#include <vector>

#include <xclass/utils.h>
#include <xclass/OException.h>
#include <xclass/OHtmlUri.h>
#include <xclass/OXHtml.h>


//----------------------------------------------------------------------

class OConnectException : public OException {
public:
  OConnectException(char *msg) { _msg = msg; }
};

class OXWebHtml : public OXHtml {
public:
  OXWebHtml(const OXWindow *p, int w, int h, int id = -1);
  virtual ~OXWebHtml();

  virtual void Clear();
  virtual int  IsVisited(const char *url);
  virtual int  ProcessToken(OHtmlElement *pElem, const char *name, int type);
  virtual int  FormAction(OHtmlForm *form, int id);
        
  virtual OImage *LoadImage(const char *uri, int w = 0, int h = 0);
        
  void AddToVisited(const char *url);
  int  HttpFetch(const char *zUrl, const char *zLocalFile,
                 int nActual, char *zActual, char *zContent);

  char *GetDocName() const { return docName; }

protected:
  int HttpTryOnce(const char *zUrl, const char *zLocalFile,
                  char *zLocation, char *zContent);
  char *HttpGetImage(const char *zUrl);

  // Each image to be loaded is an instance of the following structure
  struct Image {
    char *zUrl;                   // The URL for this image
    char *zLocal;                 // The local filename
    Image *pNext;                 // Next in a list of them all
  };
  Image *imageCache;
  int nCachedImages;              // Number of images loaded so far

  char *docName;
  int docNameStart;
  
  std::vector<char *> visitedUrl;
};


#endif  // __OXWEBHTML_H
