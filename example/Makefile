
CC	 = gcc
LDCC	 = $(CC)
LIBS	 = -L../src 
INCS     = -I../src 
LDFLAGS  = -lrabinpoly
BIN	 = example1
SRCS	 = example1.cpp

.cpp.o:
	$(CC) -c $(INCS) $< -o $@

OBJS 	 = $(SRCS:.cpp=.o)

$(BIN):	$(OBJS)
	$(LDCC) -o $@ $(OBJS) $(LIBS) $(INCS) $(LDFLAGS)

clean:
	rm -f $(BIN) *.o core 
