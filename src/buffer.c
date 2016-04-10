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

#include "buffer.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

void buffer_write_data(struct block_buffer* buffer, void* data, int size) {
  if (size+buffer->offset >MAX_BUFFER_SIZE) {
    fprintf(stderr, "Buffer not big enough (%d/%d)\n", size, MAX_BUFFER_SIZE - buffer->offset);
    exit(EXIT_FAILURE);
  }
  
  memcpy(buffer->data + buffer->offset, data, size);
  buffer->offset += size;
  buffer->length += size;
}

void buffer_read_data(struct block_buffer* buffer, void* data, int size) {
  memcpy(data, &(buffer->data[buffer->offset]), size);
  buffer->offset += size;
}

void buffer_write_uleb128 (struct block_buffer* buffer, int data) {
    do {
        buffer->data[buffer->offset] = data & 0x7fU;
        if (data >>= 7)
            buffer->data[buffer->offset] |= 0x80U;

        buffer->offset++;
    } while (data);
}

uint64_t buffer_read_uleb128(struct block_buffer* buffer) {
  uint64_t x = 0;
  int bytes = 0;
  do {
    if (bytes == 0) x = 0;
    x |= (buffer->data[buffer->offset] & 0x7fULL) << (7 * bytes++);
    if (!(buffer->data[buffer->offset] & 0x80U)) break;
  } while (buffer->offset++ < buffer->length);
  buffer->offset++;
  return x;
}
