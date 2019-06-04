all: awsget.cgi
	   
awsget.cgi: solution.o
	gcc -pedantic -Wall -Werror -O3 -o awsget.cgi solution.o

solution.o: solution.c 
	gcc -pedantic -Wall -Werror -c solution.c

clean:
	rm -f *.o awsget.cgi

