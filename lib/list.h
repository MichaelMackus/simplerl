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

#ifndef RL_LIST_H
#define RL_LIST_H

struct _RL_List;
typedef struct _RL_List RL_List;

RL_List *RL_list_new(void);
RL_List *RL_list_allocate(int nb_elements);
RL_List *RL_list_duplicate(RL_List *l);
void RL_list_delete(RL_List *l);
void RL_list_push(RL_List *l, const void * elt);
void *RL_list_pop(RL_List *l);
void *RL_list_peek(RL_List *l);
void RL_list_add_all(RL_List *l, RL_List *l2);
void *RL_list_get(RL_List *l,int idx);
void RL_list_set(RL_List *l,const void *elt, int idx);
void **RL_list_begin(RL_List *l);
void **RL_list_end(RL_List *l);
void RL_list_reverse(RL_List *l);
void **RL_list_remove_iterator(RL_List *l, void **elt);
void RL_list_remove(RL_List *l, const void * elt);
void **RL_list_remove_iterator_fast(RL_List *l, void **elt);
void RL_list_remove_fast(RL_List *l, const void * elt);
int RL_list_contains(RL_List *l,const void * elt);
void RL_list_clear(RL_List *l);
void RL_list_clear_and_delete(RL_List *l);
int RL_list_size(RL_List *l);
void ** RL_list_insert_before(RL_List *l,const void *elt,int before);
int RL_list_is_empty(RL_List *l);

#endif
