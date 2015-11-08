/**************************************************************************

   This implementation of in-memory hash tables is based on the tcl hash
   table code.

   The tclHash.c file contains the following copyright notice:

     Copyright (c) 1991-1993 The Regents of the University of California.
     Copyright (c) 1994 Sun Microsystems, Inc.

     See the file "tcl.license.terms" for information on usage and
     redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.

     [ tclHash.c 1.16 96/04/29 10:30:49 ]

**************************************************************************/

#include <xclass/utils.h>
#include <xclass/OHashTable.h>


// When there are this many entries per bucket, on average, rebuild
// the hash table to make it larger.

#define REBUILD_MULTIPLIER	3


// The following macro takes a preliminary integer hash value and
// produces an index into a hash tables bucket list. The idea is
// to make it so that preliminary values that are arbitrarily similar
// will end up in different buckets. The hash function was taken
// from a random-number generator.

#define RANDOM_INDEX(i) \
    (((((long) (i))*1103515245) >> downShift) & mask)


//----------------------------------------------------------------------

OHashTable::OHashTable() {
  buckets = staticBuckets;
  staticBuckets[0] = staticBuckets[1] = 0;
  staticBuckets[2] = staticBuckets[3] = 0;
  numBuckets = SMALL_HASH_TABLE;
  numEntries = 0;
  rebuildSize = SMALL_HASH_TABLE * REBUILD_MULTIPLIER;
  downShift = 28;
  mask = 3;
}

OHashTable::~OHashTable() {
  register OHashEntry *h, *next;
  int i;

  // Free up all the entries in the table.
  for (i = 0; i < numBuckets; i++) {
    h = buckets[i];
    while (h != NULL) {
      next = h->next;
      delete h;
      h = next;
    }
  }

  // Free up the bucket array, if it was dynamically allocated.
  if (buckets != staticBuckets) delete[] buckets;
}


//----------------------------------------------------------------------

// Remove a single entry from a hash table.
//
// The entry given by "entry" is deleted from its table and should never
// again be used by the caller. It is up to the caller to free the clientData
// field of the entry, if that is relevant.

void OHashTable::DeleteHashEntry(OHashEntry *entry) {
  register OHashEntry *prev;

  if (*entry->bucket == entry) {
    *entry->bucket = entry->next;
  } else {
    for (prev = *entry->bucket; ; prev = prev->next) {
      if (prev == NULL)
	FatalError("Malformed bucket chain in OHashTable::DeleteHashEntry()");
      if (prev->next == entry) {
	prev->next = entry->next;
	break;
      }
    }
  }
  entry->table->numEntries--;

  delete entry;
}

OHashEntry::~OHashEntry() {
/* do not delete entries directly from the application!!!
  register OHashEntry *prev;

  if (*bucket == this) {
    *bucket = next;
  } else {
    for (prev = *bucket; ; prev = prev->next) {
      if (prev == NULL)
	FatalError("Malformed bucket chain found during OHashEntry deletion");
      if (prev->next == this) {
	prev->next = next;
	break;
      }
    }
  }
  table->numEntries--;
*/
}


//----------------------------------------------------------------------

// Locate the first entry in a hash table and set up a record that can
// be used to step through all the remaining entries of the table.
//
// The return value is a pointer to the first entry in the table, or NULL
// if the table has no entries in it. The memory at *search is initialized
// so that subsequent calls to NextHashEntry will return all of the entries
// in the table, one at a time.

OHashEntry *OHashTable::FirstHashEntry(OHashSearch *search) {
  search->table = this;
  search->nextIndex = 0;
  search->nextEntry = NULL;
  return NextHashEntry(search);
}

//----------------------------------------------------------------------

// Once a hash table enumeration has been initiated by calling
// FirstHashEntry, this procedure may be called to return successive
// elements of the table.
//
// The return value is the next entry in the hash table being enumerated,
// or NULL if the end of the table is reached.

