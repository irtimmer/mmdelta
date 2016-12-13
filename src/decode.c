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

#include "stream.h"
#include "utils.h"
#include "match.h"
#include "mismatch.h"

#include <sys/mman.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

void decode(const char* old_file, const char* diff_file, const char* target_file) {
  char *old_file_data;

  int old_file_size = mapfile(old_file, 0, (void*) &old_file_data);
  if (old_file_size < 0)
    exit(EXIT_FAILURE);

  struct delta_stream* stream = malloc(sizeof(struct delta_stream));
  stream->fd = open(diff_file, O_RDONLY);
  lseek(stream->fd, 7, SEEK_CUR); //Skip MMDELTA

  int new_file_size;
  char *new_file_data;
  read(stream->fd, &new_file_size, sizeof(int));
  if (mapfile(target_file, new_file_size, (void*) &new_file_data) < 0)
    exit(EXIT_FAILURE);

  lzma_stream_decoder(&stream->lzma, UINT64_MAX, LZMA_TELL_NO_CHECK);

  char* buffer;
  u_int32_t buffer_length = 0, buffer_offset, written = 0;
  u_int32_t last_length = 0;
  while (buffer_read_all(stream)) {
    for (int i=0;i<buffer_operations(stream).length;i++) {
      u_int32_t length;
      u_int8_t index;
      switch (buffer_operations(stream).data[i]) {
      case 'A':
      case 'C':
          if (buffer_length > 0) {
              memcpy(new_file_data + written, buffer + buffer_offset, buffer_length);
              written += buffer_length;
              buffer_length = 0;
          }
      }

      switch (buffer_operations(stream).data[i]) {
      case 'A':
        buffer_read(&buffer_lengths(stream), length);

        memcpy(new_file_data + written, buffer_data(stream).data + buffer_data(stream).offset, length);
        buffer_data(stream).offset += length;
        written += length;
        break;
      case 'C':
        buffer_read(&buffer_lengths(stream), buffer_length);
        buffer_read(&buffer_addresses(stream), buffer_offset);

        if (buffer_offset >= old_file_size / BLOCKSIZE) {
          buffer_offset -= old_file_size / BLOCKSIZE;
          buffer = new_file_data;
        } else
          buffer = old_file_data;

        buffer_offset *= BLOCKSIZE;
        last_length = 0;
        break;
      case 'E':
        buffer_read(&buffer_diff_index(stream), index);
        length = index+1;

        int position = buffer_read_uleb128(&buffer_offsets(stream)) - last_length;

        memcpy(new_file_data + written, buffer + buffer_offset, position);
        buffer_length -= position;
        buffer_offset += position;
        written += position;

        struct mismatch_diff *diff = mismatch_add_dec(buffer + buffer_offset, buffer_data(stream).data + buffer_data(stream).offset, length, written);
        buffer_data(stream).offset += length;

        memcpy(new_file_data + written, diff->new_data, length);
        buffer_length -= length;
        buffer_offset += length;
        written += length;
        last_length = length;
        break;
      case 'F':
        buffer_read(&buffer_diff_index(stream), index);
        length = (index & 0x3) + 1;
        index >>= 2;

        position = buffer_read_uleb128(&buffer_offsets(stream)) - last_length;

        memcpy(new_file_data + written, buffer + buffer_offset, position);
        written += position;
        buffer_length -= position;
        buffer_offset += position;

        if (index > 0) {
          diffs[length-1][index-1].last_usage = written;
          char mismatch_buffer[MAX_MISMATCHES];
          for (int i=0;i<length;i++) {
            mismatch_buffer[i] = buffer[buffer_offset+i] ^ diffs[length-1][index-1].diff_data[i];
          }
          memcpy(new_file_data + written, mismatch_buffer, length);
        } else {
          struct mismatch_diff* diff = mismatch_find_data(buffer + buffer_offset, length);
          if (diff == NULL) {
            printf("Mismatch diff can't be found\n");
            exit(EXIT_FAILURE);
          }
          diff->last_usage = written;
          memcpy(new_file_data + written, diff->new_data, length);
        }

        buffer_length -= length;
        buffer_offset += length;
        last_length = length;
        written += length;
        break;
      default:
        fprintf(stderr, "Unsupported action '%c'\n", buffer_operations(stream).data[i]);
        exit(EXIT_FAILURE);
      }
    }
  }
  if (buffer_length > 0) {
    memcpy(new_file_data + written, buffer + buffer_offset, buffer_length);
  }
}
