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

#ifndef __TDLLIST_H
#define __TDLLIST_H

/*
 * Templated Inline Array Using double linked lists
 *
 *
 * Requires classes to have valid copy constructor and assignment operator.
 * --------
 *
 *	TIDLList is for use with builtin types or types that require no
 *		 memory management.
 *
 *	TDDLList is for use with pointers to objects (it deletes the object)
 *
 *
 * [int] is 0 based.
 * (int) is 1 based.
 *
 * Interals are 1 based..0 is for empty array.
 * ---------------------------------------------------------------------
 * 
 * Notes
 * -----
 * Add(item) adds to the end and doesn't move the work pointer.
 * += item Calls Add(item)
 * AddAfterWork(item) adds to the Works next pointer and moves the Work
 *		pointer to item.
 * To(int) is relative to the Work pointer using n=n->Next or n=n->Prev loops.
 * GetLoc() returns the 1 based int count of the work item.
 * Remove(int) moves the work pointer and removes the item leaving the work
 * 		on Work->Prev
 *
 *
 */

template <class T> class TIDLList {

  // You are respossible for the objects memory.
  // The node will only clean itself.

  protected:
    struct Node {
      T    Data;
      Node *Next;
      Node *Prev;
      Node(T item) : Data(item), Next(0), Prev(0) {}
      ~Node() {}
    };
    
  //------------------ State Members
  int Count, Current;
  Node *Head, *Tail, *Work;

public:
  TIDLList(): Count(0), Current(0), Head(0), Tail(0), Work(0) {}
  ~TIDLList() { Erase(); }
    
  //------------------ Access Operators
  T operator[] (int i)       { return GetAt(i+1); }
  T operator() (int i)       { return GetAt(i); }
  TIDLList<T> &operator += (T item) { Add(item); return *this; } 
  TIDLList<T> &operator += (TIDLList<T> &arr) {
    for (int i = 0; i < arr.GetSize(); i++) Add(arr[i]);
    return *this;
  }
  TIDLList<T> &operator = (TIDLList<T> &arr) {
    Erase();
    for (int i = 0; i < arr.GetSize(); i++) Add(arr[i]);
    return *this;
  }
    
  // Add Members: Notice.. These Move the Work Pointer
  bool Add(T item) { return AddAfterEnd(item); }
  bool AddAfterEnd(T item) {
    Node *n = new Node(item);
    if (Tail) {
      n->Prev = Tail;
      Tail->Next = n;
      Tail = n;
      Count++;
    } else {
      Head = Tail = Work = n;
      Current = Count = 1;
    }
    return true;
  }
  bool AddAfterWork(T item) {
    Node *n = new Node(item);
    if (Work) {
      n->Prev = Work;
      n->Next = Work->Next;
      if (Work->Next)
        Work->Next->Prev = n;
      else
        Tail = n;
      Work->Next = n;
      Count++;
    } else {
      Work = Head = Tail = n;
      Count = Current = 1;
    }
    return true;
  }

  // Misc Members

  void Erase() {
    while (Work = Head) {
      Head = Work->Next;
      delete Work;
    }
    Head = Tail = Work = 0;
    Count = Current = 0;
  }
  int GetSize() { return Count; }
  int GetLoc()  { return Current; }
  bool AtTail() { return (Work == Tail); }
  bool AtHead() { return (Work == Head); }
  bool Remove(int i) {
    Node *c, *p, *n;
    To(i);
    p = Work->Prev;
    n = Work->Next;
    c = n;
    if (p == NULL) {
      Head = n;
    } else {
      p->Next = n;
    }
    if (n == NULL) {
      Tail = p;
      c = p;
      Current--;
    } else {
      n->Prev = p;
    }
    delete Work;
    Work = c;
    Count--;
    return true;
  }

  // Get Members. All Move the Work pointer

  T GetAt(int i) { To(i); return Look(); }

  // these use a little hack to look back after moving, that way
  // they return the current then move the pointer. At least that's
  // what it's supposed to look like :)

  T GetNext() {
    if (Work->Next) {
      Work = Work->Next;
      Current++;
      return Look();
    } else {
      return 0;
    }
  }
  T GetPrev() {
    if (Work->Prev) {
      Work = Work->Prev;
      Current--;
      return Look();
    } else {
      return 0;
    }
  }
  T GetHead() { ToHead(); return Look(); }
  T GetTail() { ToTail(); return Look(); }

  // Look Members do not move the Work pointer

  T LookNext() { 
    if (Work->Next) {
      return Work->Next->Data;  
    } else {
      return 0;
    }
  }
  T LookPrev() { 
    if (Work->Prev) {
      return Work->Prev->Data;  
    } else {
      return 0;
    }
  }
  T LookHead() {
    if (Head) {
      return Head->Data;
    } else {
      return 0;
    }
  }
  T LookTail() {
    if (Tail) {
      return Tail->Data;
    } else {
      return 0;
    }
  }
  T Look() {
    if (Work) {
      return Work->Data;
    } else {
      return 0;
    }
  }

  // Move Work pointer Members

  void ToTail() { Work = Tail; Current = Count; }
  void ToHead() { Work = Head; Current = 1; }
  void Forward(int i = 1) {
    while ((i-->0) && (Work->Next)) {
      Work = Work->Next;
      Current++;
    }
  }
  void Backward(int i = 1) {
    while ((i-->0) && (Work->Prev)) {
      Work = Work->Prev;
      Current--;
    }
  }
  void To(int i) {
    if (i > Current) Forward(i - Current);
    if (i < Current) Backward(Current - i);
  }
}; 

