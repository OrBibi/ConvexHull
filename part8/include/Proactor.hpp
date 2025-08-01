#ifndef PROACTOR_HPP
#define PROACTOR_HPP

#include <pthread.h>

/**
 * @typedef proactorFunc
 * @brief Function pointer type for a client-handling thread.
 *
 * Each function of this type will handle a client socket connection.
 *
 * @param sockfd The client's socket file descriptor.
 * @return A void pointer (can be used to return a status or result).
 */
typedef void* (*proactorFunc)(int sockfd);

/**
 * @brief Starts a new proactor thread to handle a client connection.
 *
 * Creates a pthread that executes the given function `threadFunc` with the provided
 * socket file descriptor `sockfd`.
 *
 * @param sockfd The client socket file descriptor.
 * @param threadFunc The function to be executed by the new thread.
 * @return The thread ID of the newly created thread.
 */
pthread_t startProactor(int sockfd, proactorFunc threadFunc);

/**
 * @brief Stops a running proactor thread.
 *
 * Sends a cancellation signal to the specified pthread.
 *
 * @param tid The thread ID of the proactor thread.
 * @return 0 on success, or an error code on failure.
 */
int stopProactor(pthread_t tid);

#endif // PROACTOR_HPP
