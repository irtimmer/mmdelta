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

#ifndef MISMATCH_H
#define MISMATCH_H

#define DIFF_TABLE_SIZE 63
#define MAX_MISMATCHES 4

struct mismatch_diff {
  char* old_data;
  char* new_data;
  char diff_data[MAX_MISMATCHES];
  unsigned int last_usage;
};

struct mismatch_diff diffs[MAX_MISMATCHES][DIFF_TABLE_SIZE];

int mismatch_find(char *old_data, char *new_data, unsigned int size, unsigned int *diff);
struct mismatch_diff *mismatch_add(char *old_data, char *new_data, unsigned int size, unsigned int last_usage);

#endif /* MISMATCH_H */
