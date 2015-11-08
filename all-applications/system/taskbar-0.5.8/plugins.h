#ifndef __PLUGINS_H
#define __PLUGINS_H

#include <xclass/OMessage.h>
#include <xclass/OString.h>


/*----------------------------------------------------------------------

  This include file is intended to be used by both the taskbar and
  the plug-in modules. It defines the message codes and a class for
  plug-in message objects.
  
  I was thinking about defining a virtual pure class with a set of
  methods to be used by plugins as a sort of interface class, so the
  taskbar could query plugins for some specific information. But, I'm
  not sure it would be even necessary. Right now plugins are just
  simple or composite frame objects, they are passed the information
  necessary to parse their commands from the rc file, and an ID number.
  The last is used when sending messages back to the taskbar. Plugin
  widgets act in the same way frame widgets do, i.e. they can receive
  mouse events, can make use of menus, etc.
  
  Plugins are compiled as dynamic lib modules, and since the taskbar
  is compiled with the -rdynamic flag, they do not need not to be
  linked statically to the xclass library, they would resolve their
  references from the symbols exported by the taskbar. That way we
  can keep the sizes of plugin modules very small.

  The only restriction to that method is that plugins can use only
  the same kind of objects that are in use by the taskbar. Of course,
  that restriction would not apply if xclass is compiled as shared
  library as well.

  ---

  An additional note: be sure to make use only of the GetItem method
  when parsing the configuration file, calling GetNext would confuse
  OXTaskBar.

----------------------------------------------------------------------*/


#define MSG_PLUGIN       (MSG_USERMSG+1800)

#define PLUGIN_HIDE      1
#define PLUGIN_SHOW      2
#define PLUGIN_UNLOAD    3
#define PLUGIN_CHANGED   4
#define PLUGIN_DOACTION  5


class OPluginMessage : public OWidgetMessage {
public:
  OPluginMessage(int typ, int act, int wid, char *cmd = NULL) :
    OWidgetMessage(typ, act, wid) {
      command = cmd ? new OString(cmd) : (OString *) NULL;
  }
  virtual ~OPluginMessage() { if (command) delete command; };
        
  OString *command;
};


#endif  // __PLUGINS_H
