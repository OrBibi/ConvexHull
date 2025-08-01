#pragma once
#include <functional>

/**
 * @typedef reactorFunc
 * @brief Type definition for a callback function associated with a file descriptor.
 *
 * This function is called when the associated file descriptor is ready (e.g., readable).
 *
 * @param fd The file descriptor that triggered the event.
 */
typedef void (*reactorFunc)(int fd);

/**
 * @brief Starts the reactor loop in a background thread.
 *
 * Initializes a reactor instance and launches its event loop.
 *
 * @return A pointer to the internal reactor instance, or nullptr on failure.
 */
void* startReactor();

/**
 * @brief Registers a file descriptor and its callback with the reactor.
 *
 * When the given FD is ready, the provided callback function will be invoked.
 *
 * @param reactor A pointer to the reactor instance returned by `startReactor()`.
 * @param fd The file descriptor to monitor.
 * @param func The function to call when the FD is ready.
 * @return 0 on success, -1 on error.
 */
int addFdToReactor(void* reactor, int fd, reactorFunc func);

/**
 * @brief Removes a file descriptor from the reactor.
 *
 * Stops monitoring the given FD and removes its callback.
 *
 * @param reactor A pointer to the reactor instance.
 * @param fd The file descriptor to remove.
 * @return 0 on success, -1 on error.
 */
int removeFdFromReactor(void* reactor, int fd);

/**
 * @brief Stops the reactor loop and deallocates the reactor instance.
 *
 * Safely stops the background thread and frees associated resources.
 *
 * @param reactor A pointer to the reactor instance.
 * @return 0 on success, -1 on error.
 */
int stopReactor(void* reactor);
