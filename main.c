#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>

// Function declarations
void init_taskbar(Display *dpy, Window root, int screen);
void handle_taskbar_events();
void init_window_manager(Display *display, int screen);

int main() {
    Display *display = XOpenDisplay(NULL);
    if (display == NULL) {
        fprintf(stderr, "Cannot open display\n");
        exit(1);
    }

    int screen = DefaultScreen(display);
    Window root = RootWindow(display, screen);

    // Initialize the window manager
    init_window_manager(display, screen);

    // Initialize the taskbar
    init_taskbar(display, root, screen);

    // Handle taskbar events
    handle_taskbar_events();

    XCloseDisplay(display);
    return 0;
}

