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

import java.io.InputStream;
import java.io.FileInputStream;
import java.io.BufferedInputStream;

public class RabinWindow extends RabinPoly
{
  public int size;

  private /*unsigned*/ long fingerprint;
  private int bufpos;
  private /*unsigned*/ long U[] = new long[256];
  private /*unsigned*/ byte buf[];
  
  // made the window size configurable
  public RabinWindow (/*unsigned*/ long poly)
  {
    this(poly, DEFAULT_WINDOW_SIZE);
  }

  public RabinWindow (/*unsigned*/ long poly, /*unsigned*/ int winsz)
  {
    super (poly);
    this.size = winsz;
    this.fingerprint = 0;
    this.bufpos = -1;

    /*unsigned*/ long sizeshift = 1;
    for (int i = 1; i < size; i++)
      sizeshift = append8 (sizeshift, (byte)0);
    for (int i = 0; i < 256; i++)
      U[i] = polymmult (i, sizeshift, poly);
    buf = new /*unsigned*/ byte[winsz];
  }
  
    /*unsigned*/ long slide8 (/*unsigned*/ byte m) {
      if (++bufpos >= size)
        bufpos = 0;
      /*unsigned*/ byte om = buf[bufpos];
      buf[bufpos] = m;
      return fingerprint = append8 (fingerprint ^ U[om], m);
    }
  
    void reset () { 
      fingerprint = 0; 
      for(byte b : buf) { b = 0; }
    }

  public static void main(String[] args) throws Exception
  {
    RabinWindow rw = new RabinWindow(0x0087a30a3487a395l);

    InputStream is = new BufferedInputStream(new FileInputStream(args[0]));
    int next;
    int size = 0;
    long val = 0;
    while((next = is.read()) != -1)
    {
      ++size;

      val = rw.slide8((byte)next);
      //break (on average) every 2^16 bytes
      if( (val & 0xffffffffffff0000l) == 0 )
      {
          System.out.printf("Found chunk of length %d with fingerprint %x\n", size, val);
          size = 0;
          rw.reset();
      }
    }

    is.close();

    System.out.printf("Last chunk was of length %d with fingerprint %x\n", size, val);
  }
}
