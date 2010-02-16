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

#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include <iterator>

using namespace std;

#define INT64(n) n##LL
#define MSB64 INT64(0x8000000000000000)
#define FINGERPRINT_PT 0xbfe6b8a5bf378d83LL	

typedef unsigned short BOOL;

const int FALSE = 0;
const int TRUE  = 1;

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
  printf("%s chunk hash: %016llx fingerprint: %016llx length: %d\n",
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
      return (((fingerprint & CHUNK_BOUNDARY_MASK) == 0 && size >= MIN_SIZE) ||
              (MAX_SIZE != -1 && size >= MAX_SIZE) );
  }

  int getMaxChunkSize() { return MAX_SIZE; }

};

class ChunkProcessor
{
private:
  int size;

//protected:
public:
  virtual void internalProcessByte(char c)
  {
     ++size;
//printf("processed byte %d\n", getSize());
  }

  virtual void internalCompleteChunk(u_int64_t hash, u_int64_t fingerprint)
  {
    size = 0;
  }

public:
  ChunkProcessor() : size(0) {}

  void processByte(char c) 
  {
     internalProcessByte(c);
  }

  void completeChunk(u_int64_t hash, u_int64_t fingerprint)
  {
    internalCompleteChunk(hash, fingerprint);
  }

  virtual int getSize()
  {
    return size;
  }
};

string toString(u_int64_t num)
{
  ostringstream oss;
  oss << setfill('0') << setw(16) << hex << num;
  return oss.str();
}


class PrintChunkProcessor : public ChunkProcessor
{
private:
protected:
  virtual void internalCompleteChunk(u_int64_t hash, u_int64_t fingerprint)
  {
    printChunkData("Found", getSize(), fingerprint, hash);
    ChunkProcessor::internalCompleteChunk(hash, fingerprint);
  }

public:
  PrintChunkProcessor()
  {
  }
};

class CreateFileChunkProcessor : public ChunkProcessor
{
private:
  string      chunkDir;
  FILE*       tmpChunkFile;
protected:
  virtual void internalProcessByte(char c)
  {
    ChunkProcessor::internalProcessByte(c);
    fputc(c, getTmpChunkFile());
  }

  virtual void internalCompleteChunk(u_int64_t hash, u_int64_t fingerprint)
  {
    ChunkProcessor::internalCompleteChunk(hash, fingerprint);

    string chunkName(chunkDir);
    chunkName += "/";
    chunkName += toString(hash);
    chunkName += ".rabin";

    fclose(tmpChunkFile);
    tmpChunkFile = NULL;

    //if chunk already exists, just delete tmp chunk file
    if( fileExists(chunkName.c_str()) )
    {
       unlink(getTmpChunkFileName().c_str());
    }
    //else rename tmp chunk to correct chunk name and close file
    else
    {
       rename(getTmpChunkFileName().c_str(), chunkName.c_str());
    }
  }

public:
  CreateFileChunkProcessor(string chunkDir)
    : tmpChunkFile(NULL),
      chunkDir(chunkDir)
  {
  }

  ~CreateFileChunkProcessor()
  {
    if( tmpChunkFile != NULL )
    {
      cerr << "Final chunk never completed!" << endl;
      fclose(tmpChunkFile);
      tmpChunkFile = NULL;
    }
  }

  string getTmpChunkFileName()
  {
      string s(chunkDir);
      s += "/tempChunk.rabin.tmp";
      return s;
  }

  FILE* getTmpChunkFile()
  {
    if( tmpChunkFile == NULL )
    {
      string s = getTmpChunkFileName();
      tmpChunkFile = fopen(s.c_str(), "w");

      if( tmpChunkFile == 0 )
      {
         printf("Could not open %s\n", s.c_str());
         exit(-3);
      }
    }

    return tmpChunkFile;
  }
};

