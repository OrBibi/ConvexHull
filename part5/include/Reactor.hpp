#pragma once
#include <functional>

/**
 * @typedef reactorFunc
 * @brief Function pointer type for file descriptor callbacks.
 *
 * Each callback function receives the file descriptor (`fd`) that triggered the event.
 */
typedef void (*reactorFunc)(int fd);

/**
 * @brief Starts the reactor loop in a separate thread or event context.
 *
 * @return A pointer to the reactor object (opaque to user). Returns nullptr on failure.
 */
void* startReactor();

/**
 * @brief Registers a file descriptor and associated callback with the reactor.
 *
 * When the file descriptor becomes ready, the provided callback will be called.
 *
 * @param reactor The reactor instance returned by `startReactor`.
 * @param fd The file descriptor to monitor.
 * @param func The callback function to invoke when the fd is ready.
 * @return 0 on success, or -1 on failure.
 */
int addFdToReactor(void* reactor, int fd, reactorFunc func);

/**
 * @brief Removes a file descriptor from the reactor.
 *
 * @param reactor The reactor instance.
 * @param fd The file descriptor to remove.
 * @return 0 on success, or -1 on failure.
 */
int removeFdFromReactor(void* reactor, int fd);

/**
 * @brief Stops the reactor loop and frees any associated resources.
 *
 * @param reactor The reactor instance to stop.
 * @return 0 on success, or -1 on failure.
 */
int stopReactor(void* reactor);
