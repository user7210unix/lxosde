# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2
GTK_FLAGS = $(shell pkg-config --cflags --libs gtk+-3.0)
X11_FLAGS = -lX11

# Targets
TARGETS = lightwm panel

# Default target
all: $(TARGETS)

# Window Manager
lightwm: window_manager.c
	$(CC) $(CFLAGS) window_manager.c -o lightwm $(X11_FLAGS)

# Panel
panel: panel.c
	$(CC) $(CFLAGS) panel.c -o panel $(GTK_FLAGS)

# Clean build artifacts
clean:
	rm -f $(TARGETS)

# Phony targets
.PHONY: all clean
