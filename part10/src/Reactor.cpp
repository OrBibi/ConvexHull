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
 * @brief Holds the state for the Reactor pattern implementation.
 *
 * Manages file descriptors and their associated callback handlers,
 * as well as threading and synchronization primitives.
 */
struct Reactor {
    std::unordered_map<int, reactorFunc> handlers;
    std::mutex lock;
    std::atomic<bool> running;
    std::thread loopThread;
};

/**
 * @brief The main reactor loop that monitors file descriptors using select().
 *
 * Waits for I/O readiness on registered file descriptors and invokes
 * their corresponding handler functions.
 *
 * @param reactor Pointer to the Reactor instance.
 */
static void reactorLoop(Reactor* reactor) {
    while (reactor->running) {
        fd_set readfds;
        FD_ZERO(&readfds);
        int maxfd = -1;

        {
            std::lock_guard<std::mutex> guard(reactor->lock);
            for (const auto& pair : reactor->handlers) {
                FD_SET(pair.first, &readfds);
                if (pair.first > maxfd) maxfd = pair.first;
            }
        }

        if (maxfd == -1) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        struct timeval tv = {1, 0};
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
 * @brief Starts the reactor by initializing a new Reactor instance and launching the event loop thread.
 *
 * @return A pointer to the newly created Reactor instance.
 */
void* startReactor() {
    Reactor* reactor = new Reactor;
    reactor->running = true;
    reactor->loopThread = std::thread(reactorLoop, reactor);
    return reactor;
}

/**
 * @brief Registers a file descriptor and its handler with the reactor.
 *
 * @param reactorPtr Pointer to the Reactor instance.
 * @param fd The file descriptor to monitor.
 * @param func The callback function to invoke when the fd is ready.
 * @return 0 on success.
 */
int addFdToReactor(void* reactorPtr, int fd, reactorFunc func) {
    Reactor* reactor = static_cast<Reactor*>(reactorPtr);
    std::lock_guard<std::mutex> guard(reactor->lock);
    reactor->handlers[fd] = func;
    return 0;
}

/**
 * @brief Unregisters a file descriptor from the reactor.
 *
 * @param reactorPtr Pointer to the Reactor instance.
 * @param fd The file descriptor to remove.
 * @return 0 on success.
 */
int removeFdFromReactor(void* reactorPtr, int fd) {
    Reactor* reactor = static_cast<Reactor*>(reactorPtr);
    std::lock_guard<std::mutex> guard(reactor->lock);
    reactor->handlers.erase(fd);
    return 0;
}

/**
 * @brief Stops the reactor event loop and cleans up resources.
 *
 * @param reactorPtr Pointer to the Reactor instance.
 * @return 0 on success.
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
