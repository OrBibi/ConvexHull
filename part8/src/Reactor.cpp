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
 * @brief Represents the state of the reactor pattern, managing file descriptors and their associated callbacks.
 */
struct Reactor {
    std::unordered_map<int, reactorFunc> handlers;
    std::mutex lock;
    std::atomic<bool> running;
    std::thread loopThread;
};

/**
 * @brief Internal loop that waits for I/O events and calls appropriate handler functions.
 * 
 * This function continuously monitors the registered file descriptors using `select()`.
 * When a file descriptor becomes ready, it invokes the associated callback function.
 *
 * @param reactor Pointer to the reactor instance.
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
 * @brief Starts a new reactor and begins the event loop in a separate thread.
 * 
 * @return Pointer to the newly allocated reactor instance.
 */
void* startReactor() {
    Reactor* reactor = new Reactor;
    reactor->running = true;
    reactor->loopThread = std::thread(reactorLoop, reactor);
    return reactor;
}

/**
 * @brief Registers a file descriptor and its callback function with the reactor.
 * 
 * @param reactorPtr Pointer to the reactor instance.
 * @param fd File descriptor to monitor.
 * @param func Callback function to invoke when the file descriptor is ready.
 * @return 0 on success.
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
 * @param reactorPtr Pointer to the reactor instance.
 * @param fd File descriptor to remove.
 * @return 0 on success.
 */
int removeFdFromReactor(void* reactorPtr, int fd) {
    Reactor* reactor = static_cast<Reactor*>(reactorPtr);
    std::lock_guard<std::mutex> guard(reactor->lock);
    reactor->handlers.erase(fd);
    return 0;
}

/**
 * @brief Stops the reactor and cleans up its resources.
 * 
 * This function signals the event loop to stop and joins the thread.
 *
 * @param reactorPtr Pointer to the reactor instance.
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
