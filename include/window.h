/** @file */

#ifndef MICROWINDOW_WINDOW_H
#define MICROWINDOW_WINDOW_H

#include "error.h"
#include <stdint.h>
#include <stdbool.h>
#include <wayland-client.h>
#include <wayland-egl.h>
#include <EGL/egl.h>
#include "xdg-shell-client-protocol.h"


/**
 * @brief Callback function for window resize events
 *
 * A pointer to a function receiving resize callback events.
 *
 * The function must have the signature `void function(int32_t new_width, int32_t new_height)`.
 *
 * @param new_width The width of the window after the resize
 * @param new_height The height of the window after the resize
 */
typedef void (*MW_Window_resize_cb)(int32_t new_width, int32_t new_height);


/**
 * @brief The main window state object
 *
 * This struct holds all data related to a window.
 * One instance of this structure should exist for every open window at a given time.
 * It is created using @ref MW_Window_create and freed using @ref MW_Window_destroy.
 *
 * @note The parameters of this structure are used internally and are subject to change.
 * The user of the library should not actually need to interface with this struct at all.
 * In case you need to access @ref MW_Window::current_width or @ref MW_Window::current_height,
 * consider calling `glGetIntegerv` with `GL_VIEWPORT` instead.
 */
typedef struct {
    /**
     * @brief The preferred width of the window as set by the user
     *
     * The value of this member is set when calling @ref MW_Window_create with a preferred width.
     */
    int32_t preferred_width;

    /**
     * @brief The preferred height of the window as set by the user
     *
     * The value of this member is set when calling @ref MW_Window_create with a preferred height.
     */
    int32_t preferred_height;

    /** @brief The width of the open window */
    int32_t current_width;

    /** @brief The height of the open window */
    int32_t current_height;


    /** @brief A pointer to an object representing a drawable surface in the wayland window manager */
    struct wl_surface *wayland_surface;

    /** @brief A pointer to an object representing a drawable surface in the xdg-wayland protocol */
    struct xdg_surface *xdg_surface;

    /** @brief A pointer to an object representing a general window manager interface */
    struct xdg_toplevel *xdg_toplevel;

    /** @brief A pointer to an object representing a window in the wayland EGL extension */
    struct wl_egl_window *egl_window;

    /** @brief A pointer to an EGL surface setup for drawing into with OpenGL */
    EGLSurface *egl_surface;

    /** @brief A pointer to an EGL drawing context */
    EGLContext *egl_context;

    /** @brief An internal variable for keeping track of window resize events */
    bool resize_needed;

    /** @brief The user-provided window resize callback (or `NULL` if none was provided)
     *
     * This member is initialised to `NULL` and set to a valid function once the user calls @ref MW_Window_register_resize_callback.
     */
    MW_Window_resize_cb resize_cb;
} MW_Window;


/**
 * @brief Initialises the μWindow library
 *
 * The main initialisation function of the μWindow library.
 * This function needs to be called before calling any other μWindow library functions.
 *
 * If the function fails nothing is initialised or allocated and the function can safely be called again.
 *
 * @returns An @ref MW_Error code:
 * - **MW_SUCCESS**: The initialisation completed successfully
 * - **MW_ALREADY_INITIALISED**: The library had already been initialised before this function call
 * - **MW_NO_DISPLAY**: Failed to initialise the wayland display
 *   **  - The host system may not have wayland installed / running
 * - **MW_NO_EGL_DISPLAY**: Failed to get an EGL display object for the wayland display
 *   **  - The host system might not have EGL installed
 * - **MW_FAILED_EGL_DISPLAY_INIT**: Failed to initialise the EGL display object
 * - **MW_NO_EGL_CONFIG**: Failed to get a suitable RGB EGL config
 * - **MW_NO_REGISTRY**: Failed to connect to the wayland registry
 * - **MW_NO_COMPOSITOR**: Failed to get a wayland compositor object from the wayland registry
 * - **MW_NO_WM_BASE**: Failed to get an xdg window manager base object from the wayland registry
 *   **  - The host system might not have a wayland window manager installed
 * - **MW_FAILED_DISPLAY_DISPATCH** and **MW_FAILED_DISPLAY_ROUNDTRIP**: Failed to process wayland events
 *
 * @see @ref MW_finish()
 */
