CC=gcc
CC_WIN=x86_64-w64-mingw32-gcc

SRC = $(wildcard src/*)

ODIR = obj
ODIR_WIN = obj_win

mijnambacht: $(SRC)
	$(CC) src/main.c -lm -ldl -o mijnambacht -g -Iinclude -I/usr/local/include/SDL2 -Bstatic -lSDL2 -Wall external.o -pthread 

external:
	$(CC) src/external_libs.c -c -o external.o -g -Iinclude \
    -I/usr/local/include/SDL2 -lSDL2 -Wall 

.PHONY: clean
clean:
	rm -rf $(ODIR) $(ODIR_WIN) mijnambacht 

