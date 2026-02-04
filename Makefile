CC = gcc
CFLAGS = -Wall -Wextra -O2 -Iincludes

LDFLAGS = -lsqlite3

SRC = src/main.c src/router.c src/studentsdb.c src/http.c
OUT = bluexool

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) -o $(OUT) $(SRC) $(LDFLAGS)

clean:
	rm -f $(OUT)
