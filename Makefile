# Compiler
CC = gcc

# Flags
CFLAGS = -Wall -Wextra -O2 -Iincludes

# Libraries
LIBS = lib/bcrypt.a -lsqlite3 -lcrypt

# Directories
SRC = src
BUILD = build

# Source files
CORE_SRC = $(SRC)/core/main.c
ROUTER_SRC = $(SRC)/router/router.c
HTTP_SRC = $(SRC)/http/http.c
HTTPREAD_SRC = $(SRC)/http/httpread.c
STUDENTS_SRC = $(SRC)/services/studentsdb.c
USERS_SRC = $(SRC)/services/usersdb.c

# Object files
CORE_OBJ = $(BUILD)/core.o
ROUTER_OBJ = $(BUILD)/router.o
HTTP_OBJ = $(BUILD)/http.o
HTTPREAD_OBJ = $(BUILD)/httpread.o
STUDENTS_OBJ = $(BUILD)/studentsdb.o
USERS_OBJ = $(BUILD)/usersdb.o

OBJS = $(CORE_OBJ) $(ROUTER_OBJ) $(HTTP_OBJ) $(HTTPREAD_OBJ) $(STUDENTS_OBJ) $(USERS_OBJ)

# Default target
all: $(BUILD)/bluexool

# Ensure build directory exists
$(BUILD):
	mkdir -p $(BUILD)

# Compile core
$(CORE_OBJ): $(CORE_SRC) | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile router
$(ROUTER_OBJ): $(ROUTER_SRC) | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile http
$(HTTP_OBJ): $(HTTP_SRC) | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile http reader
$(HTTPREAD_OBJ): $(HTTPREAD_SRC) | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile services
$(STUDENTS_OBJ): $(STUDENTS_SRC) | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(USERS_OBJ): $(USERS_SRC) | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

# Link executable
$(BUILD)/bluexool: $(OBJS)
	$(CC) $(OBJS) -o $(BUILD)/bluexool $(LIBS)

# Clean build
clean:
	rm -rf $(BUILD)/*.o $(BUILD)/bluexool

# Rebuild everything
re: clean all
