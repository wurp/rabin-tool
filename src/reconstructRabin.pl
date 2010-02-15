#!/usr/bin/perl -wn
#if( $_ ~= /hash *[ 0-9a-f][0-9a-f][^0-9a-f]/ )
#if( $_ ~= /hash/ )
if( m/hash: *((?:[ 0-9a-f]{15})[0-9a-f])[^0-9a-f]/ )
{
  $hash = $1;
  $hash =~ s/ /0/g;
  print "/home/wurp/projects/rabin/chunks/$hash.rabin\n";
}