OHashEntry *OHashTable::NextHashEntry(OHashSearch *search) {
  OHashEntry *h;

  while (search->nextEntry == NULL) {
    if (search->nextIndex >= search->table->numBuckets) {
      return NULL;
    }
    search->nextEntry =	search->table->buckets[search->nextIndex];
    search->nextIndex++;
  }
  h = search->nextEntry;
  search->nextEntry = h->next;
  return h;
}


//----------------------------------------------------------------------

// Return statistics describing the layout of the hash table in its hash
// buckets.
//
// The return value is a new-ed string containing information about the
// table. It is the caller's responsibility to delete[] this string.

char *OHashTable::HashStats() {
#define NUM_COUNTERS 10
  int count[NUM_COUNTERS], overflow, i, j;
  double average, tmp;
  register OHashEntry *h;
  char *result, *p;

  // Compute a histogram of bucket usage.

  for (i = 0; i < NUM_COUNTERS; i++) count[i] = 0;
  overflow = 0;
  average = 0.0;
  for (i = 0; i < numBuckets; i++) {
    j = 0;
    for (h = buckets[i]; h != NULL; h = h->next) ++j;
    if (j < NUM_COUNTERS) {
      count[j]++;
    } else {
      overflow++;
    }
    tmp = j;
    average += (tmp + 1.0) * (tmp / numEntries) / 2.0;
  }

  // Print out the histogram and a few other pieces of information.

  result = new char[(unsigned) ((NUM_COUNTERS * 60) + 300)];
  sprintf(result, "%d entries in table, %d buckets\n",
	           numEntries, numBuckets);
  p = result + strlen(result);
  for (i = 0; i < NUM_COUNTERS; i++) {
    sprintf(p, "Number of buckets with %d entries: %d\n",
 	        i, count[i]);
    p += strlen(p);
  }
  sprintf(p, "Number of buckets with %d or more entries: %d\n",
	      NUM_COUNTERS, overflow);
  p += strlen(p);
  sprintf(p, "Average search distance for entry: %.1f", average);
  return result;
}


//----------------------------------------------------------------------

// Compute a one-word summary of a text string, which can be used to
// generate a hash index.
//
// The return value is a one-word summary of the information in string.

unsigned int OStringHashTable::HashString(const char *string) {
  register unsigned int result;
  register int c;

  // I tried a zillion different hash functions and asked many other
  // people for advice. Many people had their own favorite functions,
  // all different, but no-one had much idea why they were good ones.
  // I chose the one below (multiply by 9 and add new character)
  // because of the following reasons:
  //
  // 1. Multiplying by 10 is perfect for keys that are decimal strings,
  //    and multiplying by 9 is just about as good.
  // 2. Times-9 is (shift-left-3) plus (old). This means that each
  //    character's bits hang around in the low-order bits of the
  //    hash value for ever, plus they spread fairly rapidly up to
  //    the high-order bits to fill out the hash value. This seems
  //    works well both for decimal and non-decimal strings.

  result = 0;
  while (1) {
    c = *string++;
    if (c == 0) break;
    result += (result << 3) + c;
  }
  return result;
}


//----------------------------------------------------------------------

// Given a hash table with string keys, and a string key, find the entry
// with a matching key.
//
// The return value is a token for the matching entry in the hash table,
// or NULL if there was no matching entry.

OHashEntry *OStringHashTable::Find(const char *key) {
  register OHashEntry *h;
  register const char *p1, *p2;
  int index;

  index = HashString(key) & mask;

  // Search all of the entries in the appropriate bucket.

  for (h = buckets[index]; h != NULL; h = h->next) {
    OStringHashEntry *sh = (OStringHashEntry *) h;
    for (p1 = key, p2 = sh->keyString; ; p1++, p2++) {
      if (*p1 != *p2) break;
      if (*p1 == '\0') return h;
    }
  }

  return NULL;
}


//----------------------------------------------------------------------

