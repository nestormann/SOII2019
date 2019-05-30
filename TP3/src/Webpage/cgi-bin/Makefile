targets = cpuInfo.cgi baash.cgi list_mod.cgi upload.cgi
CFLAGS = -pedantic -Wall -Werror

.PHONY: all

all: $(targets)

cpuInfo.cgi: main.o practico1.o
	$(CC) $(CFLAGS) $^ -o $@

main.o: ../../CpuInfo/main.c ../../CpuInfo/practico1.h
	$(CC) $(CFLAGS) -c $< -o $@

practico1.o: ../../CpuInfo/practico1.c ../../CpuInfo/practico1.h
	$(CC) $(CFLAGS) -c $< -o $@

baash.cgi: solution.o
	$(CC) $(CFLAGS) $^ -o $@

solution.o: ../../Baash/solution.c
	$(CC) $(CFLAGS) -c $< -o $@

list_mod.cgi: list_mod.o
	$(CC) $(CFLAGS) $^ -o $@

list_mod.o: ../../modulitos/list_mod.c
	$(CC) $(CFLAGS) -c $< -o $@

upload.cgi:
	cp ../../modulitos/upload.cgi ./

.PHONY: clean

clean:
	$(RM) $(targets) *.o
