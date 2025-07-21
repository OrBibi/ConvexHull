#ifndef PROACTOR_HPP
#define PROACTOR_HPP

#include <pthread.h>

// Define the function pointer type for client handling thread
typedef void* (*proactorFunc)(int sockfd);

// Starts a new proactor (thread) to handle the client with given sockfd
pthread_t startProactor(int sockfd, proactorFunc threadFunc);

// Stops the proactor thread given its thread ID
int stopProactor(pthread_t tid);

#endif // PROACTOR_HPP
