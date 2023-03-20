CC = gcc
CFLAGS = -Wall -g -std=c99
LDFLAGS= -L.

INC = include/
INC_UTIL = include/util/
SRC = src/
BIN = bin/

ID3_SRC_FILES = $(wildcard src/id3*.c)
ID3_OBJ_FILES = $(patsubst src/id3%.c,bin/id3%.o,$(ID3_SRC_FILES))


sharedParser: $(BIN)libid3.so

staticParser: $(ID3_OBJ_FILES) $(INC_UTIL)id3*.h $(INC)id3v2.h 
	$(CC) $(CFLAGS) -I$(INC_UTIL) -I$(INC) $(ID3_OBJ_FILES) -c -o $(BIN)libid3.o

main: main.c $(ID3_OBJ_FILES) $(INC_UTIL)id3*.h $(INC)id3v2.h
	$(CC) $(CFLAGS) main.c $(ID3_OBJ_FILES) -I$(INC_UTIL) -I$(INC) -lm -o main

$(BIN)libid3.so: $(ID3_OBJ_FILES)
	$(CC) -shared -o $(BIN)libid3.so $(ID3_OBJ_FILES)

$(BIN)id3%.o: $(SRC)id3%.c $(INC_UTIL)id3*.h $(INC)id3v2.h
	$(CC) $(CFLAGS) -I$(INC) -I$(INC_UTIL) -lm -c -fpic $< -o $@

clean:
	rm -rf $(BIN)*.o $(BIN)*.so main *.out
###################################################################################################
