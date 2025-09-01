#pragma once

#include <iostream>
#include <cstdint>
#include <sys/epoll.h>
#include <functional>

#define MAX_EVENTS 1024
class EventLoop {
public:
    EventLoop() : epfd_(-1) {
        epfd_ = epoll_create1(0);
        if (epfd_ == -1) {
            std::cerr << "epoll_create1 failed" << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    void AddEvent(int fd, uint32_t events, void* ptr) {
        epoll_event ev;
        ev.events = events;
        ev.data.ptr = ptr;
        if (epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev) == -1) {
            std::cerr << "epoll_ctl failed" << std::endl;
        }
    }
    void ModEvent(int fd, uint32_t events, void* ptr) {
        epoll_event ev;
        ev.events = events;
        ev.data.ptr = ptr;
        if (epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &ev) == -1) {
            std::cerr << "epoll_ctl failed" << std::endl;
        }
    }
    void DelEvent(int fd) {
        if (epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, nullptr) == -1) {
            std::cerr << "epoll_ctl failed" << std::endl;
        }
    }

    void Run() {
        epoll_event events[MAX_EVENTS];
        while (true) {
            int nfds = epoll_wait(epfd_, events, MAX_EVENTS, -1);
            if (nfds == -1) {
                if (errno == EINTR)
                    continue; // Interrupted by signal, retry
                std::cerr << "epoll_wait failed" << std::endl;
                break;
            }
            for (int i = 0; i < nfds; ++i) {
                auto handler = static_cast<std::function<void(int)> *>(events[i].data.ptr);
                (*handler)(events[i].events);
            }
        }
    }
private:
    int epfd_;
};