MW_Error MW_init();


/**
 * @brief Processes all pending events
 *
 * This function processes all pending events for all windows and ensures that calls are made to the registered event handlers.
 * It should be called repeatedly in a loop to ensure that all events are processed in a timely manner.
 * This loop could either be the main frame processing loop or a seperate, slightly delayed update loop (in frame-accurate
 * events are not neccessary).
 *
 * This function processes all pending events and exits.
 * In case of no pending events being queued up, the function has no effect and simply exits.
 * For an event-driven program, where the screeen is only updated once there is an event, consider using
 * @ref MW_process_events_blocking instead.
 *
 * @returns An @ref MW_Error code:
 * - **MW_SUCCESS**: All pending events were processes successfully or there were no events to process
 * - **MW_NOT_INITIALISED**: The library has not been initialised yet
 *     - @ref MW_init() has not yet been called or the call to @ref MW_init() was unsuccessful
 * - **MW_FAILED_DISPLAY_ROUNDTRIP** and **MW_FAILED_DISPLAY_DISPATCH**: Failed to process wayland events
 * - **MW_FAILED_DISPLAY_FLUSH**: Failed to clear the event queue after processing the events
 *
 * @see @ref MW_process_events_blocking()
 */
MW_Error MW_process_events();


/**
 * @brief Waits until events are available and processes them
 *
 * This function is similar to @ref MW_process_events().
 * The major difference is that this function waits and *blocks* the calling thread until events are available.
 * It may be used in cases where framerate is not an issue and the screen only updates upon events.
 * See `examples/blocking` for an example of such a use-case.
 *
 * @returns An @ref MW_Error code:
 * - **MW_SUCCESS**: All pending events were processes successfully or there were no events to process
 * - **MW_NOT_INITIALISED**: The library has not been initialised yet
 *     - @ref MW_init() has not yet been called or the call to @ref MW_init() was unsuccessful
 * - **MW_FAILED_DISPLAY_ROUNDTRIP** and **MW_FAILED_DISPLAY_DISPATCH**: Failed to process wayland events
 *
 * @see @ref MW_process_events()
 */
MW_Error MW_process_events_blocking();


/**
 * @brief Creates an @ref MW_Window object
 *
 * This function creates an @ref MW_Window object and sets it up for OpenGL drawing.
 * After creating the window, the function automatically opens the window and selects it for drawing using
 * @ref MW_Window_make_current.
 *
 * After calling MW_Window_create, you can start using standard OpenGL functions to draw into it.
 *
 * In case of failure, the function sets the members of the @ref MW_Window structure `window` to NULL, so that another call
 * to MW_Window_create is possible.
 *
 * @param window A pointer to an allocated @ref MW_Window structure. This is usually a pointer to preallocated caller-local memory
 * @param title The title of the window, which may or may not be shown by the window manager
 * @param preferred_width A preferred width that the window should have, in case the window manager does dictate a window size\n
 *                        0 can be passed for a default window width
 * @param preferred_height A preferred height that the window should have, in case the window manager does dictate a window size\n
 *                         0 can be passed for a default window height
 *
 * @returns An @ref MW_Error code:
 * - **MW_SUCCESS**: The window was successfully created
 * - **MW_NOT_INITIALISED**: The library has not been initialised yet
 *     - @ref MW_init() has not yet been called or the call to @ref MW_init() was unsuccessful
 * - **MW_INVALID_PARAM**: An invalid parameter was passed to the function
 *     - `title` cannot be NULL
 * - **MW_FAILED_OPENGL_API_BIND**: Failed to bind EGL to an OpenGL API
 *     - The host system may not have OpenGL installed
 * - **MW_FAILED_DISPLAY_ROUNDTRIP** and **MW_FAILED_DISPLAY_DISPATCH**: Failed to process wayland events
 *
 * @note Due to this function calling @ref MW_Window_make_current on the new window, it changes the drawing context.\n
 *       In case of multi-window applications, make sure you reselect the previously active window if necessary.
 *
 * @warning The `preferred_width` and `preferred_height` are only hints given to the window manager and
 *          should not be considered the initial size of the window!
 *          Always check the window size using a call like `glGetIntegerv` with `GL_VIEWPORT` before drawing any size-dependant
 *          graphics.
 *
 * @see @ref MW_Window_destroy
 */
