/** @file */

#ifndef MICROWINDOW_ERROR_H
#define MICROWINDOW_ERROR_H


/** @brief An generic error type enum for all library functions
 *
 * This enum specifies all return codes that library functions may return.
 * It is used as a return value by almost all μWindow functions.
 *
 * To get a human-readable error message from an MW_Error code, call @ref MW_get_error_string
 */
typedef enum {
    MW_SUCCESS = 0,
    MW_INVALID_PARAM,
    MW_INVALID_WINDOW_STATE,
    MW_NOT_INITIALISED,
    MW_ALREADY_INITIALISED,
    MW_NO_DISPLAY,
    MW_NO_EGL_DISPLAY,
    MW_FAILED_EGL_DISPLAY_INIT,
    MW_NO_EGL_CONFIG,
    MW_NO_REGISTRY,
    MW_NO_COMPOSITOR,
    MW_NO_WM_BASE,
    MW_FAILED_OPENGL_API_BIND,
    MW_FAILED_TO_MAKE_EGL_CONTEXT_CURRENT,
    MW_FAILED_TO_SWAP_EGL_BUFFERS,
    MW_FAILED_DISPLAY_ROUNDTRIP,
    MW_FAILED_DISPLAY_DISPATCH,
    MW_FAILED_DISPLAY_FLUSH
} MW_Error;


/**
 * @brief Return a human-readable error message for an @ref MW_Error code
 *
 * This function cannot fail.
 *
 * @param error_code The error code to return the human-readable string for
 *
 * @returns A null-terminated string containing a human-readable error string
 */
const char *MW_get_error_string(MW_Error error_code);

#endif
