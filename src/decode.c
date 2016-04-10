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

  int old_file_size = mapfile(old_file, (void*) &old_file_data);
  if (old_file_size < 0)
    exit(EXIT_FAILURE);

  struct delta_stream* stream = malloc(sizeof(struct delta_stream));
  stream->fd = open(diff_file, O_RDONLY);
  lseek(stream->fd, 7, SEEK_CUR); //Skip MMDELTA
  lzma_stream_decoder(&stream->lzma, UINT64_MAX, LZMA_TELL_NO_CHECK);

  int fd = open(target_file, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

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
              write(fd, &old_file_data[buffer_offset], buffer_length);
              written += buffer_length;
              buffer_length = 0;
          }
      }

      switch (buffer_operations(stream).data[i]) {
      case 'A':
        memcpy(&length, &(buffer_lengths(stream).data[buffer_lengths(stream).offset]), sizeof(u_int32_t));
        buffer_lengths(stream).offset += sizeof(u_int32_t);
        write(fd, &(buffer_data(stream).data[buffer_data(stream).offset]), length);
        written += length;
        buffer_data(stream).offset += length;
        break;
      case 'C':
        memcpy(&buffer_length, &(buffer_lengths(stream).data[buffer_lengths(stream).offset]), sizeof(u_int32_t));
        buffer_lengths(stream).offset += sizeof(u_int32_t);

        memcpy(&buffer_offset, &(buffer_addresses(stream).data[buffer_addresses(stream).offset]), sizeof(u_int32_t));
        buffer_addresses(stream).offset += sizeof(u_int32_t);

        buffer_offset *= BLOCKSIZE;
        last_length = 0;
        break;
      case 'E':
        memcpy(&index, &(buffer_diff_index(stream).data[buffer_diff_index(stream).offset]), sizeof(u_int8_t));
        buffer_diff_index(stream).offset += sizeof(u_int8_t);
        length = index+1;

        int position = buffer_read_uleb128(&buffer_offsets(stream)) - last_length;

        write(fd, &old_file_data[buffer_offset], position);
        buffer_length -= position;
        buffer_offset += position;
        written += position;

        struct mismatch_diff *diff = mismatch_add_dec(&old_file_data[buffer_offset], buffer_data(stream).data + buffer_data(stream).offset, length, written);
        buffer_data(stream).offset += length;

        write(fd, diff->new_data, length);
        buffer_length -= length;
        buffer_offset += length;
        written += length;
        last_length = length;
        break;
      case 'F':
        memcpy(&index, &(buffer_diff_index(stream).data[buffer_diff_index(stream).offset]), sizeof(u_int8_t));
        buffer_diff_index(stream).offset += sizeof(u_int8_t);
        length = (index & 0x3) + 1;
        index >>= 2;

        position = buffer_read_uleb128(&buffer_offsets(stream)) - last_length;

        write(fd, &old_file_data[buffer_offset], position);
        written += position;
        buffer_length -= position;
        buffer_offset += position;

        if (index > 0) {
          diffs[length-1][index-1].last_usage = written;
          char mismatch_buffer[MAX_MISMATCHES];
          for (int i=0;i<length;i++) {
            mismatch_buffer[i] = old_file_data[buffer_offset+i] ^ diffs[length-1][index-1].diff_data[i];
          }
          write(fd, mismatch_buffer, length);
        } else {
          struct mismatch_diff* diff = mismatch_find_data(&old_file_data[buffer_offset], length);
          if (diff == NULL) {
            printf("Mismatch diff can't be found\n");
            exit(EXIT_FAILURE);
          }
          diff->last_usage = written;
          write(fd, diff->new_data, length);
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
      write(fd, &old_file_data[buffer_offset], buffer_length);
  }
}
