CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -g -Iinclude

SRC = src/main.c src/memtrack.c
OBJ = build/main.o build/memtrack.o

TARGET = memtrack

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

build/main.o: src/main.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c src/main.c -o build/main.o

build/memtrack.o: src/memtrack.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c src/memtrack.c -o build/memtrack.o

clean:
	rm -rf build $(TARGET)