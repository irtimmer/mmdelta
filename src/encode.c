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

#include "buffer.h"
#include "match.h"
#include "adler32.h"
#include "progress.h"
#include "utils.h"

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>

static lzma_filter filters[2];
static lzma_options_lzma options;

void _flush_add_bytes(unsigned int* current_add, char *data) {
  while (*current_add > 0) {
    buffer_check_flush();
    int to_write = *current_add;
    if (to_write + buffer_data.offset > MAX_BUFFER_SIZE) {
      to_write = MAX_BUFFER_SIZE - buffer_data.offset;
    }

    buffer_write(&buffer_operations, "A", 1);
    buffer_write(&buffer_data, data, to_write);
    buffer_write(&buffer_lengths, &to_write, sizeof(u_int32_t));
    *current_add -= to_write;
    data += to_write;
  }
}

void encode(const char* old_file, const char* new_file, const char* diff_file) {
  memset(&filters, 0, sizeof(filters));
  if (lzma_lzma_preset(&options, 9)) {
    fprintf(stderr, "Invalid options\n");
    exit(-1);
  }
  filters[0].id = LZMA_FILTER_LZMA2;
  filters[0].options = &options;
  filters[1].id = LZMA_VLI_UNKNOWN;
  lzma_stream_encoder(&stream, &filters[0], LZMA_CHECK_NONE);

  char *old_file_data;
  char *new_file_data;

  int old_file_size = mapfile(old_file, (void*) &old_file_data);
  if (old_file_size < 0)
    exit(EXIT_FAILURE);

  int new_file_size = mapfile(new_file, (void*) &new_file_data);
  if (new_file_size < 0)
    exit(EXIT_FAILURE);

  int fd = open(diff_file, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
  for (int i = 0; i < NUM_BUFFERS; i++)
    buffers[i].fd = fd;

  write(fd, "MMDELTA", 7);

  match_init_blocks(old_file_size);
  for (int i = 0; i < num_blocks; i++) {
    block_hashes[i] = adler32(old_file_data + i * BLOCKSIZE, BLOCKSIZE);
    match_add_hash(i, block_hashes[i]);
  }

  unsigned int current_pointer = 0;
  unsigned int current_add = 0;
  u_int32_t hash;

  struct progress_bar progress;
  progress_init(&progress, new_file_size);

  while (current_pointer < new_file_size) {
    progress_update(&progress, current_pointer);

    if (current_add>0 && current_pointer + BLOCKSIZE < new_file_size)
      hash = adler32_add(new_file_data[current_pointer+BLOCKSIZE-1], new_file_data[current_pointer-1], hash, BLOCKSIZE);
    else
      hash = adler32(new_file_data + current_pointer, current_pointer + BLOCKSIZE < new_file_size ? BLOCKSIZE : new_file_size - current_pointer);

    struct match *matches = match_get_list(hash, (current_pointer + BLOCKSIZE < new_file_size ? BLOCKSIZE : new_file_size - current_pointer), old_file_data, &new_file_data[current_pointer]);
    match_grow(matches, old_file_data, &new_file_data[current_pointer], new_file_size - current_pointer);

    struct match *match = match_get_best(matches);
    if (match != NULL) {
      _flush_add_bytes(&current_add, new_file_data + current_pointer - current_add);

      buffer_check_flush();
      buffer_write(&buffer_operations, "C", 1);
      buffer_write(&buffer_lengths, &(match->length), sizeof(u_int32_t));
      buffer_write(&buffer_addresses, &(match->position), sizeof(u_int32_t));

      struct mismatch *mismatches = match->mismatches_start;
      int mismatch_last_position = 0;
      while (mismatches != NULL) {
        buffer_check_flush();
        unsigned int diff;
        int find = mismatch_find(old_file_data + match->position * BLOCKSIZE + mismatches->position, new_file_data + current_pointer + mismatches->position, mismatches->length, &diff);
        u_int8_t index = (mismatches->length - 1);

        struct mismatch_diff *current_diff;
        switch (find) {
        case -1:
          current_diff = mismatch_add_enc(&old_file_data[match->position * BLOCKSIZE + mismatches->position], &new_file_data[current_pointer + mismatches->position], mismatches->length, current_pointer + mismatches->position);
          buffer_write(&buffer_operations, "E", 1);
          buffer_write(&buffer_data, current_diff->diff_data, mismatches->length);
          break;
        case 1:
          index += (diff + 1) << 2;
        case 2:
          buffer_write(&buffer_operations, "F", 1);
          diffs[mismatches->length - 1][diff].last_usage = current_pointer + mismatches->position;
        }

        buffer_write_uleb128(&buffer_offsets, mismatches->position - mismatch_last_position);
        mismatch_last_position = mismatches->position;

        buffer_write(&buffer_diff_index, &(index), sizeof(u_int8_t));

        mismatches = mismatches->next;
      }

      current_pointer += match->length;
    } else {
      current_pointer++;
      current_add++;
    }
    match_free_list(matches);
  }

  _flush_add_bytes(&current_add, new_file_data + current_pointer - current_add);
  buffer_flush_all();
}