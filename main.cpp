//
// Created by Maria.Filipanova on 3/28/21.
//

#include <string>
#include <vector>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <fcntl.h>
#include <cerrno>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <libgen.h>
#include <linux/hdreg.h>
#include <linux/types.h>
#include <linux/fs.h>

using namespace std;

double vectSum(const vector<double> &a) {
  double sum = 0;
  for (size_t i = 0; i < a.size(); ++i) {
    sum += a[i];
  }
  return sum;
}

double statsForFile(const string &path, unsigned int &size) {
  struct stat sb;
  int fd;

  if ((fd = open(path.c_str(), O_RDONLY)) == -1) {
    perror(path.c_str());
    exit(EXIT_FAILURE);
  }

  if (lstat(path.c_str(), &sb) == -1) {
    perror("fstat");
    exit(EXIT_FAILURE);
  }

  unsigned int n_blocks = (sb.st_size + sb.st_blksize - 1) / sb.st_blksize;
  unsigned int prev = 0;
  unsigned int len = 0;
  vector<double> a;
  for (unsigned int j = 0; j < n_blocks; ++j) {
    unsigned int b_c = j;
    if (ioctl(fd, FIBMAP, &b_c) != 0) {
      switch (errno) {
        case EBADF:
          cout << "fd is not a valid file descriptor\n";
          perror("ioctl");
          exit(EXIT_FAILURE);
        case EFAULT:
          cout << "argp references an inaccessible memory area\n";
          perror("ioctl");
          exit(EXIT_FAILURE);
        case EINVAL:
          cout << "request or argp is not valid\n";
          perror("ioctl");
          exit(EXIT_FAILURE);
        case ENOTTY:
          cout
              << "fd is not associated with a character special device&. The specified request does not apply to the kind of object\n"
                 "              that the file descriptor fd references.\n";
          perror("ioctl");
          exit(EXIT_FAILURE);
      }

      perror("ioctl");
      exit(EXIT_FAILURE);
    }
    if (b_c == 0) {
      len = 0;
    } else if (len == 0) {
      len = 1;
    } else if (b_c == prev + 1) {
      len++;
    } else {
      a.push_back((double)len / n_blocks);
      len = 1;
    }
    prev = b_c;
  }
  close(fd);
  size = sb.st_size;
  return vectSum(a) / a.size();
}

int main(int argc, char *argv[]) {
  vector<string> filenames;

  unsigned long long size_sum = 0;
  double stats = 0;
  for (int i = 1; i < argc; ++i) {
    auto filename = argv[i];
    unsigned int size = 0;
    double stat = statsForFile(filename, size);
    stats += (stat / size);
    size_sum += size;
  }
  cout << stats << "\n";
  return 0;
}