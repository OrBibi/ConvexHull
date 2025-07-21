#include "../include/Proactor.hpp"
#include <pthread.h>
#include <unistd.h>
#include <iostream>

// Internal wrapper to adapt function pointer signature
struct ThreadArgs {
    int sockfd;
    proactorFunc func;
};

// Wrapper function passed to pthread_create
void* threadWrapper(void* arg) {
    ThreadArgs* args = static_cast<ThreadArgs*>(arg);
    if (args->func) {
        args->func(args->sockfd);  // Execute user-defined function
    }
    close(args->sockfd);  // Close socket when done
    delete args;
    return nullptr;
}

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
