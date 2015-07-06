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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

struct file_buffer buffers[NUM_BUFFERS];

static bool flush_event;

void buffer_write(struct file_buffer* buffer, void* data, int size) {
  if (size+buffer->offset >MAX_BUFFER_SIZE) {
    fprintf(stderr, "Buffer not big enough (%d/%d)\n", size, MAX_BUFFER_SIZE - buffer->offset);
    exit(EXIT_FAILURE);
  }
  
  memcpy(buffer->data + buffer->offset, data, size);
  buffer->offset += size;
  buffer->length += size;
  if (buffer->offset > MAX_FULL_BUFFER_SIZE) {
    flush_event = true;
  }
}

void buffer_write_int(struct file_buffer* buffer, int data, int size) {
  buffer_write(buffer, &data, size);
}

void buffer_write_uleb128 (struct file_buffer* buffer, int data) {
    do {
        buffer->data[buffer->offset] = data & 0x7fU;
        if (data >>= 7)
            buffer->data[buffer->offset] |= 0x80U;

        buffer->offset++;
    } while (data);

    if (buffer->offset > MAX_FULL_BUFFER_SIZE) {
      flush_event = true;
    }
}

void buffer_flush_all() {
  for (int i=0;i<NUM_BUFFERS;i++) {
    write(buffers[i].fd, &(buffers[i].offset), sizeof(int));
  }

  for (int i=0;i<NUM_BUFFERS;i++) {
    write(buffers[i].fd, buffers[i].data, buffers[i].offset);
    buffers[i].offset = 0;
  }
}

void buffer_check_flush() {
    if (flush_event) {
        buffer_flush_all();
        flush_event = false;
    }
}

bool buffer_read_all() {
  for (int i=0;i<NUM_BUFFERS;i++) {
    int len = read(buffers[i].fd, &(buffers[i].length), sizeof(int));
    if (len != sizeof(int))
      return false;
  }
  for (int i=0;i<NUM_BUFFERS;i++) {
    int len = read(buffers[i].fd, buffers[i].data, buffers[i].length);
    buffers[i].offset = 0;
    if (len != buffers[i].length)
      return false;
  }
  return true;
}

uint64_t buffer_read_uleb128(struct file_buffer* buffer) {
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
