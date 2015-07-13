/*
 * This file is part of MMDelta.
 *
 * Copyright (C) 2015 Iwan Timmer
 *
 * MMDelta is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MMDelta is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MMDelta; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MATCH_H
#define MATCH_H

#include "mismatch.h"

#include <sys/types.h>

#define BLOCKSIZE 16

struct block_hash {
  unsigned int position;
  struct block_hash* next;
};

struct mismatch {
  unsigned int position;
  unsigned int length;
  unsigned int diff;
  struct mismatch* next;
};

struct match {
  unsigned int position;
  unsigned int length;
  unsigned int code_length;
  unsigned int mismatches;
  unsigned int consecutive_mismatches;
  struct mismatch* mismatches_start;
  struct mismatch* mismatches_end;
  struct match* next;
};

extern u_int32_t* block_hashes;
extern int num_blocks;

void match_init_blocks(unsigned int size);

void match_add_hash(unsigned int position, u_int32_t hash);

struct match* match_get_list(u_int32_t hash, int checksize, char *old_data, char *new_data);
void match_free_list(struct match* entry);

void match_grow(struct match* entry, char* old_data, char* new_data, int size);
struct match* match_get_best(struct match* entry);

#endif /* MATCH_H */
