CC = gcc
CFLAGS = -Wall -Wextra

OBJECTS = main.o common.o pasazer.o kapitan_statku.o kapitan_portu.o

all: statek

statek: $(OBJECTS)
	$(CC) $(CFLAGS) -o statek $(OBJECTS)

main.o: main.c common.h
	$(CC) $(CFLAGS) -c main.c

common.o: common.c common.h
	$(CC) $(CFLAGS) -c common.c

pasazer.o: pasazer.c common.h
	$(CC) $(CFLAGS) -c pasazer.c

kapitan_statku.o: kapitan_statku.c common.h
	$(CC) $(CFLAGS) -c kapitan_statku.c

kapitan_portu.o: kapitan_portu.c common.h
	$(CC) $(CFLAGS) -c kapitan_portu.c

clean:
	rm -f statek $(OBJECTS)

.PHONY: all clean