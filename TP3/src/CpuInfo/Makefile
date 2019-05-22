targets = cpuInfo.cgi

.PHONY: all

all: $(targets)

cpuInfo.cgi: main.o practico1.o
	$(CC) $(CFLAGS) $^ -o $@

main.o: main.c practico1.h
	$(CC) $(CFLAGS) -c $< -o $@

practico1.o: practico1.c practico1.h
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	$(RM) $(targets) *.o
