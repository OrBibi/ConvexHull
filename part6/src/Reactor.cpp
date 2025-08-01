#include "../include/Reactor.hpp"
#include <unordered_map>
#include <sys/select.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>

struct Reactor {
     std::unordered_map<int, reactorFunc> handlers; // Map of file descriptors to callback functions
    std::mutex lock; // Mutex for synchronizing access to `handlers`
    std::atomic<bool> running; // Flag indicating if the reactor is running
    std::thread loopThread; //Thread running the reactor event loop
};

/**
 * @brief The internal event loop for the reactor.
 *
 * Monitors registered file descriptors using `select`. When a descriptor becomes ready,
 * its associated callback function is called. The loop runs as long as the `running` flag is true.
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
                func(fd); // Invoke the registered callback
            }
        }
    }
}

/**
 * @brief Starts a new reactor instance and its event loop in a separate thread.
 *
 * @return A pointer to the created reactor instance, or nullptr on failure.
 */
void* startReactor() {
    Reactor* reactor = new Reactor;
    reactor->running = true;
    reactor->loopThread = std::thread(reactorLoop, reactor);
    return reactor;
}

/**
 * @brief Registers a file descriptor and a callback function with the reactor.
 *
 * When the file descriptor becomes ready, the callback function is invoked.
 *
 * @param reactorPtr Pointer to the reactor instance.
 * @param fd The file descriptor to monitor.
 * @param func The callback function to call when `fd` is ready.
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
 * Stops monitoring the file descriptor and removes its associated callback.
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
 * @brief Stops the reactor loop and deallocates the reactor instance.
 *
 * Signals the background event loop to stop, joins the thread, and frees memory.
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
