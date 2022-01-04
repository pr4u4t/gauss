CC=g++
CFLAGS=-I. -D_DEBUG=0
DEPS = 
OBJ = gauss.o

%.o: %.cxx
	$(CC) -c -o $@ $< $(CFLAGS)

solver: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f solver $(OBJ)
