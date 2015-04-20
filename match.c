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

#include "match.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <limits.h>

u_int32_t *block_hashes;
struct block_hash **block_hash_buckets;
struct block_hash *block_hash_entries;

int num_blocks;
int num_buckets;

void match_init_blocks(unsigned int size) {
  num_blocks = num_buckets = size / BLOCKSIZE;
  block_hash_buckets = calloc(num_buckets, sizeof(struct block_hash *));
  if (block_hash_buckets == NULL) {
    fprintf(stderr, "Out of mermory\n");
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < num_buckets; i++) {
    block_hash_buckets[i] = NULL;
  }

  block_hashes = calloc(num_blocks, sizeof(u_int32_t));
  if (block_hashes == NULL) {
    fprintf(stderr, "Out of mermory\n");
    exit(EXIT_FAILURE);
  }

  block_hash_entries = calloc(num_blocks, sizeof(struct block_hash));
  if (block_hash_entries == NULL) {
    fprintf(stderr, "Out of mermory\n");
    exit(EXIT_FAILURE);
  }
}

void match_add_hash(unsigned int position, u_int32_t value) {
  unsigned int bucket = value % num_buckets;
  block_hash_entries[position].position = position;
  block_hash_entries[position].next = block_hash_buckets[bucket];
  block_hash_buckets[bucket] = &block_hash_entries[position];
}

void match_free_list(struct match *entry) {
  struct match *prev;
  while (entry != NULL) {
    prev = entry;
    entry = entry->next;
    free(prev);
  }
}

struct match *match_get_list(u_int32_t value) {
  unsigned int bucket = value % num_buckets;
  struct match *first = NULL;

  struct block_hash *entry = block_hash_buckets[bucket];
  while (entry != NULL) {
    if (value == block_hashes[entry->position]) {
      struct match *match = malloc(sizeof(struct match));
      if (match == NULL) {
        fprintf(stderr, "Out of mermory\n");
        exit(-1);
      }

      match->position = entry->position;
      match->length = 0;
      match->next = first;
      first = match;
    }

    entry = entry->next;
  }

  return first;
}

int match_check(struct match **prev, char *old_data, char *new_data, int size, int checksize) {
  if (checksize > size)
    checksize = size;

  if (*prev == NULL)
    return -1;

  struct match *entry = *prev;
  struct match **first = prev;
  unsigned int total_matches = 0;
  unsigned int data_position = 0;
  while (entry != NULL) {
    while (entry->length < checksize && (entry->position + entry->length / BLOCKSIZE) < num_blocks) {
      if (old_data[entry->position * BLOCKSIZE + entry->length] != new_data[entry->length])
        break;

      entry->length++;
    }

    if (entry->length < checksize) {
      *prev = entry->next;
      free(entry);
    } else {
      total_matches++;
      prev = &(entry->next);
      data_position += entry->position;
    }

    entry = *prev;
  }

  return total_matches;
}

void match_grow(struct match *entry, char *old_data, char *new_data, int size) {
  while (entry != NULL) {
    int start_length = entry->length;
    while (entry->length < (size - start_length) && old_data[entry->position * BLOCKSIZE + entry->length] == new_data[entry->length]) {
      entry->length++;
    }
    entry = entry->next;
  }
}

struct match *match_get_best(struct match *entry) {
  unsigned int score = UINT_MAX;
  struct match *best = NULL;
  while (entry != NULL) {
    if (entry->length > score) {
      score = entry->length;
      best = entry;
    }
    entry = entry->next;
  }

  return best;
}
