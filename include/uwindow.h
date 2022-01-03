/** @file */

#ifndef MW_MAIN_H
#define MW_MAIN_H

#include "error.h"
#include "window.h"


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


#endif

