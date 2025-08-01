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
 * @brief Internal data structure representing the reactor.
 *
 * Manages file descriptors and their associated callbacks, as well as
 * synchronization and threading for the event loop.
 */
struct Reactor {
    std::unordered_map<int, reactorFunc> handlers;
    std::mutex lock;
    std::atomic<bool> running;
    std::thread loopThread;
};

/**
 * @brief The internal loop that runs the reactor.
 *
 * Uses `select()` to monitor all registered file descriptors.
 * When a descriptor becomes ready, the associated callback is invoked.
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
 * @brief Starts the reactor loop in a new thread.
 *
 * Allocates a new `Reactor` instance, starts the internal event loop, and returns a pointer to it.
 *
 * @return Pointer to the running reactor, or nullptr on failure.
 */
void* startReactor() {
    Reactor* reactor = new Reactor;
    reactor->running = true;
    reactor->loopThread = std::thread(reactorLoop, reactor);
    return reactor;
}

/**
 * @brief Registers a file descriptor and its callback with the reactor.
 *
 * When the file descriptor becomes readable, the specified callback function is called.
 *
 * @param reactorPtr Pointer to the reactor instance (from `startReactor`).
 * @param fd The file descriptor to monitor.
 * @param func The callback function to invoke when the fd is ready.
 * @return 0 on success, -1 on failure.
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
 * Stops monitoring the specified fd and removes its callback.
 *
 * @param reactorPtr Pointer to the reactor instance.
 * @param fd The file descriptor to remove.
 * @return 0 on success, -1 on failure.
 */
int removeFdFromReactor(void* reactorPtr, int fd) {
    Reactor* reactor = static_cast<Reactor*>(reactorPtr);
    std::lock_guard<std::mutex> guard(reactor->lock);
    reactor->handlers.erase(fd);
    return 0;
}

/**
 * @brief Stops the reactor loop and cleans up resources.
 *
 * Signals the event loop to stop, joins the loop thread, and deletes the reactor.
 *
 * @param reactorPtr Pointer to the reactor instance.
 * @return 0 on success, -1 on failure.
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
