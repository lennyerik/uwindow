#ifndef MW_INTERNAL_H
#define MW_INTERNAL_H

#include <stdbool.h>
#include <wayland-client.h>
#include <EGL/egl.h>
#include "xdg-shell-client-protocol.h"

// An internal struct to hold the libraries state across translation units
typedef struct {
    bool initialised;
    struct wl_display *display;
    EGLDisplay *egl_display;
    EGLConfig egl_config;
    struct wl_registry *registry;
    struct wl_compositor *compositor;
    struct xdg_wm_base *wm_base;
} MW_LibState;

// A singleton global object that will be initalised in uwindow.c and used by uwindow.c and window.c
extern MW_LibState state;

// Internal compiler macros for function argument checking
#define MW_CHECK_NONNULL(p) if (p == NULL) return MW_INVALID_PARAM;
#define MW_CHECK_WINDOW_NONNULL(w) if ( \
        w->wayland_surface == NULL || \
        w->xdg_surface == NULL || \
        w->xdg_toplevel == NULL || \
        w->egl_window == NULL || \
        w->egl_surface == NULL || \
        w->egl_context == NULL) return MW_INVALID_WINDOW_STATE;
#define MW_CHECK_INITIALISED() if (!state.initialised) return MW_NOT_INITIALISED;

#endif

