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
#include <unistd.h>
#include "rabinpoly.h"

#define INT64(n) n##LL
#define MSB64 INT64(0x8000000000000000)
#define FINGERPRINT_PT 0xbfe6b8a5bf378d83LL	

typedef unsigned short BOOL;

//utility functions probably belong somewhere else

BOOL fileExists(const char* fname)
{
  return access( fname, F_OK ) != -1;
}

//end utility functions


void printChunkData(
    const char* msgPrefix,
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

class ChunkBoundaryChecker
{
  public:
  virtual BOOL isBoundary(u_int64_t fingerprint, int size) = 0;
};

class BitwiseChunkBoundaryChecker : public ChunkBoundaryChecker
{
  private:
    const int BITS;
    u_int64_t CHUNK_BOUNDARY_MASK;
    const int MAX_SIZE;
    const int MIN_SIZE;

  public:
  BitwiseChunkBoundaryChecker(int numBits)
    : BITS(numBits),
      CHUNK_BOUNDARY_MASK(makeBitMask(BITS)), 
      MAX_SIZE(4 * (1 << BITS)),
      MIN_SIZE(1 << (BITS - 2))
  {
    //printf("bitmask: %16llx\n", chunkBoundaryBitMask);
    //exit(0);
  }

  virtual BOOL isBoundary(u_int64_t fingerprint, int size)
  {
      return (((fingerprint & CHUNK_BOUNDARY_MASK) == 0 && size > MIN_SIZE) ||
              (MAX_SIZE != -1 && size > MAX_SIZE) );
  }
};

class ChunkProcessor
{
private:
  int size;

protected:
  virtual void internalProcessByte(char c) = 0;
  virtual void internalCompleteChunk(u_int64_t hash, u_int64_t fingerprint) = 0;

public:
  ChunkProcessor() : size(0) {}

  void processByte(char c)
  {
     ++size;
     internalProcessByte(c);
  }

  void completeChunk(u_int64_t hash, u_int64_t fingerprint)
  {
    internalCompleteChunk(hash, fingerprint);
    size = 0;
  }

  virtual int getSize()
  {
    return size;
  }
};

class PrintingChunkProcessor : public ChunkProcessor
{
protected:
  virtual void internalProcessByte(char c) {}

  virtual void internalCompleteChunk(u_int64_t hash, u_int64_t fingerprint)
  {
    printChunkData("Found", getSize(), fingerprint, hash);
  }
};

  void processChunks(FILE* is,
                     ChunkBoundaryChecker& chunkBoundaryChecker,
                     ChunkProcessor& chunkProcessor)
  {
    const u_int64_t POLY = FINGERPRINT_PT;
    window rw(POLY);
    rabinpoly rp(POLY);

    int next;
    u_int64_t val = 0;
    u_int64_t hash = 0;

    while((next = fgetc(is)) != -1)
    {
      chunkProcessor.processByte(next);

      hash = rp.append8(hash, (char)next);
      val = rw.slide8((char)next);

      if( chunkBoundaryChecker.isBoundary(val, chunkProcessor.getSize()) )
      {
          chunkProcessor.completeChunk(hash, val);
          hash = 0;
          rw.reset();
      }
    }

    chunkProcessor.completeChunk(hash, val);
  }

  int main(int argc, char **argv)
  {
    if( argc < 2 )
    {
      printf("Usage: rabin <file name>\n");
      exit(-1);
    }

    FILE* is = fopen(argv[1], "r");

    if( is == 0 )
    {
       printf("Could not open %s\n", argv[1]);
       exit(-2);
    }

    BitwiseChunkBoundaryChecker bcbc(12);
    PrintingChunkProcessor pcp;
    processChunks(is, bcbc, pcp);

    fclose(is);
  }

