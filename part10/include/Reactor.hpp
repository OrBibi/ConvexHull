#pragma once
#include <functional>

/**
 * @brief Type alias for the callback function used by the reactor.
 * 
 * Each function receives a file descriptor (fd) that is ready for I/O.
 */
typedef void (*reactorFunc)(int fd);

/**
 * @brief Starts the reactor loop in a new thread.
 * 
 * The reactor monitors file descriptors and calls the appropriate callback when ready.
 * 
 * @return A pointer to the reactor instance (opaque void*).
 */
void* startReactor();

/**
 * @brief Adds a file descriptor to the reactor with its associated callback.
 * 
 * @param reactor Pointer to the reactor instance.
 * @param fd File descriptor to monitor.
 * @param func Callback function to call when fd is ready.
 * @return 0 on success, non-zero on failure.
 */
int addFdToReactor(void* reactor, int fd, reactorFunc func);

/**
 * @brief Removes a file descriptor from the reactor.
 * 
 * @param reactor Pointer to the reactor instance.
 * @param fd File descriptor to remove.
 * @return 0 on success, non-zero on failure.
 */
int removeFdFromReactor(void* reactor, int fd);

/**
 * @brief Stops the reactor and frees its resources.
 * 
 * @param reactor Pointer to the reactor instance.
 * @return 0 on success, non-zero on failure.
 */
int stopReactor(void* reactor);
