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

typedef struct {
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
} MW_Window;

int MW_init();
int MW_process_events();
int MW_process_events_blocking();
int MW_Window_create(MW_Window *window, const char *title, int32_t preferred_width, int32_t preferred_height);
int MW_Window_register_resize_callback(MW_Window *window, MW_Window_resize_cb callback);
int MW_Window_make_current(MW_Window *window);
int MW_Window_swap_buffers(MW_Window *window);
void MW_Window_destroy(MW_Window *window);
void MW_finish();

#endif

