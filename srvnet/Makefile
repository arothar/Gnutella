CC = gcc
CFLAGS = -Wall -ansi
OBJETOS = srvnet.o srvnet-proc.o Genericas.o

srvnet: $(OBJETOS)
	$(CC) $(CFLAGS) -o $@ $^

srvnet.o: srvnet.c srvnet.h
	$(CC) $(CFLAGS) -c $<

srvnet-proc.o: srvnet-proc.c srvnet-proc.h
	$(CC) $(CFLAGS) -c $<

Genericas.o: Genericas.c Genericas.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o srvnet *.log *~
