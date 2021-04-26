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
#include <csignal>
#include <pthread.h>
#include <memory>
#include <mutex>

struct MyMemoryCleaner {
    explicit MyMemoryCleaner(size_t len) : len(len) {
        ptr = mmap(nullptr, len, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0);
        if (ptr == MAP_FAILED) {
            throw std::runtime_error(std::strerror(errno));
        }
    }

    ~MyMemoryCleaner() {
        if (ptr != nullptr) {
            munmap(ptr, len);
        }
    }

    void* ptr;

private:
    size_t len;
};

template<class IIt, class ValueType>
struct myState {
    IIt start;
    IIt end;
    ValueType *ptr;
    pthread_mutex_t mutex;

    explicit myState(IIt start, IIt end, ValueType* ptr): start(start), end(end), ptr(ptr) {
        pthread_mutexattr_t attr;
        if (pthread_mutexattr_init(&attr)) {
            throw std::runtime_error(std::strerror(errno));
        }
        if (pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED)) {
            throw std::runtime_error(std::strerror(errno));
        }
        if (pthread_mutex_init(&mutex, &attr)) {
            throw std::runtime_error(std::strerror(errno));
        }
    }

    void lock() {
        auto err = pthread_mutex_lock(&mutex);
        if (err != 0) {
            throw std::runtime_error("failed to lock mutex");
        }
    }

    void unlock() {
        auto err = pthread_mutex_unlock(&mutex);
        if (err != 0) {
            throw std::runtime_error("failed to unlock mutex");
        }
    }

    ~myState() {
        pthread_mutex_destroy(&mutex);
    }

private:
};

template<typename IIt, typename Func, typename ValueType>
void runTask(Func f, myState<IIt, ValueType>* mm, uint chunk_size) {
    while (true) {
        IIt my_start;
        IIt my_end;
        ValueType* my_out;
        {
            std::unique_lock<myState<IIt, ValueType>> lock(*mm);
            my_start = mm->start;
            my_out = mm->ptr;
            if (mm->start == mm->end) {
                return;
            }
            for (int i = 0; i < chunk_size; ++i) {
                if (mm->start == mm->end) break;
                (mm->start)++;
                (mm->ptr)++;
            }
            my_end = mm->start;
        }
        while (my_start != my_end) {
            new(my_out) ValueType(f(*my_start));
            my_start++;
            my_out++;
        }
    }
}

template<typename IIt, typename OIt, typename Func>
void transform(uint n_proc, IIt start, IIt end, OIt o_start, Func f, uint chunk_size) {
    auto len = std::distance(start, end);
    if (len == 0) return;
    if (n_proc > len) {
        n_proc = len;
    }

    using ValueType = decltype(f(*start));

    auto mem = MyMemoryCleaner(len * sizeof (ValueType) + sizeof(myState<IIt, ValueType>));
    auto* mm = reinterpret_cast<myState<IIt, ValueType>*>(mem.ptr);

    auto* ptr = reinterpret_cast<ValueType *>(mm + 1);

    new (mm) myState<IIt, ValueType>(start, end, ptr);

    std::vector<int> pids(n_proc);
    for (int i = 0; i < n_proc; ++i) {
        int child = fork();
        if (child == -1) {
            perror("fork");
            throw std::runtime_error("fork");
        } else if (child != 0) {
            pids[i] = child;
        } else {
            try {
                runTask(f, mm, chunk_size);
            } catch (std::exception &e) {
                std::cerr << e.what() << std::endl;
            }
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < n_proc; i++) {
        int status;
        do {
            pid_t w;
            w = waitpid(pids[i], &status, WUNTRACED | WCONTINUED);
            if (w == -1) {
                perror("waitpid");
                exit(EXIT_FAILURE);
            }

            if (WIFEXITED(status)) {
                printf("exited, status=%d\n", WEXITSTATUS(status));
            } else if (WIFSIGNALED(status)) {
                printf("killed by signal %d\n", WTERMSIG(status));
            } else if (WIFSTOPPED(status)) {
                printf("stopped by signal %d\n", WSTOPSIG(status));
            } else if (WIFCONTINUED(status)) {
                printf("continued\n");
            }
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    std::move(ptr, mm->ptr, o_start);
}

template<typename IIt, typename Func, typename ValueType>
void basicRunTask(Func f, IIt start, IIt end, ValueType* out) {
    while (start != end) {
        new (out) ValueType(f(*start));
        start++;
        out++;
    }
}

template<typename IIt, typename OIt, typename Func>
void basicTransform(uint n_proc, IIt start, IIt end, OIt o_start, Func f, uint chunk_size) {
    auto len = std::distance(start, end);
    if (len == 0) return;
    if (n_proc > len) {
        n_proc = len;
    }
    auto partLen = len / n_proc;

    using ValueType = decltype(f(*start));

    auto mem = MyMemoryCleaner(len * sizeof(ValueType));

    auto* ptr = reinterpret_cast<ValueType *>(mem.ptr);

    std::vector<int> pids(n_proc);
    int c = 0;
    for (int i = 0; i < n_proc; ++i) {
        auto my_begin = start;
        std::advance(start, partLen);
        int child = fork();
        if (child == -1) {
            perror("fork");
            throw std::runtime_error("fork");
        } else if (child != 0) {
            pids[i] = child;
            c += partLen;
        } else {
            try {
                basicRunTask(f, my_begin, start, ptr + c);
            } catch (std::exception &e) {
                std::cerr << e.what() << std::endl;
            }
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < n_proc; i++) {
        int status;
        do {
            pid_t w;
            w = waitpid(pids[i], &status, WUNTRACED | WCONTINUED);
            if (w == -1) {
                perror("waitpid");
                exit(EXIT_FAILURE);
            }

            if (WIFEXITED(status)) {
                printf("exited, status=%d\n", WEXITSTATUS(status));
            } else if (WIFSIGNALED(status)) {
                printf("killed by signal %d\n", WTERMSIG(status));
            } else if (WIFSTOPPED(status)) {
                printf("stopped by signal %d\n", WSTOPSIG(status));
            } else if (WIFCONTINUED(status)) {
                printf("continued\n");
            }
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    std::move(ptr, ptr + len, o_start);
}


#endif //HW_TRANSFORM_H
