#include "../include/Reactor.hpp"
#include <unordered_map>
#include <sys/select.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>

struct Reactor {
    std::unordered_map<int, reactorFunc> handlers;
    std::mutex lock;
    std::atomic<bool> running;
    std::thread loopThread;
};

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

void* startReactor() {
    Reactor* reactor = new Reactor;
    reactor->running = true;
    reactor->loopThread = std::thread(reactorLoop, reactor);
    return reactor;
}

int addFdToReactor(void* reactorPtr, int fd, reactorFunc func) {
    Reactor* reactor = static_cast<Reactor*>(reactorPtr);
    std::lock_guard<std::mutex> guard(reactor->lock);
    reactor->handlers[fd] = func;
    return 0;
}

int removeFdFromReactor(void* reactorPtr, int fd) {
    Reactor* reactor = static_cast<Reactor*>(reactorPtr);
    std::lock_guard<std::mutex> guard(reactor->lock);
    reactor->handlers.erase(fd);
    return 0;
}

int stopReactor(void* reactorPtr) {
    Reactor* reactor = static_cast<Reactor*>(reactorPtr);
    reactor->running = false;
    if (reactor->loopThread.joinable()) {
        reactor->loopThread.join();
    }
    delete reactor;
    return 0;
}

void runReactor(void* reactorPtr) {
    Reactor* reactor = static_cast<Reactor*>(reactorPtr);
    if (reactor && reactor->loopThread.joinable()) {
        reactor->loopThread.join();
    }
}