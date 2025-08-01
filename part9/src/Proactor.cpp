#include "../include/Proactor.hpp"
#include <pthread.h>
#include <unistd.h>
#include <iostream>

/**
 * @struct ThreadArgs
 * @brief Internal structure to pass multiple arguments to the thread function.
 */
struct ThreadArgs {
    int sockfd;
    proactorFunc func;
};

/**
 * @brief Wrapper function passed to pthread_create.
 * 
 * This function unpacks arguments, calls the provided function,
 * and closes the socket after the function completes.
 * 
 * @param arg Pointer to ThreadArgs struct.
 * @return nullptr when thread ends.
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
 * @brief Starts a new thread (Proactor) to handle the client connection.
 * 
 * @param sockfd Client socket file descriptor.
 * @param threadFunc Function to be executed in the thread.
 * @return pthread_t ID of the created thread, or default-initialized on failure.
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
 * @brief Stops a running Proactor thread by cancelling and joining it.
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
