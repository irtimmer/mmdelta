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

#ifndef STREAM_H
#define STREAM_H

#include "buffer.h"

#include <lzma.h>

#define NUM_BUFFERS 6

#define buffer_operations(stream) stream->buffers[0]
#define buffer_addresses(stream) stream->buffers[1]
#define buffer_lengths(stream) stream->buffers[2]
#define buffer_data(stream) stream->buffers[3]
#define buffer_diff_index(stream) stream->buffers[4]
#define buffer_offsets(stream) stream->buffers[5]

struct delta_stream {
  int fd;
  lzma_stream lzma;
  struct block_buffer buffers[NUM_BUFFERS];
};

void stream_check_flush(struct delta_stream* stream);
void stream_flush(struct delta_stream* stream);

bool stream_read(struct delta_stream* st);

#endif /* STREAM_H */
