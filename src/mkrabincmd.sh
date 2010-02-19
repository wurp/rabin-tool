make
g++ -I. -c rabincmd.C 
g++ rabincmd.o librabinpoly.a 
mv a.out rabin