// Given a hash table with string keys, and a string key, find the entry
// with a matching key. If there is no matching entry, then create a new
// entry that does match.
//
// The return value is a pointer to the matching entry. If this is a
// newly-created entry, then *inew will be set to a non-zero value;
// otherwise *inew will be set to 0. If this is a new entry the value
// stored in the entry will initially be 0.

OHashEntry *OStringHashTable::Create(const char *key, int *inew) {
  register OHashEntry *h;
  register const char *p1, *p2;
  int index;

  index = HashString(key) & mask;

  // Search all of the entries in this bucket.

  for (h = buckets[index]; h != NULL; h = h->next) {
    OStringHashEntry *sh = (OStringHashEntry *) h;
    for (p1 = key, p2 = sh->keyString;; p1++, p2++) {
      if (*p1 != *p2) break;
      if (*p1 == '\0') {
        *inew = 0;
	return h;
      }
    }
  }

  // Entry not found. Add a new one to the bucket.

  *inew = 1;
  h = (OHashEntry *) new OStringHashEntry((char *) key);
  h->table = this;
  h->bucket = &buckets[index];
  h->next = *h->bucket;
  h->clientData = 0;
  *h->bucket = h;
  numEntries++;

  // If the table has exceeded a decent size, rebuild it with many
  // more buckets.

  if (numEntries >= rebuildSize) Rebuild();

  return h;
}


//----------------------------------------------------------------------

// Given a hash table with one-word keys, and a one-word key, find the
// entry with a matching key.
//
// The return value is a token for the matching entry in the hash table,
// or NULL if there was no matching entry.

OHashEntry *OIntHashTable::Find(const char *key) {
  register OHashEntry *h;
  int index;

  index = RANDOM_INDEX(key);

  // Search all of the entries in the appropriate bucket.

  for (h = buckets[index]; h != NULL; h = h->next) {
    OIntHashEntry *ih = (OIntHashEntry *) h;
    if (ih->keyOneWord == key) return h;
  }
  return NULL;
}


//----------------------------------------------------------------------

// Given a hash table with one-word keys, and a one-word key, find the
// entry with a matching key. If there is no matching entry, then create
// a new entry that does match.
//
// The return value is a pointer to the matching entry. If this is a
// newly-created entry, then *inew will be set to a non-zero value;
// otherwise *inew will be set to 0. If this is a new entry the value
// stored in the entry will initially be 0.

OHashEntry *OIntHashTable::Create(const char *key, int *inew) {
  register OHashEntry *h;
  int index;

  index = RANDOM_INDEX(key);

  // Search all of the entries in this bucket.

  for (h = buckets[index]; h != NULL; h = h->next) {
    OIntHashEntry *ih = (OIntHashEntry *) h;
    if (ih->keyOneWord == key) {
      *inew = 0;
      return h;
    }
  }

  // Entry not found.  Add a new one to the bucket.

  *inew = 1;
  h = (OHashEntry *) new OIntHashEntry((char *) key);
  h->table = this;
  h->bucket = &buckets[index];
  h->next = *h->bucket;
  h->clientData = 0;
  *h->bucket = h;
  numEntries++;

  // If the table has exceeded a decent size, rebuild it with many
  // more buckets.

  if (numEntries >= rebuildSize) Rebuild();

  return h;
}


//----------------------------------------------------------------------

// Given a hash table with array-of-int keys, and a key, find the entry
// with a matching key.
//
// The return value is a token for the matching entry in the hash table,
// or NULL if there was no matching entry.

OHashEntry *OArrayHashTable::Find(const char *key) {
  register OHashEntry *h;
  int *array = (int *) key;
  register int *iPtr1, *iPtr2;
  int index, count;

  for (index = 0, count = keySize, iPtr1 = array;
       count > 0; count--, iPtr1++) {
    index += *iPtr1;
  }
  index = RANDOM_INDEX(index);

  // Search all of the entries in the appropriate bucket.

  for (h = buckets[index]; h != NULL; h = h->next) {
    OArrayHashEntry *ah = (OArrayHashEntry *) h;
    for (iPtr1 = array, iPtr2 = ah->keyWords,
	 count = keySize; ; count--, iPtr1++, iPtr2++) {
      if (count == 0) return h;
      if (*iPtr1 != *iPtr2) break;
    }
  }
  return NULL;
}


