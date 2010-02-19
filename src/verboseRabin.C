/* 
 * Copyright (C) 2010 Bobby Martin (bobbymartin2@gmail.com)
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
#include <stdarg.h>
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

int main(int argc, char **argv)
{
  unsigned char buff[] = { 0, 0, 0, 0, 0x30, 0x30 };

  rabinpoly rply(FINGERPRINT_PT);
  u_int64_t hash = 0;
  for(int i = 0; i < sizeof(buff) / sizeof(*buff); ++i)
  {
    hash = rply.append8(hash, buff[i]);
    printf("New hash is %016llx after appending %02x\n", hash, buff[i]);
  }
 
  printf("0x10 ^ 0x01 = %02x\n", 0x10 ^ 0x01);
}

