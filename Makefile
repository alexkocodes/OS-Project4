CC = gcc
CFLAGS = -Wall -g -pthread

# list of source files
SRCS = adzip.c

# list of object files
OBJS = $(SRCS:.c=.o)

# dependencies
# DEPS = 

# default target
all: adzip

# compile the reader program
adzip: adzip.o 
	$(CC) $(CFLAGS) -o $@ $^


# clean up object files and executables
clean:
	rm -f $(OBJS) adzip
