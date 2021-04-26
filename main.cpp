#include "transform.h"
using namespace std;

int my_func(int a) {
    usleep(30);
    return a + 5;
}

int main(int argc, char *argv[]) {
    std::vector<int> kek(15);
    std::vector<int> out(15);
    for (int i = 0; i < 15; ++i) {
        kek[i] = i;
    }

    transform(3, kek.begin(), kek.end(), out.begin(), my_func);

    for (const auto& kek : out) {
        std::cout << kek << " ";
    }
    std::cout << "\n";
    return 0;
}