template <class T> class TDDLList {

  // The node will only clean itself and the data memory.

protected:
  struct Node {
    T    Data;
    Node *Next;
    Node *Prev;
    Node(T item) : Data(item), Next(0), Prev(0) {}
    ~Node() { if (Data) delete Data; }
  };
    
  //------------------ State Members
  int Count, Current;
  Node *Head, *Tail, *Work;

public:
  TDDLList(): Count(0), Current(0), Head(0), Tail(0), Work(0) {}
  ~TDDLList() { Erase(); }
    
  //------------------ Access Operators
  T operator[] (int i) { return GetAt(i+1); }
  T operator() (int i) { return GetAt(i); }
  TDDLList<T> &operator += (T item) { Add(item); return *this; } 
  TDDLList<T> &operator += (TDDLList<T> &arr) {
    for (int i = 0; i < arr.GetSize(); i++) Add(arr[i]);
    return *this;
  }
  TDDLList<T> &operator = (TDDLList<T> &arr) {
    Erase();
    for (int i = 0; i < arr.GetSize(); i++) Add(arr[i]);
    return *this;
  }

  // Add Members: Notice.. These Move the Work Pointer
  bool Add(T item) { return AddAfterEnd(item); }
  bool AddAfterEnd(T item) {
    Node *n = new Node(item);
    if (Tail) {
      n->Prev = Tail;
      Tail->Next = n;
      Tail = n;
      Count++;
    } else {
      Head = Tail = Work = n;
      Current = Count = 1;
    }
    return true;
  }
  bool AddAfterWork(T item) {
    Node *n = new Node(item);
    if (Work) {
      n->Prev = Work;
      n->Next = Work->Next;
      if (Work->Next)
        Work->Next->Prev = n;
      else
        Tail = n;
      Work->Next = n;
      Count++;
    } else {
      Work = Head = Tail = n;
      Count = Current = 1;
    }
    return true;
  }

  // Misc Members

  void Erase() {
    while (Work = Head) {
      Head = Work->Next;
      delete Work;
    }
    Head = Tail = Work = 0;
    Count = Current = 0;
  }
  int GetSize() { return Count; }
  int GetLoc()  { return Current; }
  bool AtTail() { return (Work == Tail); }
  bool AtHead() { return (Work == Head); }
  bool Remove(int i) {
    Node *c, *p, *n;
    To(i);
    p = Work->Prev;
    n = Work->Next;
    c = n;
    if (p == NULL) {
      Head = n;
    } else {
      p->Next = n;
    }
    if (n == NULL) {
      Tail = p;
      c = p;
      Current--;
    } else {
      n->Prev = p;
    }
    delete Work;
    Work = c;
    Count--;
    return true;
  }

  // Get Members. All Move the Work pointer

  T GetAt(int i) { To(i); return Look(); }

  // these use a little hack to look back after moving, that way
  // they return the current then move the pointer. At least that's
  // what it's supposed to look like :)

  T GetNext() {
    if (Work->Next) {
      Work = Work->Next;
      Current++;
      return Look();
    } else {
      return 0;
    }
  }
  T GetPrev() {
    if (Work->Prev) {
      Work = Work->Prev;
      Current--;
      return Look();
    } else {
      return 0;
    }
  }
  T GetHead() { ToHead(); return Look(); }
  T GetTail() { ToTail(); return Look(); }

  // Look Members do not move the Work pointer

  T LookNext() { 
    if (Work->Next) {
      return Work->Next->Data;  
    } else {
      return 0;
    }
  }
  T LookPrev() { 
    if (Work->Prev) {
      return Work->Prev->Data;  
    } else {
      return 0;
    }
  }
  T LookHead() {
    if (Head) {
      return Head->Data;
    } else {
      return 0;
    }
  }
  T LookTail() {
    if (Tail) {
      return Tail->Data;
    } else {
      return 0;
    }
  }
  T Look() {
    if (Work) {
      return Work->Data;
    } else {
      return 0;
    }
  }

  // Move Work pointer Members

  void ToTail() { Work = Tail; Current = Count; }
  void ToHead() { Work = Head; Current = 1; }
  void Forward(int i = 1) {
    while ((i-->0) && (Work->Next)) {
      Work = Work->Next;
      Current++;
    }
  }
  void Backward(int i = 1) {
    while ((i-->0) && (Work->Prev)) {
      Work = Work->Prev;
      Current--;
    }
  }
  void To(int i) {
    if (i > Current) Forward(i - Current);
    if (i < Current) Backward(Current - i);
  }
}; 


#endif  // __TDLLIST_H
