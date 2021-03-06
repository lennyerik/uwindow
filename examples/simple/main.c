#include <stdio.h>
#include <stdbool.h>
#include <GL/gl.h>
#include "../../include/uwindow.h"

#include <unistd.h>


MW_Window wind;


bool check_error(int status) {
    if (status != MW_SUCCESS) {
        fprintf(stderr, "An error occurred: %s\n", MW_get_error_string(status));
        return true;
    }
    return false;
}

void draw() {
    glClearColor(1.0, 0.4, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    MW_Window_swap_buffers(&wind);
}

void window_resize_cb(__attribute__((unused))int32_t w, __attribute__((unused))int32_t h) {
    draw();
}

int main() {
    if (check_error(MW_init())) {
        return 1;
    }

    if (check_error(MW_Window_create(&wind, "UWINDOW TEST", 800, 600))) {
        MW_finish();
        return 1;
    }

    MW_Window_set_resize_callback(&wind, window_resize_cb);

    draw();

    for (int i = 0; i < 5; i++) {
        MW_process_events_blocking();
    }

    MW_Window_destroy(&wind);
    MW_finish();

    return 0;
}

