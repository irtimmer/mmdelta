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
#include <stdbool.h>

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
