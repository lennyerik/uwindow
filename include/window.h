#ifndef MICROWINDOW_WINDOW_H
#define MICROWINDOW_WINDOW_H

#include "xdg-shell-client-protocol.h"
#include <stdint.h>
#include <stdbool.h>
#include <wayland-client.h>
#include <wayland-egl.h>
#include <EGL/egl.h>

// Callback for window resize events
typedef void (*MW_Window_resize_cb)(int32_t new_width, int32_t new_height);

struct MW_Window {
    int32_t preferred_width;
    int32_t preferred_height;

    struct wl_surface *wayland_surface;
    struct xdg_surface *xdg_surface;
    struct xdg_toplevel *xdg_toplevel;
    struct wl_egl_window *egl_window;
    EGLSurface *egl_surface;
    EGLContext *egl_context;

    bool resize_needed;
    int32_t current_width;
    int32_t current_height;
    MW_Window_resize_cb resize_cb;
};

int MW_init();
int MW_Window_create(struct MW_Window *window, const char *title, int32_t preferred_width, int32_t preferred_height);
int MW_Window_register_resize_callback(struct MW_Window *window, MW_Window_resize_cb callback);
int MW_Window_make_current(struct MW_Window *window);
int MW_Window_swap_buffers(struct MW_Window *window);
int MW_Window_process_events(struct MW_Window *window);
void MW_Window_destroy(struct MW_Window *window);
void MW_finish();

#endif

