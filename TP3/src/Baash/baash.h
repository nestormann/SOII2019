#ifndef INCLUDE_BAASH_H
#define INCLUDE_BAASH_H

	extern const char* searchPath(char*, char*, char*);
	extern const char* searchFile(char*, char*);
	extern void parse(char**,int*,char* argv[],int*,int*);
	extern const char* formatAndSearch(char*,char*);
    extern int baash(char*);

#endif
