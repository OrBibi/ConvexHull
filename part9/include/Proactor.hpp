#ifndef PROACTOR_HPP
#define PROACTOR_HPP

#include <pthread.h>

/**
 * @typedef proactorFunc
 * @brief A function pointer type for a client-handling thread function.
 *
 * This function will be executed in a new thread to handle the client with the given socket.
 * @param sockfd The socket file descriptor associated with the client.
 * @return A void pointer (typically used to return thread results).
 */
typedef void* (*proactorFunc)(int sockfd);

/**
 * @brief Starts a new proactor (thread) to handle a client connection.
 *
 * Creates a new thread that will execute the provided function on the given socket.
 *
 * @param sockfd The socket file descriptor of the client.
 * @param threadFunc The function to run in the new thread.
 * @return The thread ID (pthread_t) of the created thread.
 */
pthread_t startProactor(int sockfd, proactorFunc threadFunc);


/**
 * @brief Stops a proactor thread.
 *
 * Cancels and joins the specified thread to stop it cleanly.
 *
 * @param tid The thread ID to stop.
 * @return 0 on success, -1 on failure.
 */
int stopProactor(pthread_t tid);

#endif 