class CompressChunkProcessor : public ChunkProcessor
{
private:
  FILE*   outfile;
  int     maxChunkSize;
  char*   buffer;
  long    fileLoc;
  map<u_int64_t, long> chunkLocations;
protected:
  virtual void internalProcessByte(char c)
  {
    if( getSize() < maxChunkSize)
    {
      buffer[getSize()] = c;
    }
    else
    {
      fprintf(stderr, "Compression buffer overflow, size = %d\n", getSize());
    }

    ChunkProcessor::internalProcessByte(c);
    ++fileLoc;
  }

  virtual void internalCompleteChunk(u_int64_t hash, u_int64_t fingerprint)
  {
    //if this is the very first chunk, just write it out
    if( chunkLocations.size() == 0 )
    {
      fwrite(buffer, 1, getSize(), outfile);
      chunkLocations[hash] = fileLoc;
    }
    else
    {
      map<u_int64_t, long>::iterator chunkIter = chunkLocations.find(hash);

      //otherwise if it hasn't been written out, mark it as an in-place chunk & record its location for future reference
      if( chunkIter == chunkLocations.end() )
      {
        char flag = 0;
        fwrite(&flag, 1, 1, outfile);
        fwrite(buffer, 1, getSize(), outfile);
      }
      //otherwise mark this as an already found chunk & write the location
      else
      {
        char flag = 1;
        fwrite(&flag, 1, 1, outfile);
        long chunkLoc = chunkIter->second;
        fwrite(&chunkLoc, sizeof(long), 1, outfile);
      }
    }

    ChunkProcessor::internalCompleteChunk(hash, fingerprint);
  }

public:
  CompressChunkProcessor(FILE* outfile, int maxChunkSize)
    : outfile(outfile),
      maxChunkSize(maxChunkSize),
      buffer(new char[maxChunkSize]),
      fileLoc(0)
  {
  }

