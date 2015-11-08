/**************************************************************************

   This hash table implementation is based on the tcl hash table code.

   The tcl.h file contains the following copyright notice:

     Copyright (c) 1987-1994 The Regents of the University of California.
     Copyright (c) 1994-1997 Sun Microsystems, Inc.
     Copyright (c) 1993-1996 Lucent Technologies.

     See the file "tcl.license.terms" for information on usage and
     redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.

     [ tcl.h 1.24 98/08/06 12:10:41 ]

**************************************************************************/

#ifndef __OHASHTABLE_H
#define __OHASHTABLE_H

#include <string.h>
#include <xclass/OBaseObject.h>


class OHashTable;
class OHashSearch;

typedef void* ClientData;


//----------------------------------------------------------------------

// Class definition for an entry in a hash table.

class OHashEntry : public OBaseObject {
protected:
  OHashEntry() {}
  virtual ~OHashEntry();

  OHashEntry *next;		// Pointer to next entry in this hash bucket,
  				// or NULL for end of chain.
  OHashTable *table;		// Pointer to table containing entry.
  OHashEntry **bucket;		// Pointer to bucket that points to first
  				// entry in this entry's chain, used for
  				// deleting the entry.

  ClientData clientData;	// Application stores something here
				// with SetHashValue.

  friend class OHashTable;
  friend class OStringHashTable;
  friend class OIntHashTable;
  friend class OArrayHashTable;

public:
  ClientData GetHashValue() const { return clientData; }
  void SetHashValue(const ClientData data) { clientData = data; }
  
  virtual char *GetHashKey() const = 0;
};

class OStringHashEntry : public OHashEntry {
protected:
  OStringHashEntry(char *key) {
    keyString = new char[strlen(key)+1];
    strcpy(keyString, key);
  }
  virtual ~OStringHashEntry() { delete[] keyString; }

  virtual char *GetHashKey() const { return keyString; }

  friend class OStringHashTable;

protected:
  char *keyString;
};

class OIntHashEntry : public OHashEntry {
protected:
  OIntHashEntry(char *key) { keyOneWord = key; }

  virtual char *GetHashKey() const { return keyOneWord; }

  friend class OIntHashTable;
  
protected:
  char *keyOneWord;
};

class OArrayHashEntry : public OHashEntry {
protected:
  OArrayHashEntry(int *key, int size) {
    keyWords = new int[size];
    for (int i=0; i<size; ++i) keyWords[i] = key[i];
  }
  virtual ~OArrayHashEntry() {
    delete[] keyWords;
  }

  virtual char *GetHashKey() const { return (char *) keyWords; }

  friend class OArrayHashTable;
  
protected:
  int *keyWords;
  int keySize;
};

//-------------------------------------------------------------------

// Class definition for a generic hash table.

#define SMALL_HASH_TABLE 4

class OHashTable : public OBaseObject {
public:
  OHashTable();
  virtual ~OHashTable();

  OHashEntry *FirstHashEntry(OHashSearch *search);
  OHashEntry *NextHashEntry(OHashSearch *search);
  char *HashStats();
  int  NumEntries() const { return numEntries; }

  void DeleteHashEntry(OHashEntry *entry);
  virtual OHashEntry *Find(const char *key) = 0;
  virtual OHashEntry *Create(const char *key, int *inew) = 0;

  friend class OHashEntry;

protected:
  void Rebuild();
  virtual int GetIndex(OHashEntry *h) = 0;

  OHashEntry **buckets;	// Pointer to bucket array. Each element points to
  			// first entry in bucket's hash chain, or NULL.
  OHashEntry *staticBuckets[SMALL_HASH_TABLE];
			// Bucket array used for small tables (to avoid
			// mallocs and frees).
  int numBuckets;	// Total number of buckets allocated at **buckets.
  int numEntries;	// Total number of entries present in table.
  int rebuildSize;	// Enlarge table when numEntries gets to be this large.
  int downShift;	// Shift count used in hashing function. Designed to
  			// use high-order bits of randomized keys.
  int mask;		// Mask value used in hashing function.
};

class OStringHashTable : public OHashTable {
public:
  virtual OHashEntry *Find(const char *key);
  virtual OHashEntry *Create(const char *key, int *inew);
  
protected:
  virtual  int GetIndex(OHashEntry *h);
  unsigned int HashString(const char *string);
};

class OIntHashTable : public OHashTable {
public:
  virtual OHashEntry *Find(const char *key);
  virtual OHashEntry *Create(const char *key, int *inew);

protected:
  virtual int GetIndex(OHashEntry *h);
};

class OArrayHashTable : public OHashTable {
public:
  OArrayHashTable(int arraySize) { keySize = arraySize; }

  virtual OHashEntry *Find(const char *key);
  virtual OHashEntry *Create(const char *key, int *inew);

protected:
  virtual int GetIndex(OHashEntry *h);
  
  int keySize;
};


// Structure definition for information used to keep track of searches
// through hash tables:

class OHashSearch : public OBaseObject {

  friend class OHashTable;

  OHashTable *table;		// Table being searched.
  int nextIndex;		// Index of next bucket to be enumerated
  				// after present one.
  OHashEntry *nextEntry;	// Next entry to be enumerated in the
				// the current bucket.
};


#endif  // __OHASHTABLE_H
