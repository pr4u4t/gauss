CC=g++
CFLAGS=-I.
DEPS = #hellomake.h
OBJ = gauss.o

%.o: %.cxx
	$(CC) -c -o $@ $< $(CFLAGS)

solver: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
