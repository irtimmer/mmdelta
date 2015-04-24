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

#ifndef BUFFER_H
#define BUFFER_H

#define MAX_BUFFER_SIZE 1024*1024*8
#define MAX_FULL_BUFFER_SIZE MAX_BUFFER_SIZE-100
#define NUM_BUFFERS 6

struct file_buffer {
  int fd;
  unsigned int offset;
  unsigned int length;
  char data[MAX_BUFFER_SIZE];
};

#define buffer_operations buffers[0]
#define buffer_addresses buffers[1]
#define buffer_lengths buffers[2]
#define buffer_data buffers[3]
#define buffer_diff_index buffers[4]
#define buffer_offsets buffers[5]
extern struct file_buffer buffers[NUM_BUFFERS];

void buffer_write(struct file_buffer* buffer, void* data, int size);
void buffer_write_int(struct file_buffer* buffer, int data, int size);
void buffer_write_uleb128(struct file_buffer* buffer, int data);
void buffer_check_flush();
void buffer_flush_all();

#endif /* BUFFER_H */
