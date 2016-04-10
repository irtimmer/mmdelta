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

struct block_buffer buffers[NUM_BUFFERS];
lzma_stream stream;

static bool flush_event;
static char in_buffer[4096];

void buffer_write(struct block_buffer* buffer, void* data, int size) {
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

void buffer_write_int(struct block_buffer* buffer, int data, int size) {
  buffer_write(buffer, &data, size);
}

void buffer_write_uleb128 (struct block_buffer* buffer, int data) {
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
  char outbuf[4096];
  stream.next_out = outbuf;
  stream.avail_out = sizeof(outbuf);

  //Encode block sizes
  for (int i=0;i<NUM_BUFFERS;i++) {
    stream.next_in = &(buffers[i].offset);
    stream.avail_in = sizeof(int);
    while (stream.avail_in > 0) {
      int ret = lzma_code(&stream, LZMA_RUN);
      if (stream.avail_out == 0) {
        write(buffers[i].fd, outbuf, sizeof(outbuf));
        stream.next_out = outbuf;
        stream.avail_out = sizeof(outbuf);
      }
    }
  }

  //Encode block data
  for (int i=0;i<NUM_BUFFERS;i++) {
    stream.next_in = buffers[i].data;
    stream.avail_in = buffers[i].offset;
    while (stream.avail_in > 0) {
      int ret = lzma_code(&stream, LZMA_RUN);
      if (stream.avail_out == 0) {
        write(buffers[i].fd, outbuf, sizeof(outbuf));
        stream.next_out = outbuf;
        stream.avail_out = sizeof(outbuf);
      }
    }

    buffers[i].offset = 0;
  }

  while (lzma_code(&stream, LZMA_SYNC_FLUSH) != LZMA_STREAM_END) {
    if (stream.avail_out == 0) {
      write(buffers[0].fd, outbuf, sizeof(outbuf));
      stream.next_out = outbuf;
      stream.avail_out = sizeof(outbuf);
    }
  }
  write(buffers[0].fd, outbuf, sizeof(outbuf)-stream.avail_out);
}

void buffer_check_flush() {
    if (flush_event) {
        buffer_flush_all();
        flush_event = false;
    }
}

bool buffer_read_all() {
  //Decode block sizes
  for (int i=0;i<NUM_BUFFERS;i++) {
    buffers[i].offset = 0;
    stream.next_out = &(buffers[i].length);
    stream.avail_out = sizeof(int);
    while (stream.avail_out > 0) {
      if (stream.avail_in == 0) {
        stream.avail_in = read(buffers[i].fd, in_buffer, sizeof(in_buffer));
        stream.next_in = in_buffer;
      }

      if (stream.avail_in <= 0)
        return false;

      int ret = lzma_code(&stream, LZMA_RUN);
    }
  }

  //Decode block data
  for (int i=0;i<NUM_BUFFERS;i++) {
    stream.next_out = buffers[i].data;
    stream.avail_out = buffers[i].length;
    while (stream.avail_out > 0) {
      if (stream.avail_in == 0) {
        stream.avail_in = read(buffers[i].fd, in_buffer, sizeof(in_buffer));
        stream.next_in = in_buffer;
      }

      if (stream.avail_in <= 0)
        return false;

      int ret = lzma_code(&stream, LZMA_RUN);
    }
  }

  return true;
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
