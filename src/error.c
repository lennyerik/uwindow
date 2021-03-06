#include "../include/error.h"

const char *MW_get_error_string(MW_Error error_code) {
    switch (error_code) {
        case MW_SUCCESS:
            return "The operation completed successfully";
        case MW_INVALID_PARAM:
            return "An invalid parameter was passed to the function";
        case MW_INVALID_WINDOW_STATE:
            return "The window object is in an invalid state";
        case MW_NOT_INITIALISED:
            return "uWindow has not been initialised, please call MW_init() first";
        case MW_ALREADY_INITIALISED:
            return "The uWindow library has already been initialised, please make sure you don't call MW_init() multiple times";
        case MW_NO_DISPLAY:
            return "Failed to connect to wayland display";
        case MW_NO_EGL_DISPLAY:
            return "Failed to get EGL display";
        case MW_FAILED_EGL_DISPLAY_INIT:
            return "Failed to initialise EGL display";
        case MW_NO_EGL_CONFIG:
            return "Failed to get a suitable RGB EGL config";
        case MW_NO_REGISTRY:
            return "Failed to get wayland registry";
        case MW_NO_COMPOSITOR:
            return "Failed to get wayland compositor";
        case MW_NO_WM_BASE:
            return "Failed to get xdg_wm_base from wayland registry";
        case MW_FAILED_OPENGL_API_BIND:
            return "Failed to bind thread to the OpenGL EGL API";
        case MW_FAILED_TO_MAKE_EGL_CONTEXT_CURRENT:
            return "Failed to set the EGL Context as the active one";
        case MW_FAILED_TO_SWAP_EGL_BUFFERS:
            return "Failed to swap EGL buffers";
        case MW_FAILED_DISPLAY_ROUNDTRIP:
            return "Failed to wait for the wayland server to process pending requests";
        case MW_FAILED_DISPLAY_DISPATCH:
            return "Failed to dispatch events from wayland server event queue";
        case MW_FAILED_DISPLAY_FLUSH:
            return "Failed to flush display event queue";
        default:
            return "An unknown error occurred";
    }
}

