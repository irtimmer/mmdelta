/*
 * This file is part of MMDelta.
 *
 * Copyright (C) 1996 Andrew Tridgell
 * Copyright (C) 1996 Paul Mackerras
 * Copyright (C) 2004-2009 Wayne Davison
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

#include <stdint.h>

#define CHAR_OFFSET 31

/*
 * a simple 32 bit checksum that can be upadted from either end
 * (inspired by Mark Adler's Adler-32 checksum)
 */
uint32_t adler32(char *buf, int32_t len) {
  int32_t i;
  uint32_t s1, s2;

  s1 = s2 = 0;
  for (i = 0; i < (len-4); i+=4) {
    s2 += 4*(s1 + buf[i]) + 3*buf[i+1] + 2*buf[i+2] + buf[i+3] + 10*CHAR_OFFSET;
    s1 += (buf[i+0] + buf[i+1] + buf[i+2] + buf[i+3] + 4*CHAR_OFFSET);
  }
  for (; i < len; i++) {
    s1 += (buf[i]+CHAR_OFFSET);
    s2 += s1;
  }
  return (s1 & 0xffff) + (s2 << 16);
}

uint32_t adler32_add(char in, char out, uint32_t checksum, int32_t len) {
  uint16_t s1 = checksum & 0xffff;
  uint16_t s2 = checksum >> 16;

  s1 += in - out;
  s2 += s1 - (len * (out + CHAR_OFFSET));

  return (s1 & 0xffff) + (s2 << 16);
}