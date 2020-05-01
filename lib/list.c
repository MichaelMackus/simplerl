/* BSD 3-Clause License
 *
 * Copyright © 2008-2019, Jice and the libtcod contributors.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "list.h"

#include <stdlib.h> /* calloc */
#include <string.h> /* NULL/memcpy */

#define MIN(a,b) (((a)<(b))?(a):(b))

/**
 *  A lightweight and generic container that provides array, list, and stack
 *  paradigms.
 */
struct _RL_List {
  /**
   *  A pointer to an array of void pointers.
   */
  void **array;
  /** The current count of items in the array. */
  int fillSize;
  /** The maximum number of items that `array` can currently hold. */
  int allocSize;
};
/**
 *  Initialize or expand the array of a RL_List *struct.
 *
 *  If `l->allocSize` is zero then a new array is allocated, and allocSize is
 *  set.  If `l->allocSize` is not zero then the allocated size is doubled.
 */
static void RL_list_allocate_int(RL_List *l) {
  void **newArray;
  int newSize = l->allocSize * 2;
  if (newSize == 0) { newSize = 16; }
  newArray = (void **)calloc(sizeof(void *), newSize);
  if (l->array) {
    if (l->fillSize > 0) {
      memcpy(newArray, l->array, sizeof(void *) * l->fillSize);
    }
    free(l->array);
  }
  l->array = newArray;
  l->allocSize = newSize;
}
/**
 *  Set the number of items in a list.
 *
 *  The new size will be no more than `l->allocSize`.
 */
void RL_list_set_size(RL_List *l, int size) {
  l->fillSize = MIN(size, l->allocSize);
}
/**
 *  Return a new list.
 */
RL_List *RL_list_new(void) {
  return (RL_List*)calloc(1, sizeof(struct _RL_List));
}
/**
 *  Return a new list which can hold up to `nb_elements` items.
 */
RL_List *RL_list_allocate(int nb_elements) {
  RL_List *l = RL_list_new();
  l->array = (void **)calloc(sizeof(void *), nb_elements);
  l->allocSize = nb_elements;
  return l;
}
/**
 *  Return a new copy of `l`.
 */
RL_List *RL_list_duplicate(RL_List *l) {
  int i = 0;
  void **t;
  RL_List *ret = RL_list_allocate(l->allocSize);
  ret->fillSize = l->fillSize;
  for (t = RL_list_begin(l); t != RL_list_end(l); ++t) {
    ret->array[i++] = *t;
  }
  return ret;
}
/**
 *  Delete a list.
 *
 *  This only frees the list itself, if the list contains any pointers then
 *  those will need to be freed separately.
 */
void RL_list_delete(RL_List *l) {
  if (l) {
    free(l->array);
  }
  free(l);
}
/**
 *  Add `elt` to the end of a list.
 */
void RL_list_push(RL_List *l, const void *elt) {
  if (l->fillSize + 1 >= l->allocSize) { RL_list_allocate_int(l); }
  l->array[l->fillSize++] = (void *)elt;
}
/**
 *  Remove the last item from a list and return it.
 *
 *  If the list is empty this will return NULL.
 */
void * RL_list_pop(RL_List *l) {
  if (l->fillSize == 0) { return NULL; }
  return l->array[--(l->fillSize)];
}
/**
 *  Return the list item from the list, without removing it.
 *
 *  If the list is empty this will return NULL.
 */
void * RL_list_peek(RL_List *l) {
  if (l->fillSize == 0) { return NULL; }
  return l->array[l->fillSize - 1];
}
/**
 *  Add all items from `l2` to the end of `l`.
 */
void RL_list_add_all(RL_List *l, RL_List *l2) {
  void **curElt;
  for (curElt = RL_list_begin(l2); curElt != RL_list_end(l2); ++curElt) {
    RL_list_push(l, *curElt);
  }
}
/**
 *  Return the item at index `idx`.
 */
void * RL_list_get(RL_List *l,int idx) {
  return l->array[idx];
}
/**
 *  Set the item at `idx` to `elt`.
 *
 *  The list will automatically resize to fit and item at `idx`.
 */
