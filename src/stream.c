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

static char in_buffer[4096];

void buffer_flush_all(struct delta_stream* stream) {
  char outbuf[4096];
  stream->lzma.next_out = outbuf;
  stream->lzma.avail_out = sizeof(outbuf);

  //Encode block sizes
  for (int i=0;i<NUM_BUFFERS;i++) {
    stream->lzma.next_in = &(stream->buffers[i].offset);
    stream->lzma.avail_in = sizeof(int);
    while (stream->lzma.avail_in > 0) {
      int ret = lzma_code(&stream->lzma, LZMA_RUN);
      if (stream->lzma.avail_out == 0) {
        write(stream->fd, outbuf, sizeof(outbuf));
        stream->lzma.next_out = outbuf;
        stream->lzma.avail_out = sizeof(outbuf);
      }
    }
  }

  //Encode block data
  for (int i=0;i<NUM_BUFFERS;i++) {
    stream->lzma.next_in = stream->buffers[i].data;
    stream->lzma.avail_in = stream->buffers[i].offset;
    while (stream->lzma.avail_in > 0) {
      int ret = lzma_code(&stream->lzma, LZMA_RUN);
      if (stream->lzma.avail_out == 0) {
        write(stream->fd, outbuf, sizeof(outbuf));
        stream->lzma.next_out = outbuf;
        stream->lzma.avail_out = sizeof(outbuf);
      }
    }

    stream->buffers[i].offset = 0;
  }

  while (lzma_code(&stream->lzma, LZMA_SYNC_FLUSH) != LZMA_STREAM_END) {
    if (stream->lzma.avail_out == 0) {
      write(stream->fd, outbuf, sizeof(outbuf));
      stream->lzma.next_out = outbuf;
      stream->lzma.avail_out = sizeof(outbuf);
    }
  }
  write(stream->fd, outbuf, sizeof(outbuf)-stream->lzma.avail_out);
}

void buffer_check_flush(struct delta_stream* stream) {
  for (int i=0;i<NUM_BUFFERS;i++) {
    if (stream->buffers[i].offset > MAX_BUFFER_SIZE - 8) {
      buffer_flush_all(stream);
      return;
    }
  }
}

bool buffer_read_all(struct delta_stream* stream) {
  //Decode block sizes
  for (int i=0;i<NUM_BUFFERS;i++) {
    stream->buffers[i].offset = 0;
    stream->lzma.next_out = &(stream->buffers[i].length);
    stream->lzma.avail_out = sizeof(int);
    while (stream->lzma.avail_out > 0) {
      if (stream->lzma.avail_in == 0) {
        stream->lzma.avail_in = read(stream->fd, in_buffer, sizeof(in_buffer));
        stream->lzma.next_in = in_buffer;
      }

      if (stream->lzma.avail_in <= 0)
        return false;

      int ret = lzma_code(&stream->lzma, LZMA_RUN);
    }
  }

  //Decode block data
  for (int i=0;i<NUM_BUFFERS;i++) {
    stream->lzma.next_out = stream->buffers[i].data;
    stream->lzma.avail_out = stream->buffers[i].length;
    while (stream->lzma.avail_out > 0) {
      if (stream->lzma.avail_in == 0) {
        stream->lzma.avail_in = read(stream->fd, in_buffer, sizeof(in_buffer));
        stream->lzma.next_in = in_buffer;
      }

      if (stream->lzma.avail_in <= 0)
        return false;

      int ret = lzma_code(&stream->lzma, LZMA_RUN);
    }
  }

  return true;
}
