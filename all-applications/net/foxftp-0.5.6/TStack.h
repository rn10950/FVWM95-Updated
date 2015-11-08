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

#ifndef __TSTACK_H
#define __TSTACK_H


//----------------------------------------------------------------------
//	Simple Templated Stack class.
//
//	TIStack is for use with builtin type (No memory Management)
//	TDStack is for use with pointers to objects (deletes data)

template <class T>
class TIStack {
protected:
  struct Node {
    T Data;
    Node *Next;
    Node *Prev;
    Node(T data) : Next(0), Prev(0), Data(data) {}
    ~Node() {
      if (Next) Next->Prev = Prev;
      if (Prev) Prev->Next = Next;
    }
  };

  int Count;
  Node *Head;
  Node *Tail;

public:
  TIStack() : Count(0), Head(0), Tail(0) {}

  T Pop() {
    if (!Tail) return (T) 0;
    T tmp = Tail->Data;
    Node *n = Tail->Prev;
    delete Tail;
    Tail = n;
    Count--;
    return tmp;
  }

  int Push(T data){
    Node *n = new Node(data);
    if (!n) return false;
    if (Tail) {
      Tail->Next = n;
      n->Prev = Tail;
      Tail = n;
    } else {
      Head = Tail = n;
    }
    Count++;
    return true;
  }

  int GetSize() { return Count; }

};

template <class T>
class TDStack {
protected:
  struct Node {
    T Data;
    Node *Next;
    Node *Prev;
    Node(T data) : Next(0), Prev(0), Data(data) {}
    ~Node() {
      if (Next) Next->Prev = Prev;
      if (Prev) Prev->Next = Next;
      if (Data) delete Data;
    }
  };

  int Count;
  Node *Head;
  Node *Tail;

public:
  TDStack() : Count(0), Head(0), Tail(0) {}
  T Pop() {
    if (!Tail) return (T) 0;
    T tmp = Tail->Data;
    Node *n = Tail->Prev;
    delete Tail;
    Tail = n;
    Count--;
    return tmp;
  }

  int Push(T data) {
    Node *n = new Node(data);
    if (!n) return false;
    if (Tail) {
      Tail->Next = n;
      n->Prev = Tail;
      Tail = n;
    } else {
      Head = Tail = n;
    }
    Count++;
    return true;
  }

  int GetSize() { return Count; }

};

#endif  // __TSTACK_H
