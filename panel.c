#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define SOCKET_PATH "/tmp/lightwm_socket"

GtkWidget *taskbar_box;
int socket_fd;

void update_taskbar(const char *message) {
    // Parse the message (e.g., "MAP <window_id>" or "UNMAP <window_id>")
    unsigned long window_id;
    char action[16];
    sscanf(message, "%s %lu", action, &window_id);

    if (strcmp(action, "MAP") == 0) {
        // Add a button for the new window
        char button_label[32];
        snprintf(button_label, sizeof(button_label), "Window %lu", window_id);
        GtkWidget *button = gtk_button_new_with_label(button_label);
        gtk_box_pack_start(GTK_BOX(taskbar_box), button, FALSE, FALSE, 0);
        gtk_widget_show(button);
    } else if (strcmp(action, "UNMAP") == 0) {
        // Remove the button for the closed window
        GList *children = gtk_container_get_children(GTK_CONTAINER(taskbar_box));
        for (GList *iter = children; iter != NULL; iter = iter->next) {
            GtkWidget *button = GTK_WIDGET(iter->data);
            if (strstr(gtk_button_get_label(GTK_BUTTON(button)), "Window") != NULL) {
                gtk_widget_destroy(button);
                break;
            }
        }
        g_list_free(children);
    }
}

gboolean read_socket(gpointer data) {
    char buffer[128];
    ssize_t count = read(socket_fd, buffer, sizeof(buffer) - 1);
    if (count > 0) {
        buffer[count] = '\0';
        update_taskbar(buffer);
    }
    return TRUE; // Continue calling this function
}

gboolean update_clock(gpointer data) {
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%H:%M:%S", tm_now);
    gtk_label_set_text(GTK_LABEL(data), time_str);
    return TRUE;
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    // Create the main window (panel)
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Panel");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 50);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_container_set_border_width(GTK_CONTAINER(window), 5);

    // Create a horizontal box to hold the taskbar and clock
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_container_add(GTK_CONTAINER(window), hbox);

    // Add a taskbar
    taskbar_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(hbox), taskbar_box, TRUE, TRUE, 0);

    // Add a clock
    GtkWidget *clock_label = gtk_label_new("");
    gtk_box_pack_end(GTK_BOX(hbox), clock_label, FALSE, FALSE, 0);

    // Update the clock every second
    g_timeout_add_seconds(1, update_clock, clock_label);

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
        return 1;
    }

    // Read from the socket in the background
    g_timeout_add(100, read_socket, NULL);

    // Show the window
    gtk_widget_show_all(window);

    // Start the GTK main loop
    gtk_main();

    // Cleanup
    close(socket_fd);
    return 0;
}
