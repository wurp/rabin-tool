/* 
 * Copyright (C) 2010 Bobby Martin (bobbymartin2@gmail.com)
 * Copyright (C) 2004 Hyang-Ah Kim (hakim@cs.cmu.edu)
 * Copyright (C) 2000 David Mazieres (dm@uun.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "rabinpoly.h"

#define INT64(n) n##LL
#define MSB64 INT64(0x8000000000000000)
#define FINGERPRINT_PT 0xbfe6b8a5bf378d83LL	

void printChunkData(
    char* msgPrefix,
    int size,
    u_int64_t fingerprint,
    u_int64_t hash)
{
  printf("%s chunk hash: %16llx fingerprint: %16llx length: %d\n",
         msgPrefix,
         hash,
         fingerprint,
         size);
}

u_int64_t makeBitMask(int maskSize)
{
  u_int64_t retval = 0;

  u_int64_t currBit = 1;
  for(int i = 0; i < maskSize; ++i)
  {
    retval |= currBit;
    currBit <<= 1;
  }

  return retval;
}

  int main(int argc, char **argv)
  {
    if( argc < 2 )
    {
      printf("Usage: rabin <file name>\n");
      exit(-1);
    }

    const u_int64_t POLY = FINGERPRINT_PT;
    window rw(POLY);
    rabinpoly rp(POLY);

    FILE* is = fopen(argv[1], "r");

    if( is == 0 )
    {
       printf("Could not open %s\n", argv[1]);
       exit(-2);
    }

    int next;
    int size = 0;
    u_int64_t val = 0;
    u_int64_t hash = 0;

    //break (on average) every 2^BITS bytes
    const int BITS = 12;
    u_int64_t chunkBoundaryBitMask = makeBitMask(BITS);


    const int MAX_SIZE = 4 * (1 << BITS);
    const int MIN_SIZE = (1 << (BITS - 4));

    //printf("bitmask: %16llx\n", chunkBoundaryBitMask);
    //exit(0);

    while((next = fgetc(is)) != -1)
    {
      ++size;

      hash = rp.append8(hash, (char)next);
      val = rw.slide8((char)next);

      if( ((val & chunkBoundaryBitMask) == 0 && size > MIN_SIZE) ||
          (MAX_SIZE != -1 && size > MAX_SIZE) )
      {
          printChunkData("Found", size, val, hash);
          size = 0;
          hash = 0;
          rw.reset();
      }
    }

    fclose(is);

    printChunkData("Last ", size, val, hash);
  }
