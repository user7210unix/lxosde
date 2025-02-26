CC = gcc
CFLAGS = -Wall -Wextra -std=c99
SRC = main.c src/window_manager.c src/taskbar.c
OBJ = $(SRC:.c=.o)
TARGET = lxosde

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) -lX11

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean

