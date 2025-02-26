#define _DEFAULT_SOURCE

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <X11/XKBlib.h>
#include <string.h>
#include <dirent.h>

Display *display;
Window taskbar_win;
Window start_menu_win;
Window managed_windows[10];
int window_count = 0;
int screen_width, screen_height;

void init_taskbar(Display *dpy, Window root, int screen) {
    display = dpy;
    screen_width = DisplayWidth(display, screen);
    screen_height = DisplayHeight(display, screen);

    // Create a window for the taskbar at the bottom
    taskbar_win = XCreateSimpleWindow(display, root, 0, screen_height - 40, screen_width, 40, 1,
                                      BlackPixel(display, screen), WhitePixel(display, screen));

    // Select events for the taskbar window
    XSelectInput(display, taskbar_win, ExposureMask | ButtonPressMask | KeyPressMask);

    // Map the taskbar window to the display
    XMapWindow(display, taskbar_win);
    XMapRaised(display, taskbar_win);

    // Create a window for the start menu
    start_menu_win = XCreateSimpleWindow(display, root, 0, screen_height - 340, 200, 300, 1,
                                         BlackPixel(display, screen), WhitePixel(display, screen));

    // Select events for the start menu window
    XSelectInput(display, start_menu_win, ExposureMask | ButtonPressMask);

    // Create desktop icons
    DIR *dir;
    struct dirent *entry;
    dir = opendir(getenv("HOME"));
    if (dir) {
        int x = 10, y = 60; // Starting position for icons
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_DIR || entry->d_type == DT_REG) {
                Window icon = XCreateSimpleWindow(display, root, x, y, 64, 64, 1,
                                                 BlackPixel(display, screen), WhitePixel(display, screen));
                XSelectInput(display, icon, ExposureMask | ButtonPressMask);
                XMapWindow(display, icon);
                managed_windows[window_count++] = icon;
                y += 74; // Move to the next row
                if (y > screen_height - 100) { // Wrap to the next column
                    y = 60;
                    x += 74;
                }
            }
        }
        closedir(dir);
    }
}

void launch_application(const char *command) {
    if (command == NULL) {
        return;
    }

    if (fork() == 0) {
        // Child process
        execl("/usr/bin/pcmanfm", "pcmanfm", NULL);
        perror("execl");
        exit(EXIT_FAILURE);
    }
}

void draw_start_menu() {
    XClearWindow(display, start_menu_win);
    XDrawString(display, start_menu_win, DefaultGC(display, DefaultScreen(display)), 10, 30, "Start Menu", strlen("Start Menu"));

    // Scan for .desktop files in common directories
    const char *app_dirs[] = {"/usr/share/applications/", "/usr/local/share/applications/", NULL};
    for (int i = 0; app_dirs[i] != NULL; i++) {
        DIR *dir = opendir(app_dirs[i]);
        if (dir) {
            struct dirent *entry;
            int y = 50;
            while ((entry = readdir(dir)) != NULL) {
                if (strstr(entry->d_name, ".desktop") != NULL) {
                    XDrawString(display, start_menu_win, DefaultGC(display, DefaultScreen(display)), 20, y, entry->d_name, strlen(entry->d_name));
                    y += 20;
                }
            }
            closedir(dir);
        }
    }
}

void handle_taskbar_events() {
    XEvent e;
    KeySym key;
    while (1) {
        XNextEvent(display, &e);
        if (e.type == Expose && e.xexpose.window == taskbar_win) {
            // Draw the taskbar
            XFillRectangle(display, taskbar_win, DefaultGC(display, DefaultScreen(display)), 0, 0, screen_width, 40);
            XDrawString(display, taskbar_win, DefaultGC(display, DefaultScreen(display)), 10, 20, "Start", strlen("Start"));
        }
        if (e.type == ButtonPress && e.xbutton.window == taskbar_win) {
            // Handle button press events on the taskbar
            if (e.xbutton.x < 50 && e.xbutton.y < 40) {
                // Clicked the "Start" button
                printf("Start button clicked\n"); // Debug statement
                XMapWindow(display, start_menu_win);
                XRaiseWindow(display, start_menu_win);
                draw_start_menu();
            }
        }
        if (e.type == KeyPress && e.xkey.window == taskbar_win) {
            // Handle key press events on the taskbar
            char buffer[10];
            XLookupString(&e.xkey, buffer, sizeof(buffer), &key, NULL);
            if (key == XK_Super_L || key == XK_Super_R) {
                // Windows key: Show start menu
                printf("Windows key pressed\n"); // Debug statement
                XMapWindow(display, start_menu_win);
                XRaiseWindow(display, start_menu_win);
                draw_start_menu();
            }
        }
        if (e.type == Expose && e.xexpose.window == start_menu_win) {
            // Draw the start menu
            draw_start_menu();
        }
        if (e.type == ButtonPress && e.xbutton.window == start_menu_win) {
            // Handle button press events on the start menu
            if (e.xbutton.button == Button1) {
                // Left-click: Launch pcmanfm
                launch_application("pcmanfm");
                XUnmapWindow(display, start_menu_win);
            }
        }
        if (e.type == ButtonPress && e.xbutton.window != taskbar_win && e.xbutton.window != start_menu_win) {
            // Handle desktop icon clicks
            launch_application("pcmanfm");
        }
        if (e.type == MapNotify) {
            // Add newly mapped windows to the managed list
            managed_windows[window_count++] = e.xmap.window;
        }
    }
}

