CC = gcc
CFLAGS = -Wall -ansi
OBJETOS = upload.o upload-proc.o Genericas.o

upload: $(OBJETOS)
	$(CC) $(CFLAGS) -o $@ $^

upload.o: upload.c upload.h
	$(CC) $(CFLAGS) -c $<

upload-proc.o: upload-proc.c upload-proc.h
	$(CC) $(CFLAGS) -c $<

Genericas.o: Genericas.c Genericas.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o upload
