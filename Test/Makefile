#	Make test files

CFLAGS = -Wall -g -dDEBUG -I..

OBJECTS = AccessTest.o HTAccess.o HTParse.o ../obj/HTTP.o
ROBJECTS = Relative.o HTParse.o

Relative : $(ROBJECTS)
	cc $(CFLAGS) -o Relative $(ROBJECTS)

Relative.o : Relative.c ../HTParse.h
	cc $(CFLAGS) -c -o Relative.o Relative.c

HTParse.o : ../HTParse.c ../HTParse.h
	cc $(CFLAGS) -c -o HTParse.o ../HTParse.c
 

AccessTest : $(OBJECTS)
	cc $(CFLAGS) -o AccessTest $(OBJECTS)

HTAccess.o : ../HTAccess.c ../HTAccess.h
	cc $(CFLAGS) -c -o HTAccess.o ../HTAccess.c

AccessTest.o : AccessTest.c ../HTParse.h ../HTTP.h
	cc $(CFLAGS) -c -o AccessTest.o AccessTest.c

