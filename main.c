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

#include "encode.h"
#include "decode.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define ACTION_ENCODE 1
#define ACTION_DECODE 2

int main(int argc, char* const argv[]) {
  char c;
  char* sourceFile = NULL;
  char* diffFile = NULL;
  char* targetFile = NULL;
  int action = 0;
  
  while ((c = getopt(argc, argv, "-ed")) != -1) {
    switch (c) {
    case 'e':
      action = ACTION_ENCODE;
      break;
    case 'd':
      action = ACTION_DECODE;
      break;
    default:
      if (sourceFile == NULL)
        sourceFile = optarg;
      else if (diffFile == NULL)
        diffFile = optarg;
      else if (targetFile == NULL)
        targetFile = optarg;
      else {
        fprintf(stderr, "Invalid option '%s'\n", optarg);
        exit(-1);
      }
    }
  }
  
  switch (action) {
  case ACTION_ENCODE:
    if (targetFile == NULL) {
      fprintf(stderr, "Specify a source, target and diff file\n");
      exit(EXIT_FAILURE);
    }
    encode(sourceFile, diffFile, targetFile);
    break;
  case ACTION_DECODE:
    if (targetFile == NULL) {
      fprintf(stderr, "Specify a source, diff and target file\n");
      exit(EXIT_FAILURE);
    }
    decode(sourceFile, diffFile, targetFile);
    break;
  default:
    fprintf(stderr, "Specify an action\n");
    exit(EXIT_FAILURE);
  }
}
