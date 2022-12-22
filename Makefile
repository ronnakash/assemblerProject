CC=gcc
INFILES= util.c tables.c instructions.c secondPass.c firstPass.c inOut.c assembler.c

CFLAGS= -ansi -Wall -pedantic -o

OUTNAME= assembler

ex: compile run clean

compile:
	$(CC) $(INFILES) $(CFLAGS) $(OUTNAME)

run:
	./$(OUTNAME) 

clean: 
	rm $(OUTNAME)
