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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int mapfile(const char *filename, int size, void **filedata) {
  struct stat sb;
  int fd;

  if (size == 0) {
    fd = open(filename, O_RDONLY);
    if (fstat(fd, &sb) == -1) {
      fprintf(stderr, "Can't find file %s\n", filename);
      return -1;
    }
    if (!S_ISREG(sb.st_mode)) {
      fprintf(stderr, "%s is not a regular file\n", filename);
      return -1;
    }
    *filedata = mmap(0, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
  } else {
    fd = open(filename, O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    ftruncate(fd, size);
    *filedata = mmap(0, size, PROT_WRITE, MAP_SHARED, fd, 0);
  }

  if (*filedata == MAP_FAILED) {
    fprintf(stderr, "Can't map %s to memory\n", filename);
    return -1;
  }

  if (close(fd) == -1) {
    fprintf(stderr, "Can't close file %s\n", filename);
    return -1;
  }

  return size > 0 ? size : sb.st_size;
}
