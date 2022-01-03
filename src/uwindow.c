#include "../include/uwindow.h"

#include "internal.h"
#include <string.h>


// Zero initialise the internal library state
MW_LibState state = { 0 };


// Wayland event listeners

static void wm_base_ping(__attribute__((unused))void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial) {
    xdg_wm_base_pong(xdg_wm_base, serial);
}

static const struct xdg_wm_base_listener wm_base_listener = {
    .ping = wm_base_ping
};

static void reg_handler(__attribute__((unused))void * data, struct wl_registry *reg,
        uint32_t id, const char *interface, __attribute__((unused))uint32_t ver) {
    if (strcmp(interface, "wl_compositor") == 0) {
        state.compositor = wl_registry_bind(reg, id, &wl_compositor_interface, 1);
    } else if (strcmp(interface, "xdg_wm_base") == 0) {
        state.wm_base = wl_registry_bind(reg, id, &xdg_wm_base_interface, 1);
        xdg_wm_base_add_listener(state.wm_base, &wm_base_listener, NULL);
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


MW_Error MW_init() {
    if (state.initialised == true) {
        return MW_ALREADY_INITIALISED;
    }

    state.initialised = true;

    state.display = wl_display_connect(NULL);
    if (state.display == NULL) {
        state.initialised = false;
        return MW_NO_DISPLAY;
    }

    state.egl_display = eglGetDisplay(state.display);
    if (state.egl_display == EGL_NO_DISPLAY) {
        state.initialised = false;
        MW_finish();
        return MW_NO_EGL_DISPLAY;
    }

    if (eglInitialize(state.egl_display, NULL, NULL) == EGL_FALSE) {
        state.initialised = false;
        MW_finish();
        return MW_FAILED_EGL_DISPLAY_INIT;
    }

    const EGLint attr[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_NONE
    };
    EGLint num_config;
    if (eglChooseConfig(state.egl_display, attr, &state.egl_config, 1, &num_config) == EGL_FALSE) {
        state.initialised = false;
        MW_finish();
        return MW_NO_EGL_CONFIG;
    }

    state.registry = wl_display_get_registry(state.display);
    if (state.registry == NULL) {
        state.initialised = false;
        MW_finish();
        return MW_NO_REGISTRY;
    }

    wl_registry_add_listener(state.registry, &reg_listener, NULL);
    MW_Error status = MW_process_events_blocking();
    if (status != MW_SUCCESS) {
        state.initialised = false;
        MW_finish();
        return status;
    }

    if (state.compositor == NULL) {
        state.initialised = false;
        MW_finish();
        return MW_NO_COMPOSITOR;
    }
    if (state.wm_base == NULL) {
        state.initialised = false;
        MW_finish();
        return MW_NO_WM_BASE;
    }



    return MW_SUCCESS;
}

void MW_finish() {
    // TODO: If initialised, deinit all allocated windows

    if (state.egl_display != NULL) {
        eglTerminate(state.egl_display);
    }
    if (state.display != NULL) {
        wl_display_disconnect(state.display);
    }

    state = (const MW_LibState) { 0 };
}

MW_Error MW_process_events() {
    MW_CHECK_INITIALISED();

    if (wl_display_roundtrip(state.display) == -1) {
        return MW_FAILED_DISPLAY_ROUNDTRIP;
    }
    if (wl_display_dispatch_pending(state.display) == -1) {
        return MW_FAILED_DISPLAY_DISPATCH;
    }
    if (wl_display_flush(state.display) == -1) {
        return MW_FAILED_DISPLAY_FLUSH;
    }
    return MW_SUCCESS;
}

MW_Error MW_process_events_blocking() {
    MW_CHECK_INITIALISED();

    if (wl_display_dispatch(state.display) == -1) {
        return MW_FAILED_DISPLAY_DISPATCH;
    }
    if (wl_display_roundtrip(state.display) == -1) {
        return MW_FAILED_DISPLAY_ROUNDTRIP;
    }
    return MW_SUCCESS;
}

