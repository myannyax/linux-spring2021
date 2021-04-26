//
// Created by Maria.Filipanova on 4/25/21.
//

#ifndef HW_TRANSFORM_H
#define HW_TRANSFORM_H

#include <cstring>
#include <string>
#include <pthread.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <pthread.h>
#include <memory>

#define CHUNK_SIZE 3

template<class IIt, class ValueType>
struct my_mutex {
    IIt start;
    IIt end;
    pthread_mutex_t *mutex;
    size_t len;

    ValueType *ptr;

    explicit my_mutex(std::size_t len): len(len) {
        auto my_ptr = mmap(nullptr, len, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0);
        if (my_ptr == MAP_FAILED) {
            my_ptr = nullptr;
            throw std::runtime_error(std::strerror(errno));
        }
        ptr = reinterpret_cast<ValueType *>(my_ptr);
        errno = 0;
        off_t my_size = sizeof(pthread_mutex_t);

        auto addr = mmap(nullptr, my_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0);
        if (addr == MAP_FAILED) {
            throw std::runtime_error(std::strerror(errno));
        }
        auto *mutex_ptr = (pthread_mutex_t *) addr;

        pthread_mutexattr_t attr;
        if (pthread_mutexattr_init(&attr)) {
            throw std::runtime_error(std::strerror(errno));
        }
        if (pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED)) {
            throw std::runtime_error(std::strerror(errno));
        }
        if (pthread_mutex_init(mutex_ptr, &attr)) {
            throw std::runtime_error(std::strerror(errno));
        }

        mutex = mutex_ptr;
    }

    void lock() {
        auto err = pthread_mutex_lock(mutex);
        if (err != 0) {
            throw std::runtime_error("failed to lock mutex");
        }
    }

    void unlock() {
        auto err = pthread_mutex_unlock(mutex);
        if (err != 0) {
            throw std::runtime_error("failed to unlock mutex");
        }
    }

    ~my_mutex() {
        pthread_mutex_destroy(mutex);
        if (mutex != nullptr) {
            munmap(mutex, sizeof(pthread_mutex_t));
        }
        if (ptr != nullptr) {
            munmap(ptr, len);
        }
    }

private:
};

template<typename IIt, typename Func, typename ValueType>
void runTask(Func f, my_mutex<IIt, ValueType> mm) {
    std::cout << "runTask " << getpid() << "\n";
    while (true) {
        std::cout << "try in " << getpid() << "\n";
        mm.lock();
        std::cout << "in " << getpid() << "\n";
        auto my_start = mm.start;
        auto my_end = mm.end;
        auto my_out = mm.ptr;
        if (mm.start == mm.end) {
            mm.unlock();
            return;
        }
        for (int i = 0; i < CHUNK_SIZE; ++i) {
            if (mm.start == mm.end) break;
            mm.start++;
            mm.ptr++;
        }
        my_end = mm.start;
        std::cout << "out " << getpid() << "\n";
        mm.unlock();
        while (my_start != my_end) {
            new(my_out) ValueType(f(*my_start));
            my_start++;
            my_out++;
        }
    }
}

template<typename IIt, typename OIt, typename Func>
void transform(uint n_proc, IIt start, IIt end, OIt o_start, Func f) {
    auto len = std::distance(start, end);
    if (len == 0) return;
    if (n_proc > len) {
        n_proc = len;
    }

    using ValueType = decltype(f(*start));

    auto mm = my_mutex<IIt, ValueType>(len * sizeof(ValueType));
    mm.start = start;
    mm.end = end;
    auto my_ptr = mm.ptr;
    std::vector<int> pids(n_proc);
    for (int i = 0; i < n_proc; ++i) {
        int child = fork();
        if (child == -1) {
            perror("fork");
            return;
        } else if (child != 0) {
            std::cout << "fork created " << child << "\n";
            pids[i] = child;
        } else {
            try {
                runTask(f, mm);
            } catch (std::exception &e) {
                std::cerr << e.what() << std::endl;
            }
            exit(0);
        }
    }

    for (int i = 0; i < n_proc; i++) {
        std::cout << "start waiting for  " << pids[i] << "\n";
        waitpid(pids[i], nullptr, 0);
    }
    while (start != end) {
        *o_start = std::move(*my_ptr);
        o_start++;
        my_ptr++;
        start++;
    }
    std::cout << "end parent  " << "\n";
}

#endif //HW_TRANSFORM_H
