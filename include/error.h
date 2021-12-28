#ifndef MICROWINDOW_ERROR_H
#define MICROWINDOW_ERROR_H

// Error codes
#define MW_SUCCESS 0
#define MW_NO_DISPLAY 1
#define MW_NO_EGL_DISPLAY 2
#define MW_FAILED_EGL_DISPLAY_INIT 3
#define MW_NO_EGL_CONFIG 4
#define MW_NO_REGISTRY 5
#define MW_NO_COMPOSITOR 6
#define MW_NO_WM_BASE 7
#define MW_FAILED_OPENGL_API_BIND 8
#define MW_FAILED_TO_MAKE_EGL_CONTEXT_CURRENT 9
#define MW_FAILED_TO_SWAP_EGL_BUFFERS 10


const char *MW_get_error_string(int error_code);

#endif