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

#include "match.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <float.h>
#include <string.h>

#define MAX_MATCHES 100

#define SCORE_START 1
#define SCORE_MISMATCH 2

void match_init_table(struct match_table *map, char* data, unsigned int size) {
  map->num_blocks = map->num_buckets = size / BLOCKSIZE;
  map->buckets = calloc(map->num_buckets, sizeof(struct block_hash *));
  if (map->buckets == NULL) {
    fprintf(stderr, "Out of mermory\n");
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < map->num_buckets; i++) {
    map->buckets[i] = NULL;
  }

  map->hashes = calloc(map->num_blocks, sizeof(u_int32_t));
  if (map->hashes == NULL) {
    fprintf(stderr, "Out of mermory\n");
    exit(EXIT_FAILURE);
  }

  map->entries = calloc(map->num_blocks, sizeof(struct block_hash));
  if (map->entries == NULL) {
    fprintf(stderr, "Out of mermory\n");
    exit(EXIT_FAILURE);
  }

  map->size = size;
  map->data = data;
}

void match_add_hash(struct match_table *map, unsigned int position, u_int32_t value) {
  unsigned int bucket = value % map->num_buckets;
  map->entries[position].position = position;
  map->entries[position].next = map->buckets[bucket];
  map->buckets[bucket] = &map->entries[position];
}

void _match_free_mismatch_list(struct mismatch *entry) {
  struct mismatch *prev;
  while (entry != NULL) {
    prev = entry;
    entry = entry->next;
    free(prev);
  }
}

void match_free_list(struct match *entry) {
  struct match *prev;
  while (entry != NULL) {
    _match_free_mismatch_list(entry->mismatches_start);

    prev = entry;
    entry = entry->next;
    free(prev);
  }
}

void match_get_list(struct match_table *map, u_int32_t value, int checksize, char *data, struct match** first) {
  unsigned int bucket = value % map->num_buckets;

  struct block_hash *entry = map->buckets[bucket];
  int total_matches = 0;
  while (entry != NULL && total_matches < MAX_MATCHES) {
    if (value == map->hashes[entry->position]) {
      if ((entry->position + checksize / BLOCKSIZE) < map->num_blocks && memcmp(map->data + entry->position * BLOCKSIZE, data, checksize) == 0) {
        struct match *match = malloc(sizeof(struct match));
        if (match == NULL) {
          fprintf(stderr, "Out of mermory\n");
          exit(-1);
        }

        match->position = entry->position;
        match->length = checksize;
        match->mismatches = 0;
        match->consecutive_mismatches = 0;
        match->code_length = SCORE_START;
        match->mismatches_start = NULL;
        match->mismatches_end = NULL;
        match->next = *first;
        match->map = map;
        *first = match;

        total_matches++;
      }
    }

    entry = entry->next;
  }
}

void match_grow(struct match *entry, char *data, int size) {
  while (entry != NULL) {
    int start_length = entry->length;
    while (entry->consecutive_mismatches <= MAX_MISMATCHES && entry->position * BLOCKSIZE + entry->length + entry->consecutive_mismatches < entry->map->size && entry->length < (size - start_length)) {
      int old_position = entry->position * BLOCKSIZE + entry->length;
      if (entry->map->data[old_position + entry->consecutive_mismatches] == data[entry->length + entry->consecutive_mismatches]) {
        if (entry->consecutive_mismatches > 0) {
          struct mismatch *mismatch_list = malloc(sizeof(struct mismatch));
          if (mismatch_list == NULL) {
            fprintf(stderr, "Out of mermory\n");
            exit(EXIT_FAILURE);
          }

          mismatch_list->length = entry->consecutive_mismatches;
          mismatch_list->position = entry->length;
          mismatch_list->next = NULL;
          if (entry->mismatches_start == NULL)
            entry->mismatches_start = mismatch_list;
          else
            entry->mismatches_end->next = mismatch_list;

          entry->mismatches_end = mismatch_list;

          entry->code_length += SCORE_MISMATCH;
          entry->mismatches += entry->consecutive_mismatches;
          entry->length += entry->consecutive_mismatches;

          entry->consecutive_mismatches = 0;
        }
        entry->length++;
      } else {
        entry->consecutive_mismatches++;
      }
    }
    entry = entry->next;
  }
}

struct match *match_get_best(struct match *entry) {
  double score = DBL_MAX;
  struct match *best = NULL;
  while (entry != NULL) {
    double entry_score = (double)entry->code_length / (double)entry->length;
    if (entry_score < score) {
      score = entry_score;
      best = entry;
    }
    entry = entry->next;
  }

  return best;
}
