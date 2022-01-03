#include "../include/uwindow.h"
#include "internal.h"

#define DEFAULT_PREFERRED_WIDTH 800
#define DEFAULT_PREFERRED_HEIGHT 600


// Wayland xdg event listeners

static void xdg_toplevel_configure(void *data, __attribute__((unused))struct xdg_toplevel *xdg_toplevel,
        int32_t width, int32_t height, __attribute__((unused))struct wl_array *states) {
    MW_Window *window = (MW_Window *) data;

    if (width == 0) {
        width = window->preferred_width;
    }
    if (height == 0) {
        height = window->preferred_height;
    }

    if (width != window->current_width || height != window->current_height) {
        window->current_width = width;
        window->current_height = height;
        window->resize_needed = true;
    }
}

static const struct xdg_toplevel_listener toplevel_listener = {
    .configure = xdg_toplevel_configure
};

static void xdg_surface_configure(void *data, struct xdg_surface *xdg_surface, uint32_t serial) {
    MW_Window *window = (MW_Window *) data;

    if (window->resize_needed) {
        wl_egl_window_resize(window->egl_window, window->current_width, window->current_height, 0, 0);
        window->resize_needed = false;
        if (window->resize_cb != NULL) {
            window->resize_cb(window->current_width, window->current_height);
        }
    }

    xdg_surface_ack_configure(xdg_surface, serial);
}

static const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_configure
};


MW_Error MW_Window_create(MW_Window *window, const char *title, int32_t preferred_width, int32_t preferred_height) {
    MW_CHECK_INITIALISED();
    MW_CHECK_NONNULL(title);

    if (preferred_width == 0) {
        preferred_width = DEFAULT_PREFERRED_WIDTH;
    }
    if (preferred_height == 0) {
        preferred_height = DEFAULT_PREFERRED_HEIGHT;
    }

    *window = (const MW_Window) { 0 };

    if (eglBindAPI(EGL_OPENGL_API) == EGL_FALSE) {
        MW_Window_destroy(window);
        return MW_FAILED_OPENGL_API_BIND;
    }

    window->preferred_width = preferred_width;
    window->preferred_height = preferred_height;
    window->current_width = preferred_width;
    window->current_height = preferred_height;
    window->resize_needed = false;
    window->resize_cb = NULL;

    window->egl_context = eglCreateContext(state.egl_display, state.egl_config, EGL_NO_CONTEXT, NULL);

    window->wayland_surface = wl_compositor_create_surface(state.compositor);

    window->xdg_surface = xdg_wm_base_get_xdg_surface(state.wm_base, window->wayland_surface);
    xdg_surface_add_listener(window->xdg_surface, &xdg_surface_listener, (void *) window);

    window->xdg_toplevel = xdg_surface_get_toplevel(window->xdg_surface);
    xdg_toplevel_add_listener(window->xdg_toplevel, &toplevel_listener, (void *) window);
    xdg_toplevel_set_title(window->xdg_toplevel, title);

    wl_surface_commit(window->wayland_surface);
    MW_Error status = MW_process_events_blocking();
    if (status != MW_SUCCESS) {
        MW_Window_destroy(window);
        return status;
    }

    window->egl_window = wl_egl_window_create(window->wayland_surface, preferred_width, preferred_height);
    window->egl_surface = eglCreateWindowSurface(state.egl_display, state.egl_config, window->egl_window, NULL);

    // One buffer swap updates the window which causes the configure handlers to be called with
    // the initial position of the window.
    // This basically ensures that the window has the correct initial size (as selected by the
    // window manager) when returned to the user.
    // It also preselects the new window, so the user does not have to call MW_Window_make_current
    // themselves. This is very useful for applications that only have a few windows.
    MW_Window_make_current(window);
    MW_Window_swap_buffers(window);
    status = MW_process_events_blocking();
    if (status != MW_SUCCESS) {
        MW_Window_destroy(window);
        return status;
    }

    return MW_SUCCESS;
}

MW_Error MW_Window_set_resize_callback(MW_Window *window, MW_Window_resize_cb callback) {
    MW_CHECK_INITIALISED();
    MW_CHECK_WINDOW_NONNULL(window);
    window->resize_cb = callback;
    return MW_SUCCESS;
}

MW_Error MW_Window_make_current(MW_Window *window) {
    MW_CHECK_INITIALISED();
    MW_CHECK_WINDOW_NONNULL(window);
    if (eglMakeCurrent(state.egl_display, window->egl_surface, window->egl_surface, window->egl_context) == EGL_TRUE) {
        return MW_SUCCESS;
    } else {
        return MW_FAILED_TO_MAKE_EGL_CONTEXT_CURRENT;
    }
}

MW_Error MW_Window_swap_buffers(MW_Window *window) {
    MW_CHECK_INITIALISED();
    MW_CHECK_WINDOW_NONNULL(window);
    if (eglSwapBuffers(state.egl_display, window->egl_surface) == EGL_TRUE) {
        return MW_SUCCESS;
    } else {
        return MW_FAILED_TO_SWAP_EGL_BUFFERS;
    }
}

MW_Error MW_Window_set_fullscreen(MW_Window *window, bool fullscreen) {
    MW_CHECK_INITIALISED();
    MW_CHECK_WINDOW_NONNULL(window);
    if (fullscreen) {
        xdg_toplevel_set_fullscreen(window->xdg_toplevel, NULL);
    } else {
        xdg_toplevel_unset_fullscreen(window->xdg_toplevel);
    }
    return MW_SUCCESS;
}

void MW_Window_destroy(MW_Window *window) {
    if (state.egl_display != NULL && window->egl_surface != NULL) {
        eglDestroySurface(state.egl_display, window->egl_surface);
        window->egl_surface = NULL;
    }
    if (window->egl_window != NULL) {
        wl_egl_window_destroy(window->egl_window);
        window->egl_window = NULL;
    }
    if (window->xdg_toplevel != NULL) {
        xdg_toplevel_destroy(window->xdg_toplevel);
        window->xdg_toplevel = NULL;
    }
    if (window->xdg_surface != NULL) {
        xdg_surface_destroy(window->xdg_surface);
        window->xdg_surface = NULL;
    }
    if (window->wayland_surface != NULL) {
        wl_surface_destroy(window->wayland_surface);
        window->wayland_surface = NULL;
    }
    if (state.egl_display != NULL && window->egl_context != NULL) {
        eglDestroyContext(state.egl_display, window->egl_context);
        window->egl_context = NULL;
    }

    window->preferred_width = 0;
    window->preferred_height = 0;
    window->resize_needed = false;
    window->current_width = 0;
    window->current_height = 0;
    window->resize_cb = NULL;
}

