/**************************************************************************
 *
 *  Prototypes for the routines to expand environment-variables into strings.
 *  Taken from the fvwm source code.
 *
 *  Written by Sverre H. Huseby <sverrehu@ifi.uio.no> 1995/10/3
 *
 **************************************************************************/

#ifndef __ENVVAR_H
#define __ENVVAR_H


int   envExpand(char *s, int maxstrlen);
char *envDupExpand(const char *s, int extra = 0);


#endif  // __ENVVAR_H
