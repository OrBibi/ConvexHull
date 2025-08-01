#include "../include/Reactor.hpp"
#include <unordered_map>
#include <sys/select.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>

/**
 * @struct Reactor
 * @brief Internal data structure for the reactor loop.
 */
struct Reactor {
    std::unordered_map<int, reactorFunc> handlers;
    std::mutex lock;
    std::atomic<bool> running;
    std::thread loopThread;
};

/**
 * @brief The internal event loop that waits for I/O and dispatches handlers.
 * 
 * @param reactor Pointer to the Reactor instance.
 */
static void reactorLoop(Reactor* reactor) {
    while (reactor->running) {
        fd_set readfds;
        FD_ZERO(&readfds);
        int maxfd = -1;

        // Collect all active file descriptors
        {
            std::lock_guard<std::mutex> guard(reactor->lock);
            for (const auto& pair : reactor->handlers) {
                FD_SET(pair.first, &readfds);
                if (pair.first > maxfd) maxfd = pair.first;
            }
        }
        // No FDs to wait on, sleep briefly
        if (maxfd == -1) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        // Wait for activity on any file descriptor
        struct timeval tv = {1, 0}; // Timeout: 1 second
        int ready = select(maxfd + 1, &readfds, nullptr, nullptr, &tv);

        if (ready > 0) {
            std::vector<int> ready_fds;
            {
                std::lock_guard<std::mutex> guard(reactor->lock);
                for (const auto& pair : reactor->handlers) {
                    if (FD_ISSET(pair.first, &readfds)) {
                        ready_fds.push_back(pair.first);
                    }
                }
            }

            // Call handlers for all ready file descriptors
            for (int fd : ready_fds) {
                reactorFunc func;
                {
                    std::lock_guard<std::mutex> guard(reactor->lock);
                    auto it = reactor->handlers.find(fd);
                    if (it != reactor->handlers.end()) {
                        func = it->second;
                    } else {
                        continue;
                    }
                }
                func(fd);
            }
        }
    }
}

/**
 * @brief Starts the reactor by launching the internal event loop thread.
 * 
 * @return Pointer to the newly created Reactor instance
 */
void* startReactor() {
    Reactor* reactor = new Reactor;
    reactor->running = true;
    reactor->loopThread = std::thread(reactorLoop, reactor);
    return reactor;
}

/**
 * @brief Registers a new file descriptor and its callback handler to the reactor.
 * 
 * @param reactorPtr Pointer to the Reactor instance
 * @param fd File descriptor to monitor
 * @param func Callback function to call when fd is ready
 * @return int Always returns 0
 */
int addFdToReactor(void* reactorPtr, int fd, reactorFunc func) {
    Reactor* reactor = static_cast<Reactor*>(reactorPtr);
    std::lock_guard<std::mutex> guard(reactor->lock);
    reactor->handlers[fd] = func;
    return 0;
}

/**
 * @brief Removes a file descriptor from the reactor.
 * 
 * @param reactorPtr Pointer to the Reactor instance
 * @param fd File descriptor to remove
 * @return int Always returns 0
 */
int removeFdFromReactor(void* reactorPtr, int fd) {
    Reactor* reactor = static_cast<Reactor*>(reactorPtr);
    std::lock_guard<std::mutex> guard(reactor->lock);
    reactor->handlers.erase(fd);
    return 0;
}

/**
 * @brief Stops the reactor and cleans up resources.
 * 
 * @param reactorPtr Pointer to the Reactor instance
 * @return int 0 on success
 */
int stopReactor(void* reactorPtr) {
    Reactor* reactor = static_cast<Reactor*>(reactorPtr);
    reactor->running = false;
    if (reactor->loopThread.joinable()) {
        reactor->loopThread.join();
    }
    delete reactor;
    return 0;
}
