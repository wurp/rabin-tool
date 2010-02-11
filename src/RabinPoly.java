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

class LoggerDummy
{
  boolean isDebugEnabled() { return false; }
}

public class RabinPoly
{
  public static final LoggerDummy mLogger = new LoggerDummy();
  public static final int DEFAULT_WINDOW_SIZE = 16;

  private int shift;
  private /*unsigned*/ long T[] = new /*unsigned*/ long[256];		// Lookup table for mod

  public final /*unsigned*/ long poly;		// Actual polynomial

  public RabinPoly (/*unsigned*/ long p)
  {
    this.poly = p;
    calcT ();
  }

  public /*unsigned*/ long append8 (/*unsigned*/ long p, /*unsigned*/ byte m)
  {
    return ((p << 8) | m) ^ T[(int)(p >> shift)];
  }


  public static final /*unsigned*/ long MSB64 = 0x8000000000000000l;
                      
  private static /*unsigned*/ long
  polymod (/*unsigned*/ long nh, /*unsigned*/ long nl, /*unsigned*/ long d)
  {
    if( mLogger.isDebugEnabled() )
    {
      System.out.printf ("polymod (nh %llu nl %llu d %llu)\n", nh, nl, d);
    }
    int k = MostSignificantBitUtil.fls (d) - 1;
    if( mLogger.isDebugEnabled() )
    {
    System.out.printf ("polymod : k = %d\n", k);
    }
    d <<= 63 - k;
    if( mLogger.isDebugEnabled() )
    {
    System.out.printf ("polymod : d = %llu\n", d);
    System.out.printf ("polymod : MSB64 = %llu\n", MSB64);
    }
  
    if (nh != 0) {
      if ((nh & MSB64) != 0)
        nh ^= d;
    if( mLogger.isDebugEnabled() )
    {
      System.out.printf ("polymod : nh = %llu\n", nh);
    }
      for (int i = 62; i >= 0; i--)
        if ((nh & ((/*unsigned*/ long) 1) << i) != 0) {
  	nh ^= d >> (63 - i);
  	nl ^= d << (i + 1);
    if( mLogger.isDebugEnabled() )
    {
  	System.out.printf ("polymod : i = %d\n", i);
  	System.out.printf ("polymod : shift1 = %llu\n", (d >> (63 - i)));
  	System.out.printf ("polymod : shift2 = %llu\n", (d << (i + 1)));
  	System.out.printf ("polymod : nh = %llu\n", nh);
  	System.out.printf ("polymod : nl = %llu\n", nl);
    }
        }
    }
    for (int i = 63; i >= k; i--)
    {  
      if ((nl & 1 << i) != 0)
        nl ^= d >> 63 - i;
    if( mLogger.isDebugEnabled() )
    {
      System.out.printf ("polymod : nl = %llu\n", nl);
    }
    }
    
    if( mLogger.isDebugEnabled() )
    {
    System.out.printf ("polymod : returning %llu\n", nl);
    }
    return nl;
  }
  
  private static /*unsigned*/ long
  polygcd (/*unsigned*/ long x, /*unsigned*/ long y)
  {
    for (;;) {
      if (y == 0)
        return x;
      x = polymod (0, x, y);
      if (x == 0)
        return y;
      y = polymod (0, y, x);
    }
  }
  
  long[] //php, plp
  polymult (/*unsigned*/ long x, /*unsigned*/ long y)
  {
    if( mLogger.isDebugEnabled() )
    {
    System.out.printf ("polymult (x %llu y %llu)\n", x, y);
    }
    /*unsigned*/ long ph = 0, pl = 0;
    if ((x & 1) != 0)
      pl = y;
    for (int i = 1; i < 64; i++)
      if ((x & (1 << i)) != 0) {
    if( mLogger.isDebugEnabled() )
    {
        System.out.printf ("polymult : i = %d\n", i);
        System.out.printf ("polymult : ph = %llu\n", ph);
        System.out.printf ("polymult : pl = %llu\n", pl);
        System.out.printf ("polymult : y = %llu\n", y);
        System.out.printf ("polymult : ph ^ y >> (64-i) = %llu\n", (ph ^ y >> (64-i)));
        System.out.printf ("polymult : pl ^ y << i = %llu\n", (pl ^ y << i));
    }
        ph ^= y >> (64 - i);
        pl ^= y << i;
    if( mLogger.isDebugEnabled() )
    {
    System.out.printf ("polymult : ph %llu pl %llu\n", ph, pl);
    }
      }


    if( mLogger.isDebugEnabled() )
    {
    System.out.printf ("polymult : h %llu l %llu\n", ph, pl);
    }

    return new long[] { ph, pl };
  }
  
  /*unsigned*/ long
  polymmult (/*unsigned*/ long x, /*unsigned*/ long y, /*unsigned*/ long d)
  {
    if( mLogger.isDebugEnabled() )
    {
    System.out.printf ("polymmult (x %llu y %llu d %llu)\n", x, y, d);
    }
    /*unsigned*/ long h, l;
    long[] pmv = polymult (x, y);
    h = pmv[0];
    l = pmv[1];
    return polymod (h, l, d);
  }
  
  boolean
  polyirreducible (/*unsigned*/ long f)
  {
    /*unsigned*/ long u = 2;
    int m = (MostSignificantBitUtil.fls (f) - 1) >> 1;
    for (int i = 0; i < m; i++) {
      u = polymmult (u, u, f);
      if (polygcd (f, u ^ 2) != 1)
        return false;
    }
    return true;
  }
  
  void
  calcT ()
  {
    if( mLogger.isDebugEnabled() )
    {
    System.out.printf ("RabinPoly::calcT ()\n");
    }
  //  assert (poly >= 0x100);
    int xshift = MostSignificantBitUtil.fls (poly) - 1;
    shift = xshift - 8;
    /*unsigned*/ long T1 = polymod (0, 1 << xshift, poly);
    for (int j = 0; j < 256; j++)
    {
      T[j] = polymmult (j, T1, poly) | ((/*unsigned*/ long) j << xshift);
    if( mLogger.isDebugEnabled() )
    {
      System.out.printf ("RabinPoly::calcT tmp = %llu\n", polymmult (j, T1, poly));
      System.out.printf ("RabinPoly::calcT shift = %llu\n", ((/*unsigned*/ long) j <<
  						xshift));
      System.out.printf ("RabinPoly::calcT xshift = %d\n", xshift);
      System.out.printf ("RabinPoly::calcT T[%d] = %llu\n", j, T[j]);
    }
    }
    if( mLogger.isDebugEnabled() )
    {
    System.out.printf ("RabinPoly::calcT xshift = %d\n", xshift);
    System.out.printf ("RabinPoly::calcT T1 = %llu\n", T1);
    System.out.printf ("RabinPoly::calcT T = {");
    for (int i=0; i< 256; i++)
      System.out.printf ("\t%llu \n", T[i]);
    System.out.printf ("}\n");
    }
  }
  
}  
  
class RabinWindow extends RabinPoly
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
}
