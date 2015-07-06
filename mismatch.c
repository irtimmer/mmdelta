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

#include "mismatch.h"

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>

struct mismatch_diff diffs[MAX_MISMATCHES][DIFF_TABLE_SIZE];

int mismatch_find(char *old_data, char *new_data, unsigned int size, unsigned int *diff) {
  char diff_data[size];
  for (int i = 0; i < size; i++)
    diff_data[i] = (char)(old_data[i] ^ new_data[i]);

  struct mismatch_diff* sdiff = diffs[size - 1];
  bool collision = false;
  for (int i = 0; i < DIFF_TABLE_SIZE; i++) {
    struct mismatch_diff* cdiff = &sdiff[i];
    if (cdiff->old_data != NULL) {
      if (memcmp(diff_data, cdiff->diff_data, size) == 0) {
        if (!collision && memcmp(new_data, cdiff->new_data, size) == 0) {
          *diff = i;
          return 2;
        } else {
          *diff = i;
          return 1;
        }
      } else if (memcmp(old_data, cdiff->old_data, size) == 0) {
        collision = true;
      }
    }
  }

  return -1;
}

struct mismatch_diff *mismatch_add_enc(char *old_data, char *new_data, unsigned int size, unsigned int last_usage) {
  unsigned int oldest = 0;
  unsigned int oldest_usage = UINT_MAX;
  for (int i = 0; i < DIFF_TABLE_SIZE; i++) {
    if (diffs[size - 1][i].last_usage < oldest_usage) {
      oldest_usage = diffs[size - 1][i].last_usage;
      oldest = i;
    }
  }

  for (int i = 0; i < size; i++)
    diffs[size - 1][oldest].diff_data[i] = (char)(old_data[i] ^ new_data[i]);

  diffs[size - 1][oldest].last_usage = last_usage;
  diffs[size - 1][oldest].old_data = old_data;
  diffs[size - 1][oldest].new_data = new_data;

  return &(diffs[size - 1][oldest]);
}

struct mismatch_diff *mismatch_add_dec(char *old_data, char *diff_data, unsigned int size, unsigned int last_usage) {
  unsigned int oldest = 0;
  unsigned int oldest_usage = UINT_MAX;
  for (int i = 0; i < DIFF_TABLE_SIZE; i++) {
    if (diffs[size - 1][i].last_usage < oldest_usage) {
      oldest_usage = diffs[size - 1][i].last_usage;
      oldest = i;
    }
  }

  if (diffs[size - 1][oldest].old_data == NULL) {
      char *olddata_cache = malloc(size);
      if (olddata_cache == NULL) {
          fprintf(stderr, "Out of mermory, allocating %d bytes\n", size);
          exit(-1);
      }

      char *newdata_cache = malloc(size);
      if (newdata_cache == NULL) {
          fprintf(stderr, "Out of mermory, allocating %d bytes\n", size);
          exit(-1);
      }

      diffs[size - 1][oldest].old_data = olddata_cache;
      diffs[size - 1][oldest].new_data = newdata_cache;
  }

  memcpy(diffs[size - 1][oldest].old_data, old_data, size);

  diffs[size - 1][oldest].last_usage = last_usage;

  for (int i = 0; i < size; i++) {
    diffs[size - 1][oldest].diff_data[i] = diff_data[i];
    diffs[size - 1][oldest].new_data[i] = (char)(old_data[i] ^ diff_data[i]);
  }

  return &(diffs[size - 1][oldest]);
}
