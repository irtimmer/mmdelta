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

#ifndef BUFFER_H
#define BUFFER_H

#include <stdbool.h>
#include <stdint.h>

#define MAX_BUFFER_SIZE 1024*1024*8

struct block_buffer {
  unsigned int offset;
  unsigned int length;
  char data[MAX_BUFFER_SIZE];
};

void buffer_write(struct block_buffer* buffer, void* data, int size);
void buffer_write_int(struct block_buffer* buffer, int data, int size);
void buffer_write_uleb128(struct block_buffer* buffer, int data);

uint64_t buffer_read_uleb128(struct block_buffer* buffer);

#endif /* BUFFER_H */
