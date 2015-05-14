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

#include "progress.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void progress_init(struct progress_bar* progress, unsigned int length) {
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

  progress->width = w.ws_col-7;
  progress->percentage = -1;
  progress->length = length;
}

void progress_update(struct progress_bar* progress, unsigned int length) {
  unsigned int percentage = length / (progress->length/progress->width);
  if (percentage != progress->percentage) {
    printf("\r[");
    for (int i=0;i<progress->width;i++) {
      printf(i<percentage?"=":i==percentage?">":" ");
    }

    printf("] %3d%%", length / (progress->length/100));
    fflush(stdout);
    progress->percentage = percentage;
  }
}
