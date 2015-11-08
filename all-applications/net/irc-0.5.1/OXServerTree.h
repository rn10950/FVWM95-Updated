#ifndef __SERVERTREE_H
#define __SERVERTREE_H

#include <vector>

#include <xclass/OXTransientFrame.h>
#include <xclass/OXListTree.h>

class OXIrc;
class OXLabel;
class OXButton;


//----------------------------------------------------------------------

class OServerLink : public OBaseObject {
public:
  OServerLink(const char *srvname, const char *conn,
              const char *msg, int hopcnt);
  virtual ~OServerLink();

public:
  char *serverName;
  char *connectedTo;  
  char *serverMsg;
  int  hop;
};


class OLinkTreeItem : public OListTreeItem {
public:
  OLinkTreeItem(OXClient *_client, const char *name, int i);

public:
  int id;
};
        

class OXServerTree : public OXTransientFrame {
public:
  OXServerTree(const OXWindow *p, const OXWindow *main,
               OXIrc *irc, int w, int h);
  virtual ~OXServerTree();

  virtual int ProcessMessage(OMessage *msg);
  virtual int CloseWindow();

  void Clear();
  void AddLink(OServerLink *link);
  void BuildTree();
  
protected:
  OListTreeItem *_FindServer(OListTreeItem *root,
                             const char *name, int hopcnt);
  void _UpdateLabels(OServerLink *link = NULL);

  OXIrc *_irc;
  OXListTree *_listTree;
  OXLabel *_servName, *_connTo, *_servMsg;
  OXButton *_conn, *_cl;
  std::vector<OServerLink *> _links;
  int _clearPending;
};


#endif  // __OXLINKSVIEW_H
