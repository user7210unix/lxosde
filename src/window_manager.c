#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void init_window_manager(Display *display, int screen) {
    Window root = RootWindow(display, screen);
    int screen_width = DisplayWidth(display, screen);
    int screen_height = DisplayHeight(display, screen);

    // Create a main window
    Window win = XCreateSimpleWindow(display, root, 10, 10, 800, 600, 1,
                                     BlackPixel(display, screen), WhitePixel(display, screen));

    // Create a title bar window
    Window title_bar = XCreateSimpleWindow(display, win, 0, 0, 800, 30, 1,
                                            BlackPixel(display, screen), WhitePixel(display, screen));

    // Select input events
    XSelectInput(display, win, ExposureMask | KeyPressMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | FocusChangeMask);
    XSelectInput(display, title_bar, ExposureMask | ButtonPressMask | ButtonMotionMask);

    // Map the windows
    XMapWindow(display, win);
    XMapWindow(display, title_bar);

    // Variables for window movement and resizing
    int moving = 0, resizing = 0;
    int x_offset, y_offset;
    Window grabbed_window = None;
    int grab_x, grab_y;

    XEvent e;
    while (1) {
        XNextEvent(display, &e);
        if (e.type == Expose && e.xexpose.window == win) {
            XFillRectangle(display, win, DefaultGC(display, screen), 20, 20, 10, 10);
        }
        if (e.type == Expose && e.xexpose.window == title_bar) {
            XFillRectangle(display, title_bar, DefaultGC(display, screen), 0, 0, 800, 30);
            XDrawString(display, title_bar, DefaultGC(display, screen), 10, 20, "Window Title", strlen("Window Title"));
            XDrawString(display, title_bar, DefaultGC(display, screen), 750, 20, "□", 1); // Maximize
            XDrawString(display, title_bar, DefaultGC(display, screen), 770, 20, "_", 1); // Minimize
            XDrawString(display, title_bar, DefaultGC(display, screen), 790, 20, "X", 1); // Close
        }
        if (e.type == ButtonPress && e.xbutton.window == title_bar) {
            if (e.xbutton.x > 750 && e.xbutton.x < 770) {
                // Minimize button
                XUnmapWindow(display, win);
            } else if (e.xbutton.x > 770 && e.xbutton.x < 790) {
                // Maximize button
                if (e.xbutton.window == title_bar) {
                    XMoveResizeWindow(display, win, 0, 0, screen_width, screen_height - 40);
                } else {
                    XMoveResizeWindow(display, win, 10, 10, 800, 600);
                }
            } else if (e.xbutton.x > 790) {
                // Close button
                XDestroyWindow(display, win);
                break;
            } else {
                // Start moving the window
                moving = 1;
                x_offset = e.xbutton.x;
                y_offset = e.xbutton.y;
                grabbed_window = win;
            }
        }
        if (e.type == ButtonPress && e.xbutton.window == win) {
            // Start resizing the window
            resizing = 1;
            grab_x = e.xbutton.x;
            grab_y = e.xbutton.y;
        }
        if (e.type == ButtonRelease) {
            // Stop moving or resizing the window
            moving = 0;
            resizing = 0;
            grabbed_window = None;
        }
        if (e.type == MotionNotify && moving && grabbed_window == win) {
            // Move the window
            int x = e.xmotion.x_root - x_offset;
            int y = e.xmotion.y_root - y_offset;
            XMoveWindow(display, win, x, y);
        }
        if (e.type == MotionNotify && resizing && grabbed_window == win) {
            // Resize the window
            int width = e.xmotion.x_root - grab_x;
            int height = e.xmotion.y_root - grab_y;
            XResizeWindow(display, win, width, height);
        }
        if (e.type == FocusIn) {
            // Handle window focus
            XSetInputFocus(display, e.xfocus.window, RevertToPointerRoot, CurrentTime);
        }
        if (e.type == KeyPress && e.xkey.window == win) {
            break;
        }
    }
}

