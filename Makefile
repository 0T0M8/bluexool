# =================== CONFIG ===================

CC       := gcc
CFLAGS   := -Wall -Wextra -O2 -Iincludes
LDLIBS   := -lsqlite3 -lcrypt

# bcrypt (portable version)
BCRYPT_DIR := lib
BCRYPT_LIB := $(BCRYPT_DIR)/bcrypt.a

# Directories
SRC_DIR   := src
BUILD_DIR := build

SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC))

OUT := bluexool

# =================== TARGETS ===================

.PHONY: all clean

all: $(OUT)

# Link final binary
$(OUT): $(OBJ) $(BCRYPT_LIB)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

# Build object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Ensure build directory exists
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build bcrypt automatically if missing
#$(BCRYPT_LIB):
#	$(MAKE) -C $(BCRYPT_DIR)

clean:
	rm -rf $(BUILD_DIR) $(OUT)
	$(MAKE) -C $(BCRYPT_DIR) clean