//----------------------------------------------------------------------

// Given a hash table with one-word keys, and a one-word key, find the
// entry with a matching key. If there is no matching entry, then create
// a new entry that does match.
//
// The return value is a pointer to the matching entry. If this is a
// newly-created entry, then *inew will be set to a non-zero value;
// otherwise *inew will be set to 0. If this is a new entry the value
// stored in the entry will initially be 0.

OHashEntry *OArrayHashTable::Create(const char *key, int *inew) {
  register OHashEntry *h;
  int *array = (int *) key;
  register int *iPtr1, *iPtr2;
  int index, count;

  for (index = 0, count = keySize, iPtr1 = array;
       count > 0; count--, iPtr1++) {
    index += *iPtr1;
  }
  index = RANDOM_INDEX(index);

  // Search all of the entries in the appropriate bucket.

  for (h = buckets[index]; h != NULL; h = h->next) {
    OArrayHashEntry *ah = (OArrayHashEntry *) h;
    for (iPtr1 = array, iPtr2 = ah->keyWords,
	 count = keySize; ; count--, iPtr1++, iPtr2++) {
      if (count == 0) {
	*inew = 0;
	return h;
      }
      if (*iPtr1 != *iPtr2) break;
    }
  }

  // Entry not found. Add a new one to the bucket.

  *inew = 1;
  h = (OHashEntry *) new OArrayHashEntry(array, keySize);
  h->table = this;
  h->bucket = &buckets[index];
  h->next = *h->bucket;
  h->clientData = 0;
  *h->bucket = h;
  numEntries++;

  // If the table has exceeded a decent size, rebuild it with many
  // more buckets.

  if (numEntries >= rebuildSize) Rebuild();

  return h;
}


//----------------------------------------------------------------------

// This procedure is invoked when the ratio of entries to hash buckets
// becomes too large. It creates a new table with a larger bucket array
// and moves all of the entries into the new table.
//
// Memory gets reallocated and entries get re-hashed to new buckets.

void OHashTable::Rebuild() {
  int oldSize, count, index;
  OHashEntry **oldBuckets;
  register OHashEntry **oldChain, **newChain;
  register OHashEntry *h;

  oldSize = numBuckets;
  oldBuckets = buckets;

  // Allocate and initialize the new bucket array, and set up
  // hashing constants for new array size.

  numBuckets *= 4;
  buckets = new OHashEntry* [numBuckets];
      for (count = numBuckets, newChain = buckets;
	   count > 0; count--, newChain++) {
    *newChain = NULL;
  }
  rebuildSize *= 4;
  downShift -= 2;
  mask = (mask << 2) + 3;

  // Rehash all of the existing entries into the new bucket array.

  for (oldChain = oldBuckets; oldSize > 0; oldSize--, oldChain++) {
    for (h = *oldChain; h != NULL; h = *oldChain) {
      *oldChain = h->next;
      index = GetIndex(h);
      h->bucket = &(buckets[index]);
      h->next = *h->bucket;
      *h->bucket = h;
    }
  }

  // Free up the old bucket array, if it was dynamically allocated.

  if (oldBuckets != staticBuckets) delete[] oldBuckets;
}

int OStringHashTable::GetIndex(OHashEntry *h) {
  return HashString(((OStringHashEntry *)h)->keyString) & mask;
}

int OIntHashTable::GetIndex(OHashEntry *h) {
  return (RANDOM_INDEX(((OIntHashEntry *)h)->keyOneWord));
}

int OArrayHashTable::GetIndex(OHashEntry *h) {
  register int *iPtr;
  int index, count;

  for (index = 0, count = keySize, iPtr = ((OArrayHashEntry *)h)->keyWords;
       count > 0;
       count--, iPtr++) {
    index += *iPtr;
  }
  return (RANDOM_INDEX(index));
}