MW_Error MW_Window_create(MW_Window *window, const char *title, int32_t preferred_width, int32_t preferred_height);


/**
 * @brief Sets a callback for a resize event of a specific window
 *
 * This function registers a callback function for receiving window resize events.
 * After calling this function, the function specified in the parameter `callback` is called every time the
 * window `window` is resized.
 *
 * @param window The window to register the event handler function for
 * @param callback A pointer to the function which should receive the window resize events
 *
 * @returns An @ref MW_Error code:
 * - **MW_SUCCESS**: The callback was successfully registered
 * - **MW_NOT_INITIALISED**: The library has not been initialised yet
 *     - @ref MW_init() has not yet been called or the call to @ref MW_init() was unsuccessful
 * - **MW_INVALID_PARAM**: An invalid parameter was passed to the function
 *     - `callback` cannot be NULL
 * - **MW_INVALID_WINDOW_STATE**: The passed window object is in an invalid state
 */
MW_Error MW_Window_register_resize_callback(MW_Window *window, MW_Window_resize_cb callback);


/**
 * @brief Sets the specified window as the current one for OpenGL draw calls
 *
 * When building an application with multiple open windows, one may switch between the OpenGL drawing contexts using this function.
 * After calling this function, all OpenGL drawing functions now apply to the selected window.
 *
 * @param window The window to select
 *
 * @returns An @ref MW_Error code:
 * - **MW_SUCCESS**: The callback was successfully registered
 * - **MW_NOT_INITIALISED**: The library has not been initialised yet
 *     - @ref MW_init() has not yet been called or the call to @ref MW_init() was unsuccessful
 * - **MW_INVALID_WINDOW_STATE**: The passed window object is in an invalid state
 * - **MW_FAILED_TO_MAKE_EGL_CONTEXT_CURRENT**: Failed to switch the drawing context
 */
MW_Error MW_Window_make_current(MW_Window *window);


/**
 * @brief Swaps the OpenGL buffers of a given window
 *
 * μWindow uses double buffering by default and this function is used to swap the buffer that is currently draw into and the
 * buffer that is currently displayed to the user.
 * This function has to be called repeatedly in the frame update loop in order to display the contents drawn to the window.
 *
 * @param window The window to swap the buffers for
 *
 * @returns An @ref MW_Error code:
 * - **MW_SUCCESS**: The callback was successfully registered
 * - **MW_NOT_INITIALISED**: The library has not been initialised yet
 *     - @ref MW_init() has not yet been called or the call to @ref MW_init() was unsuccessful
 * - **MW_INVALID_WINDOW_STATE**: The passed window object is in an invalid state
 * - **MW_FAILED_TO_SWAP_EGL_BUFFERS**: The buffer swap failed
 */
MW_Error MW_Window_swap_buffers(MW_Window *window);


/**
 * @brief Deallocates a @ref MW_Window object
 *
 * This function destroys an @ref MW_Window object created using the @ref MW_Window_create function.
 * It has to be called for **every** created window when the window is no longer needed.
 *
 * This function cannot fail.
 *
 * @param window The window object to deallocate
 *
 * @see @ref MW_Window_create
 */
void MW_Window_destroy(MW_Window *window);


/**
 * @brief Deinitialises the μWindow library
 *
 * This function deinitialises the μWindow library.
 * It has to be called once the program does not need to call any library functions anymore.
 * After calling MW_finish(), it is safe to call @ref MW_init() again.
 *
 * This function cannot fail.
 *
 * @see @ref MW_init()
 */
void MW_finish();


#endif

