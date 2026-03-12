# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Iincludes

# Source files
SRC = src/http.c src/router.c src/middleware.c src/services.c src/core.c src/main.c

# Object files
OBJ = build/http.o build/router.o build/middleware.o build/services.o build/core.o build/main.o

# Executable
EXEC = build/bluexool

all: $(EXEC)

# Link object files with bcrypt static library and SQLite
# ⚠️ Note: we directly reference the bcrypt .a file here
$(EXEC): $(OBJ)
	$(CC) -o $@ $^ lib/bcrypt.a -lsqlite3

# Compile C files to object files
build/%.o: src/%.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build/*

.PHONY: all clean
