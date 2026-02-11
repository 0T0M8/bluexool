# Compiler & flags
CC = gcc
CFLAGS = -Wall -Wextra -O2 -Iincludes
LDFLAGS = -lsqlite3 ./lib/libbcrypt.a 

# Directories
SRC_DIR = src
BUILD_DIR = build
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC))

# Output binary
OUT = bluexool

# =================== TARGETS ===================
.PHONY: all clean

all: $(BUILD_DIR) $(OUT)

# Build binary from object files
$(OUT): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Build object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Ensure build directory exists
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(OUT)