void RL_list_set(RL_List *l,const void *elt, int idx) {
  if (idx < 0) { return; }
  while (l->allocSize < idx + 1) { RL_list_allocate_int(l); }
  l->array[idx] = (void *)elt;
  if (idx + 1 > l->fillSize) l->fillSize = idx + 1;
}
/**
 *  Return a pointer to the beginning of the list.
 */
void ** RL_list_begin(RL_List *l) {
  if (l->fillSize == 0) { return (void **)NULL; }
  return &l->array[0];
}
/**
 *  Return a pointer to the end of the list.
 */
void ** RL_list_end(RL_List *l) {
  if (l->fillSize == 0) { return (void **)NULL; }
  return &l->array[l->fillSize];
}
/**
 *  Reverse the order of the list.
 */
void RL_list_reverse(RL_List *l) {
  void **head = RL_list_begin(l);
  void **tail = RL_list_end(l) - 1;
  while (head < tail) {
    void *tmp = *head;
    *head = *tail;
    *tail = tmp;
    ++head;
    --tail;
  }
}
/**
 *  Remove an item from the list and return a new iterator.
 */
void **RL_list_remove_iterator(RL_List *l, void **elt) {
  void **curElt;
  for (curElt = elt; curElt < RL_list_end(l) - 1; ++curElt) {
    *curElt = *(curElt + 1);
  }
  l->fillSize--;
  if (l->fillSize == 0) {
    return ((void **)NULL) - 1;
  } else {
    return elt - 1;
  }
}
/**
 *  Remove an item from the list.
 */
void RL_list_remove(RL_List *l, const void *elt) {
  void **curElt;
  for (curElt = RL_list_begin(l); curElt != RL_list_end(l); ++curElt) {
    if (*curElt == elt) {
      RL_list_remove_iterator(l, curElt);
      return;
    }
  }
}
/**
 *  Remove an item from the list and return a new iterator.
 *
 *  This fast version replaces the removed item with the item at the end of the
 *  list.  This is faster but does not preserve the list order.
 */
void **RL_list_remove_iterator_fast(RL_List *l, void **elt) {
  *elt = l->array[l->fillSize-1];
  l->fillSize--;
  if (l->fillSize == 0) {
    return ((void **)NULL) - 1;
  } else {
    return elt - 1;
  }
}
/**
 *  Remove an item from the list, not preserving the list order.
 *
 *  The removed item is replaced with the item from the end of the list.
 *  This is faster but does not preserve the list order.
 */
void RL_list_remove_fast(RL_List *l, const void *elt) {
  void **curElt;
  for (curElt = RL_list_begin(l); curElt != RL_list_end(l); ++curElt) {
    if (*curElt == elt) {
      RL_list_remove_iterator_fast(l, curElt);
      return;
    }
  }
}
/**
 *  Return 1 if `elt` is in the list.
 */
int RL_list_contains(RL_List *l,const void *elt) {
  void **curElt;
  for (curElt = RL_list_begin(l); curElt != RL_list_end(l); ++curElt) {
    if (*curElt == elt) { return 1; }
  }
  return 0;
}
/**
 *  Remove ALL items from a list.
 */
void RL_list_clear(RL_List *l) {
  l->fillSize = 0;
}
/**
 *  Call free() on all items on the list, then remove them.
 */
void RL_list_clear_and_delete(RL_List *l) {
  void **curElt;
  for (curElt = RL_list_begin(l); curElt != RL_list_end(l); ++curElt) {
    free(*curElt);
  }
  l->fillSize = 0;
}
/**
 *  Return the current count of items in a list.
 */
int RL_list_size(RL_List *l) {
  return l->fillSize;
}
/**
 *  Insert `elt` on the index before `before`.
 */
void **RL_list_insert_before(RL_List *l,const void *elt,int before) {
  int idx;
  if (l->fillSize+1 >= l->allocSize) { RL_list_allocate_int(l); }
  for (idx = l->fillSize; idx > before; --idx) {
    l->array[idx] = l->array[idx - 1];
  }
  l->array[before] = (void *)elt;
  l->fillSize++;
  return &l->array[before];
}
/**
 *  Return 1 if this list is empty.
 */
int RL_list_is_empty(RL_List *l) {
  return (l->fillSize == 0);
}
