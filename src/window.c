#include "../include/window.h"

#include "../include/error.h"
#include <string.h>


static struct wl_display *display = NULL;
static EGLDisplay *egl_display = NULL;
static EGLConfig egl_config = NULL;
static struct wl_registry *registry = NULL;
static struct wl_compositor *compositor = NULL;
static struct xdg_wm_base *wm_base = NULL;


// Event listeners

static void wm_base_ping(__attribute__((unused))void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial) {
    xdg_wm_base_pong(xdg_wm_base, serial);
}

static const struct xdg_wm_base_listener wm_base_listener = {
    .ping = wm_base_ping
};


static void reg_handler(__attribute__((unused))void * data, struct wl_registry *reg,
        uint32_t id, const char *interface, __attribute__((unused))uint32_t ver) {
    if (strcmp(interface, "wl_compositor") == 0) {
        compositor = wl_registry_bind(reg, id, &wl_compositor_interface, 1);
    } else if (strcmp(interface, "xdg_wm_base") == 0) {
        wm_base = wl_registry_bind(reg, id, &xdg_wm_base_interface, 1);
        xdg_wm_base_add_listener(wm_base, &wm_base_listener, NULL);
    }
}

static void reg_loss_handler(
        __attribute__((unused))void *data,
        __attribute__((unused))struct wl_registry *reg,
        __attribute__((unused))uint32_t id) {
}

static const struct wl_registry_listener reg_listener = {
    reg_handler,
    reg_loss_handler
};


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


int MW_process_events() {
    if (wl_display_roundtrip(display) == -1) {
        return MW_FAILED_DISPLAY_ROUNDTRIP;
    }
    if (wl_display_dispatch_pending(display) == -1) {
        return MW_FAILED_DISPLAY_DISPATCH;
    }
    if (wl_display_flush(display) == -1) {
        return MW_FAILED_DISPLAY_FLUSH;
    }
    return MW_SUCCESS;
}

int MW_process_events_blocking() {
    if (wl_display_dispatch(display) == -1) {
        return MW_FAILED_DISPLAY_DISPATCH;
    }
    if (wl_display_roundtrip(display) == -1) {
        return MW_FAILED_DISPLAY_ROUNDTRIP;
    }
    return MW_SUCCESS;
}

int MW_init() {
    display = wl_display_connect(NULL);
    if (display == NULL) {
        return MW_NO_DISPLAY;
    }

    egl_display = eglGetDisplay(display);
    if (egl_display == EGL_NO_DISPLAY) {
        wl_display_disconnect(display);
        return MW_NO_EGL_DISPLAY;
    }

    if (eglInitialize(egl_display, NULL, NULL) == EGL_FALSE) {
        wl_display_disconnect(display);
        return MW_FAILED_EGL_DISPLAY_INIT;
    }

    const EGLint attr[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_NONE
    };
    EGLint num_config;
    if (eglChooseConfig(egl_display, attr, &egl_config, 1, &num_config) == EGL_FALSE) {
        eglTerminate(egl_display);
        wl_display_disconnect(display);
        return MW_NO_EGL_CONFIG;
    }

    registry = wl_display_get_registry(display);
    if (registry == NULL) {
        eglTerminate(egl_display);
        wl_display_disconnect(display);
        return MW_NO_REGISTRY;
    }

    wl_registry_add_listener(registry, &reg_listener, NULL);
    MW_process_events_blocking();

    if (compositor == NULL) {
        eglTerminate(egl_display);
        wl_display_disconnect(display);
        return MW_NO_COMPOSITOR;
    }
    if (wm_base == NULL) {
        eglTerminate(egl_display);
        wl_display_disconnect(display);
        return MW_NO_WM_BASE;
    }

    return MW_SUCCESS;
}

int MW_Window_create(MW_Window *window, const char *title, int32_t preferred_width, int32_t preferred_height) {
    if (eglBindAPI(EGL_OPENGL_API) == EGL_FALSE) {
        return MW_FAILED_OPENGL_API_BIND;
    }

    window->preferred_width = preferred_width;
    window->preferred_height = preferred_height;
    window->current_width = preferred_width;
    window->current_height = preferred_height;
    window->resize_needed = false;
    window->resize_cb = NULL;

    window->egl_context = eglCreateContext(egl_display, egl_config, EGL_NO_CONTEXT, NULL);

    window->wayland_surface = wl_compositor_create_surface(compositor);

    window->xdg_surface = xdg_wm_base_get_xdg_surface(wm_base, window->wayland_surface);
    xdg_surface_add_listener(window->xdg_surface, &xdg_surface_listener, (void *) window);

    window->xdg_toplevel = xdg_surface_get_toplevel(window->xdg_surface);
    xdg_toplevel_add_listener(window->xdg_toplevel, &toplevel_listener, (void *) window);
    xdg_toplevel_set_title(window->xdg_toplevel, title);

    wl_surface_commit(window->wayland_surface);
    MW_process_events_blocking();

    window->egl_window = wl_egl_window_create(window->wayland_surface, preferred_width, preferred_height);
    window->egl_surface = eglCreateWindowSurface(egl_display, egl_config, window->egl_window, NULL);

    // One buffer swap updates the window which causes the configure handlers to be called with
    // the initial position of the window.
    // This basically ensures that the window has the correct initial size (as selected by the
    // window manager) when returned to the user.
    // It also preselects the new window, so the user does not have to call MW_Window_make_current
    // themselves. This is very useful for applications that only have a few windows.
    MW_Window_make_current(window);
    MW_Window_swap_buffers(window);
    MW_process_events_blocking();

    return MW_SUCCESS;
}

int MW_Window_register_resize_callback(MW_Window *window, MW_Window_resize_cb callback) {
    window->resize_cb = callback;
    return MW_SUCCESS;
}

int MW_Window_make_current(MW_Window *window) {
    if (eglMakeCurrent(egl_display, window->egl_surface, window->egl_surface, window->egl_context) == EGL_TRUE) {
        return MW_SUCCESS;
    } else {
        return MW_FAILED_TO_MAKE_EGL_CONTEXT_CURRENT;
    }
}

int MW_Window_swap_buffers(MW_Window *window) {
    if (eglSwapBuffers(egl_display, window->egl_surface) == EGL_TRUE) {
        return MW_SUCCESS;
    } else {
        return MW_FAILED_TO_SWAP_EGL_BUFFERS;
    }
}

void MW_Window_destroy(MW_Window *window) {
    eglDestroySurface(egl_display, window->egl_surface);
    wl_egl_window_destroy(window->egl_window);
    xdg_toplevel_destroy(window->xdg_toplevel);
    xdg_surface_destroy(window->xdg_surface);
    wl_surface_destroy(window->wayland_surface);
    eglDestroyContext(egl_display, window->egl_context);

    /* TODO
    window->wayland_surface = NULL;
    window->egl_window = NULL;
    window->egl_surface = NULL;
    window->egl_context = NULL;
    */
}

void MW_finish() {
    eglTerminate(egl_display);
    wl_display_disconnect(display);
}

