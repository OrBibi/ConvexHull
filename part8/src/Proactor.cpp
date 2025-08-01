#include "../include/Proactor.hpp"
#include <pthread.h>
#include <unistd.h>
#include <iostream>

/**
 * @brief Internal structure to pass multiple arguments to the thread.
 */
struct ThreadArgs {
    int sockfd; //Socket file descriptor
    proactorFunc func;  //Function to execute in thread
};

/**
 * @brief Thread entry function that calls the user-defined function.
 *
 * This function adapts the `proactorFunc` signature to `void* (void*)` expected by `pthread_create`.
 *
 * @param arg Pointer to a `ThreadArgs` struct containing socket and function.
 * @return Always returns `nullptr`.
 */
void* threadWrapper(void* arg) {
    ThreadArgs* args = static_cast<ThreadArgs*>(arg);
    if (args->func) {
        args->func(args->sockfd);  // Execute user-defined function
    }
    close(args->sockfd);  // Close socket when done
    delete args;
    return nullptr;
}

/**
 * @brief Starts a new thread to handle a socket using the given function.
 *
 * The function runs asynchronously in its own thread.
 *
 * @param sockfd The socket file descriptor to handle.
 * @param threadFunc The function to run in the thread.
 * @return The created thread ID. If creation fails, returns a default-initialized thread ID.
 */
pthread_t startProactor(int sockfd, proactorFunc threadFunc) {
    pthread_t tid;
    ThreadArgs* args = new ThreadArgs{sockfd, threadFunc};

    if (pthread_create(&tid, nullptr, threadWrapper, args) != 0) {
        std::cerr << "Failed to create proactor thread" << std::endl;
        delete args;
        return pthread_t();  // Return default-initialized thread ID
    }

    return tid;
}

/**
 * @brief Attempts to stop a thread gracefully.
 *
 * Sends a cancellation request and joins the thread.
 *
 * @param tid The thread ID to stop.
 * @return 0 on success, -1 on failure.
 */
int stopProactor(pthread_t tid) {
    // Try to cancel the thread gracefully
    if (pthread_cancel(tid) != 0) {
        std::cerr << "Failed to cancel proactor thread" << std::endl;
        return -1;
    }
    if (pthread_join(tid, nullptr) != 0) {
        std::cerr << "Failed to join proactor thread" << std::endl;
        return -1;
    }
    return 0;
}
