// $Id$

/*
 *
 * Copyright (C) 2010 Bobby Martin (bobbymartin2@gmail.com)
 * Copyright (C) 1998 David Mazieres (dm@uun.org)
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

/*
 * Routines for calculating the most significant bit of an integer.
 */

public class MostSignificantBitUtil
{
  /* Find last set (most significant bit) */
  public static /*unsigned*/ short fls (/*unsigned*/ int v)
  {
    if ((v & 0xffff0000) != 0) {
      if ((v & 0xff000000) != 0)
        return (short)(24 + bytemsb[v>>24]);
      else
        return (short)(16 + bytemsb[v>>16]);
    }
    if ((v & 0x0000ff00) != 0)
      return (short)(8 + bytemsb[v>>8]);
    else
      return (short)(bytemsb[v]);
  }
  
  public static byte fls (/*unsigned*/ long v)
  {
    /*unsigned*/ int h;
    if ((h = (int)(v >> 32)) != 0)
      return (byte)(32 + fls (h));
    else
      return (byte)fls ((/*unsigned*/ int) v);
  }
  
  public static int
  log2c (/*unsigned*/ long v)
  {
    return v != 0 ? (int) fls (v - 1) : -1;
  }
  
  
  /*
   * For symmetry, a 64-bit find first set, "ffs," that finds the least
   * significant 1 bit in a word.
   */
  
  
  public static /*unsigned*/ short
  ffs (/*unsigned*/ int v)
  {
    int vv;
    if ((v & 0xffff) != 0) {
      if ((vv = v & 0xff) != 0)
        return bytelsb[vv];
      else
        return (short)(8 + bytelsb[v >> 8 & 0xff]);
    }
    else if ((vv = v & 0xff0000) != 0)
      return (short)(16 + bytelsb[vv >> 16]);
    else if (v != 0)
      return (short)(24 + bytelsb[v >> 24 & 0xff]);
    else
      return 0;
  }
  
  public static /*unsigned*/ short
  ffs (/*unsigned*/ long v)
  {
    /*unsigned*/ int l;
    if ((l = (int)(v & 0xffffffff)) != 0)
      return fls (l);
    else if ((l = (int)(v >> 32)) != 0)
      return (short)(32 + fls (l));
    else
      return 0;
  }
  
  
  
  /* Highest bit set in a byte */
  private static byte bytemsb[] = {
    0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
  };
  
  /* Least set bit (ffs) */
  private static byte bytelsb[] = {
    0, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1, 5, 1, 2, 1, 3, 1, 2, 1,
    4, 1, 2, 1, 3, 1, 2, 1, 6, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1,
    5, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1, 7, 1, 2, 1, 3, 1, 2, 1,
    4, 1, 2, 1, 3, 1, 2, 1, 5, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1,
    6, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1, 5, 1, 2, 1, 3, 1, 2, 1,
    4, 1, 2, 1, 3, 1, 2, 1, 8, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1,
    5, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1, 6, 1, 2, 1, 3, 1, 2, 1,
    4, 1, 2, 1, 3, 1, 2, 1, 5, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1,
    7, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1, 5, 1, 2, 1, 3, 1, 2, 1,
    4, 1, 2, 1, 3, 1, 2, 1, 6, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1,
    5, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1,
  };
}
