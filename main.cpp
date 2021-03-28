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
  for (double i : a) {
    sum += i;
  }
  return sum;
}

typedef struct {
  double stat;
  off_t size;
} statResult;

void handleError(const string &path, const string &location) {
  perror((path + ":" + location).c_str());
  exit(EXIT_FAILURE);
}

statResult statsForFile(const string &path) {
  struct stat sb;
  int fd;

  if ((fd = open(path.c_str(), O_RDONLY)) == -1) {
    handleError(path, "open");
  }

  if (lstat(path.c_str(), &sb) == -1) {
    handleError(path, "fstat");
  }

  unsigned int blkSize = 0;
  if (ioctl(fd, FIGETBSZ, &blkSize) == -1) {
    handleError(path, "ioctl(figetbsz)");
  }


  unsigned int n_blocks = (sb.st_size + blkSize - 1) / blkSize;
  unsigned int prev = 0;
  unsigned int len = 0;
  vector<double> sequenceLenInfo;
  for (unsigned int j = 0; j < n_blocks; ++j) {
    unsigned int b_c = j;
    if (ioctl(fd, FIBMAP, &b_c) != 0) {
      handleError(path, "ioctl(fibmap)");
    }
    if (b_c == 0) {
      len = 0;
    } else if (len == 0) {
      len = 1;
    } else if (b_c == prev + 1) {
      len++;
    } else {
      sequenceLenInfo.push_back((double)len / n_blocks);
      len = 1;
    }
    prev = b_c;
  }
  close(fd);
  if (sequenceLenInfo.empty()) {
    return statResult {0, sb.st_size};
  }
  return statResult {vectSum(sequenceLenInfo) / sequenceLenInfo.size(), sb.st_size};
}

int main(int argc, char *argv[]) {
  unsigned long long size_sum = 0;
  double stats = 0;
  for (int i = 1; i < argc; ++i) {
    auto path = argv[i];
    statResult res = statsForFile(path);
    stats += (res.stat * res.size);
    size_sum += res.size;
  }
  cout << stats / size_sum << "\n";
  return 0;
}