  ~CompressChunkProcessor()
  {
    if( outfile != stdout )
    {
      fclose(outfile);
      outfile = NULL;
    }

    delete[] buffer;
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

int requireInt(char* str)
{
  char* str_orig = str;


  //skip whitespace
  while(str[0] == ' ' || str[0] == '\t') ++str;

  //validate
  int i = 0;
  while(str[i] != 0)
  {
    int digit = str[i] - '0';
    if( digit < 0 || digit > 9 )
    {
      fprintf(stderr, "%s is not a number\n", str_orig);
      exit(-1);
    }

    ++i;
  }


  return atoi(str);
}

class Options
{
public:
  string filename;
  string chunkDir;
  string outFilename;
  string inFilename;
  BOOL   compress;
  BOOL   extract;
  BOOL   print;
  BOOL   reconstruct;
  int    bits;

  Options(int argc, char** argv)
    : compress   (FALSE),
      extract    (FALSE),
      print      (FALSE),
      reconstruct(FALSE),
      bits       (12)
  {
    setOptionsFromArguments(argc, argv);
    validateOptionCombination();
  }

  void usage()
  {
    fprintf(stderr, "Flags:\n");
    fprintf(stderr, "-b <log2 of the average chunk length desired, default is 12>\n");
    fprintf(stderr, "-d <directory in which to put/retrieve chunks>\n");
    fprintf(stderr, "-p \"print chunk data for the file\" (to standard out)\n");
    fprintf(stderr, "-c \"rabin enCode/Compress file\" (to standard out or outfile)\n");
    fprintf(stderr, "-x \"rabin eXtract/decompress file\" (to standard out or outfile)\n");
    fprintf(stderr, "-r \"reconstruct file from chunk dir and printed chunk data\" (to standard out or outfile)\n");
    fprintf(stderr, "-o <file in which to put output>\n");

    fprintf(stderr, "\nFlags c, x and r are mutually exclusive.  Flag p is incompatible\n");
    fprintf(stderr, "with r.  p is incompatible with x and c unless -o is also given.\n");
  }

  void unsupported(string s)
  {
    fprintf(stderr, "%s not yet supported\n", s.c_str());
    exit(-1);
  }

  void setOptionsFromArguments(int argc, char** argv)
  {
    int c;
    extern char *optarg;
    extern int optind, optopt;

    while ((c = getopt(argc, argv, ":cxprd:o:b:")) != -1) {
      switch(c) {
      case 'c':
          compress = TRUE;
          break;
      case 'x':
unsupported("-x");
          extract = TRUE;
          break;
      case 'p':
          print = TRUE;
          break;
      case 'r':
unsupported("-r");
          reconstruct = TRUE;
          break;
      case 'd':
          chunkDir = optarg;
          break;
      case 'o':
unsupported("-o");
          outFilename = optarg;
          break;
      case 'b':
          bits = requireInt(optarg);
          break;
      case ':':       /* -d or -o without operand */
          fprintf(stderr,
                  "Option -%c requires an operand\n", optopt);
          exit(-1);
          break;
      case '?':
          usage();
          exit(-1);
      }
    }

    if( optind != argc - 1 )
    {
      fprintf(stderr, "Expected exactly one input file specified.\n");
      exit(-1);
    }

    inFilename = argv[optind];
  }

  void validateOptionCombination()
  {
    //-d is compatible with any flag
   
    if( print )
    {
      //print is incompatible with reconstruct
      if( reconstruct )
      {
        fprintf(stderr, "-p (print) flag makes no sense with -r (reconstruct) flag\n");
        exit(-1);
      }

      //print is incompatible with extract and compress unless outFilename
      //is also given.
      if( (extract || compress) && outFilename == "" )
      {
        fprintf(stderr, "-p (print) flag combined with -c (compress) or -x (extract) requires\n-o (output file name) where compress/extract output goes.\n");
        exit(-1);
      }
    }

    if( compress )
    {
      //compress and extract are mutually exclusive
      if( extract )
      {
        fprintf(stderr, "-c (compress) flag cannot be combined with -x (extract)\n");
        exit(-1);
      }

      //compress and reconstruct are mutually exclusive
      if( reconstruct )
      {
        fprintf(stderr, "-c (compress) flag cannot be combined with -r (reconstruct)\n");
        exit(-1);
      }
    }

    if( reconstruct && (chunkDir == "") )
    {
      fprintf(stderr, "-r (reconstruct) requires -d (chunk directory) to be specified\n");
      exit(-1);
    }
  }
};

class OptionsChunkProcessor : public ChunkProcessor
{
private:
  vector<ChunkProcessor*> processors;

protected:
  virtual void internalProcessByte(char c)
  {
    ChunkProcessor::internalProcessByte(c);
    for(vector<ChunkProcessor*>::iterator procIter = processors.begin();
        procIter != processors.end();
        ++procIter)
    {
      (*procIter)->internalProcessByte(c);
    }
  }

  virtual void internalCompleteChunk(u_int64_t hash, u_int64_t fingerprint)
  {
    ChunkProcessor::internalCompleteChunk(hash, fingerprint);
    for(vector<ChunkProcessor*>::iterator procIter = processors.begin();
        procIter != processors.end();
        ++procIter)
    {
      (*procIter)->internalCompleteChunk(hash, fingerprint);
    }
  }

public:
  OptionsChunkProcessor(Options opts, int maxChunkSize)
  {
    //build list of processors
    if( opts.print ) processors.push_back(new PrintChunkProcessor());

    if( opts.chunkDir != "" )
    {
      processors.push_back(new CreateFileChunkProcessor(opts.chunkDir));
    }

    if( opts.compress )
    {
      processors.push_back(new CompressChunkProcessor(stdout, maxChunkSize));
    }
  }

  ~OptionsChunkProcessor()
  {
    for(vector<ChunkProcessor*>::iterator procIter = processors.begin();
        procIter != processors.end();
        ++procIter)
    {
      delete *procIter;
    }
  }
};



  int main(int argc, char **argv)
  {
    Options opts(argc, argv);

    FILE* is = fopen(opts.inFilename.c_str(), "r");

    if( is == 0 )
    {
       printf("Could not open %s\n", opts.inFilename.c_str());
       exit(-2);
    }

    BitwiseChunkBoundaryChecker cbc(opts.bits);
    OptionsChunkProcessor cp(opts, cbc.getMaxChunkSize());
    processChunks(is, cbc, cp);

    fclose(is);
  }

