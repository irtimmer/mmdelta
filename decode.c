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
#include "utils.h"
#include "match.h"

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

  int infd = open(diff_file, O_RDONLY);
  lseek(infd, 7, SEEK_CUR); //Skip MMDELTA
  for (int i = 0; i < NUM_BUFFERS; i++)
    buffers[i].fd = infd;

  int outfd = open(target_file, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

  while (buffer_read_all()) {
    for (int i=0;i<buffer_operations.length;i++) {
      u_int32_t length;
      switch (buffer_operations.data[i]) {
      case 'A':
        memcpy(&length, &(buffer_lengths.data[buffer_lengths.offset]), sizeof(u_int32_t));
        buffer_lengths.offset += sizeof(u_int32_t);
        write(outfd, &(buffer_data.data[buffer_data.offset]), length);
        buffer_data.offset += length;
        break;
      case 'C':
        memcpy(&length, &(buffer_lengths.data[buffer_lengths.offset]), sizeof(u_int32_t));
        buffer_lengths.offset += sizeof(u_int32_t);

        u_int32_t offset;
        memcpy(&offset, &(buffer_addresses.data[buffer_addresses.offset]), sizeof(u_int32_t));
        buffer_addresses.offset += sizeof(u_int32_t);

        write(outfd, &old_file_data[offset*BLOCKSIZE], length);
        break;
      case 'E':
      case 'F':
        fprintf(stderr, "Mismatches unsupported\n");
        exit(EXIT_FAILURE);
      default:
        fprintf(stderr, "Unsupported action '%c'\n", buffer_operations.data[i]);
        exit(EXIT_FAILURE);
      }
    }
  }
}
