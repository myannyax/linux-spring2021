//
// Created by Maria.Filipanova on 3/21/21.
//
#include <cstdio>
#include <string>
#include <string.h>
#include <vector>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <cerrno>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


using namespace std;

#define IFSTREAM 0
#define READ 1
#define MMAP 2

bool print_error() {
  if (errno) {
    perror("text");
    printf("%s", strerror(errno));
    return true;
  }
  return false;
}

ssize_t read_all(int fd, char *buff, size_t len) {
  ssize_t actual_read = 0;
  while (len != 0) {
    auto read_cnt = read(fd, buff, len);
    if (read_cnt == 0) {
      break;
    }
    if (read_cnt < 0) {
      if (errno == EINTR) {
        continue;
      }
      return -1;
    }
    actual_read += read_cnt;
    buff += read_cnt;
    len -= read_cnt;
  }
  return actual_read;
}

int fileSize(int fd) {
  struct stat s;
  if (fstat(fd, &s) == -1) {
    int saveErrno = errno;
    fprintf(stderr, "fstat(%d) returned errno=%d.", fd, saveErrno);
    return(-1);
  }
  return s.st_size;
}

static void print_map(const unordered_map<char, int> &kek) {
  for (auto&[c, n]: kek) {
    cout << "char " << c << ":" << n << "\n";
  }
}

int print_file_stats(string filename, int mode, int bs) {
  if (bs < 0) {
    int fd = open(filename.c_str(), O_RDONLY);
    bs = fileSize(fd);
    close(fd);
  }
  char *buffer = new char[bs];
  unordered_map<char, int> stats;

  if (mode == IFSTREAM) {
    ifstream fs(filename, std::ios::binary);
    for (;;) {
      if (fs.read(buffer, bs)) {
        for (int i = 0; i < bs; ++i) {
          char c = buffer[i];
          stats[c]++;
        }
      } else {
        break;
      }
    }
    fs.close();
    print_map(stats);
    return 0;
  } else if (mode == READ) {
    int fd = open(filename.c_str(), O_RDONLY);
    if (print_error()) {
      return 1;
    }

    for(;;) {
      ssize_t c = read_all(fd, buffer, bs);
      if (c == 0) {
        break;
      }
      if (c == -1) {
        print_error();
        return -1;
      }
      for (int i = 0; i < bs; ++i) {
        char сс = buffer[i];
        stats[сс]++;
      }
    }
    close(fd);
    print_map(stats);
    return 0;
  } else if (mode == MMAP) {
    int fd = open(filename.c_str(), O_RDONLY);
    auto fsize = fileSize(fd);
    char *ptr = static_cast<char *>(mmap(NULL, fsize, PROT_READ, MAP_SHARED, fd, 0));
    if (ptr == MAP_FAILED) {
      printf("Mapping Failed\n");
      return 1;
    }
    for (int i = 0; i < fsize; ++i) {
      char c = ptr[i];
      stats[c]++;
    }
    close(fd);
    print_map(stats);
    return 0;
  } else {
    printf("Unsupported mode\n");
    return 1;
  }
  //to check that i haven't missed a return statement somewhere
  return 2;
}

int main(int argc, char *argv[]) {
  vector<string> filenames;
  int mode = stoi(argv[1]);
  int bs = stoi(argv[2]);

  auto success = true;
  for (int i = 3; i < argc; ++i) {
    auto filename = argv[i];
    printf("Stats for: %s\n", filename);
    int res = print_file_stats(filename, mode, bs * 8);
    success = success && (res == 0);
  }
  return !success;
}