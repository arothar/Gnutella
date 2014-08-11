CC = gcc
CFLAGS = -Wall -ansi
OBJETOS = cache.o cache-proc.o Genericas.o

cache: $(OBJETOS)
	$(CC) $(CFLAGS) -o $@ $^

cache.o: cache.c cache.h
	$(CC) $(CFLAGS) -c $<

cache-proc.o: cache-proc.c cache-proc.h
	$(CC) $(CFLAGS) -c $<

Genericas.o: Genericas.c Genericas.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o cache *.log *~
