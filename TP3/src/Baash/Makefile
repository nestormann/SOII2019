targets = baash.cgi
CC = gcc -Wall -pedantic

.PHONY: all

all: $(targets)

baash.cgi: baash1.o baashSeek.o
	$(CC) $(CFLAGS) $^ -o $@

main.o: baash1.c baash.h
	$(CC) $(CFLAGS) -c $< -o $@

practico1.o: baashSeek.c baash.h
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	$(RM) $(targets) *.o
