/**************************************************************************

    This file is part of libfOX, a class library for the XClass95 toolkit.
    Copyright (C) 1998 by Michael McDonald.                 

    This application is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This application is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

**************************************************************************/

#ifndef __TCONTAINERS_H
#define __TCONTAINERS_H

#include "TDList.h"
#include "TStack.h"

// Predefined Arrays..

/*----------------------------------------------------------------------

 T      - is for templated :)
 I/D    - is storage type:
 		I for not calling Delete,
		D for calling Delete on an item.
 DLList - for the double linked list array.
 Stack  - for the double linked list stack..


 Predefined Objects
 ------------------

  Array's		Stack's
  -------		-------
  CharStringArray 	IntStack
  UnsignedIntArray	UnsignedIntStack
  IntArray		LongStack
  UnsignedLongArray	UnsignedLongStack
  LongArray		CharStack
  UnsignedCharArray	UnsignedCharStack
  CharArray		FloatStack
  FloatArray		DoubleStack
  DoubleArray		CharStringStack


 CharStringArray/CharStringStack **NOTE**

new calling Add make sure to call strdup or pass a pointer to a deletable
block. As the char * passed is considered as owned by the node.

------------------------------------------------------------------------

example for creating xclass arrays:

class OString;

typedef TDDLList<OString *> OStringArray;
typedef TDDLList<OXFrame *> OXFrameArray;
typedef TDDLList<OBasObject *> OBaseObjectArray;

main() {
  OStringArray st;
  int i;

  for (i = 0; i < 10; i++)
    st += new OString("Test String");

  for (i = 0; i < st.GetSize(); i++)
    printf("%d: %s\n", i, st[i]->GetString());
}

----------------------------------------------------------------------*/

// builtins..

typedef TDDLList<char *> CharStringArray;
typedef TIDLList<unsigned int> UnsignedIntArray;
typedef TIDLList<int> IntArray;
typedef TIDLList<unsigned long> UnsignedLongArray;
typedef TIDLList<long> LongArray;
typedef TIDLList<unsigned char> UnsignedCharArray;
typedef TIDLList<char> CharArray;
typedef TIDLList<float> FloatArray;
typedef TIDLList<double> DoubleArray;

// Predefined Stacks..

typedef TDStack<char *> CharStringStack;
typedef TIStack<int> IntStack;
typedef TIStack<unsigned int> UnsignedIntStack;
typedef TIStack<long> LongStack;
typedef TIStack<unsigned long> UnsignedLongStack;
typedef TIStack<char> CharStack;
typedef TIStack<unsigned char> UnsignedCharStack;
typedef TIStack<float> FloatStack;
typedef TIStack<double> DoubleStack;

#endif  // __TCONTAINERS_H
