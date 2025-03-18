#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define WINDOW_MANAGER_NAME "LightWM"
#define SOCKET_PATH "/tmp/lightwm_socket"

Display *display;
Window root;
int screen;
int socket_fd;

void send_to_panel(const char *message) {
    if (socket_fd == -1) return;

    write(socket_fd, message, strlen(message));
}

void handle_events() {
    XEvent event;
    while (1) {
        XNextEvent(display, &event);

        switch (event.type) {
            case MapRequest: {
                // A window wants to be mapped (displayed)
                XMapWindow(display, event.xmaprequest.window);
                char message[128];
                snprintf(message, sizeof(message), "MAP %lu\n", event.xmaprequest.window);
                send_to_panel(message);
                printf("Mapped window: %lu\n", event.xmaprequest.window);
                break;
            }
            case DestroyNotify: {
                // A window was destroyed
                char message[128];
                snprintf(message, sizeof(message), "UNMAP %lu\n", event.xdestroywindow.window);
                send_to_panel(message);
                printf("Window destroyed: %lu\n", event.xdestroywindow.window);
                break;
            }
            case KeyPress: {
                // Handle keypress events (e.g., Alt+F4 to close a window)
                if (event.xkey.keycode == XKeysymToKeycode(display, XK_F4)) {
                    XDestroyWindow(display, event.xkey.window);
                }
                break;
            }
            default:
                break;
        }
    }
}

int main() {
    // Open connection to the X server
    display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "Cannot open display\n");
        return 1;
    }

    screen = DefaultScreen(display);
    root = RootWindow(display, screen);

    // Set the window manager name
    XStoreName(display, root, WINDOW_MANAGER_NAME);

    // Select events to listen to
    XSelectInput(display, root, SubstructureRedirectMask | SubstructureNotifyMask | KeyPressMask);

    // Grab Alt+F4 to close windows
    XGrabKey(display, XKeysymToKeycode(display, XK_F4), Mod1Mask, root, True, GrabModeAsync, GrabModeAsync);

    // Create a Unix socket for IPC
    struct sockaddr_un addr;
    socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        perror("socket");
        return 1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(socket_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("connect");
        close(socket_fd);
        socket_fd = -1;
    }

    printf("Window manager started. Use Alt+F4 to close windows.\n");

    // Enter the event loop
    handle_events();

    // Cleanup
    if (socket_fd != -1) close(socket_fd);
    XCloseDisplay(display);
    return 0;
}
