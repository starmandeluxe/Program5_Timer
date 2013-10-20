# 
# Makefile for timer
# 

MAKE = make
CC = gcc
RM = rm -f
CFLAGS = -g

CSOURCE = timer.c

INCLUDES = -I/usr/include


all: timer
	@echo done

timer.o: timer.c
	@echo "C-compiling 'timer.c'"
	@$(CC) -g -c  $(INCLUDES) timer.c

APPL_OBJECTS = timer.o

timer: $(APPL_OBJECTS)
	@echo Building executable file
	@$(CC) -o timer $(APPL_OBJECTS)
	@echo "Executable 'timer' made"

clean::
	@$(RM) *.o
	@$(RM) timer


