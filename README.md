# gauss
Aby włączyć tryb wypisywania wiadomości o stanie programu należy ustawić w Makefile -D_DEBUG na 1

```
CC=g++
CFLAGS=-I. -D_DEBUG=1
DEPS = 
OBJ = gauss.o

%.o: %.cxx
        $(CC) -c -o $@ $< $(CFLAGS)

solver: $(OBJ)
        $(CC) -o $@ $^ $(CFLAGS)

clean:
        rm -f solver $(OBJ)
```

Aby zbudować należy skorzystać z komendy `make` bezpośrednio w katalogu źródeł programu
