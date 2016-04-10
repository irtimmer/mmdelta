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

void buffer_write_uleb128(struct block_buffer* buffer, int data);
void buffer_write_data(struct block_buffer* buffer, void* data, int size);
#define buffer_write(buffer, data) buffer_write_data(buffer, &data, sizeof(data));

uint64_t buffer_read_uleb128(struct block_buffer* buffer);
void buffer_read_data(struct block_buffer* buffer, void* data, int size);
#define buffer_read(buffer, data) buffer_read_data(buffer, &data, sizeof(data));

#endif /* BUFFER_H */
