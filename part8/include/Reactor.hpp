#pragma once
#include <functional>

/**
 * @typedef reactorFunc
 * @brief Function pointer type for handling ready file descriptors.
 *
 * The reactor will call a function of this type when the associated file
 * descriptor is ready to read.
 *
 * @param fd The ready file descriptor.
 */
typedef void (*reactorFunc)(int fd);

/**
 * @brief Starts the reactor and begins its internal event loop in a new thread.
 *
 * @return A pointer to the internal reactor structure (opaque to the user).
 */
void* startReactor();

/**
 * @brief Registers a file descriptor and its handler with the reactor.
 *
 * @param reactor Pointer returned from startReactor.
 * @param fd The file descriptor to monitor.
 * @param func The function to call when the fd is ready.
 * @return 0 on success, or a negative value on error.
 */
int addFdToReactor(void* reactor, int fd, reactorFunc func);

/**
 * @brief Removes a file descriptor from the reactor.
 *
 * @param reactor Pointer returned from startReactor.
 * @param fd The file descriptor to remove.
 * @return 0 on success, or a negative value on error.
 */
int removeFdFromReactor(void* reactor, int fd);

/**
 * @brief Stops the reactor and cleans up resources.
 *
 * @param reactor Pointer returned from startReactor.
 * @return 0 on success, or a negative value on error.
 */
int stopReactor(void* reactor);