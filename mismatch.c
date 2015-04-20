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
#include <limits.h>
#include <string.h>

struct mismatch_diff diffs[MAX_MISMATCHES][DIFF_TABLE_SIZE];

int mismatch_find(char *old_data, char *new_data, unsigned int size, unsigned int *diff) {
  for (int i = 0; i < DIFF_TABLE_SIZE; i++) {
    if (diffs[size - 1][i].old_data != NULL && memcmp(old_data, diffs[size - 1][i].old_data, size) == 0) {
      if (memcmp(new_data, diffs[size - 1][i].new_data, size) == 0) {
        *diff = i;
        return 2;
      } else {
        break;
      }
    }
  }

  char diff_data[size];
  for (int i = 0; i < size; i++)
    diff_data[i] = (char)(old_data[i] ^ new_data[i]);

  for (int i = 0; i < DIFF_TABLE_SIZE; i++) {
    if (diffs[size - 1][i].old_data != NULL && memcmp(diff_data, diffs[size - 1][i].diff_data, size) == 0) {
      *diff = i;
      return 1;
    }
  }

  return -1;
}

struct mismatch_diff *mismatch_add(char *old_data, char *new_data, unsigned int size, unsigned int last_usage) {
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
