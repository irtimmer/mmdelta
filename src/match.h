/*
 * This file is part of MMDelta.
 *
 * Copyright (C) 2015, 2016 Iwan Timmer
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

struct match_table {
  int size;
  char* data;

  u_int32_t *hashes;
  struct block_hash **buckets;
  struct block_hash *entries;

  int num_blocks;
  int num_buckets;
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
  struct match_table* map;
  struct mismatch* mismatches_start;
  struct mismatch* mismatches_end;
  struct match* next;
};

extern u_int32_t* block_hashes;
extern int num_blocks;

void match_init_blocks(struct match_table *map, char* data, unsigned int size);

void match_add_hash(struct match_table *map, unsigned int position, u_int32_t hash);

void match_get_list(struct match_table *map, u_int32_t hash, int checksize, char *data, struct match** first);
void match_free_list(struct match* entry);

void match_grow(struct match* entry, char* data, int size);
struct match* match_get_best(struct match* entry);

#endif /* MATCH_H */
