#include "transform.h"
#include "test.h"
using namespace std;

int my_func(int a) {
    usleep(30);
    return (a * 5) % 7;
}

int main() {
    test();
    std::vector<int> kek(15);
    std::vector<int> kek1(15);
    std::vector<int> out(15);
    std::vector<int> out1(15);
    for (int i = 0; i < 15; ++i) {
        kek[i] = i;
        kek1[i] = i;
    }

    transform(3, kek.begin(), kek.end(), out.begin(), my_func, 4);
    basicTransform(3, kek1.begin(), kek1.end(), out1.begin(), my_func, 4);

    for (const auto& item : out) {
        std::cout << item << " ";
    }
    std::cout << "\n";
    for (const auto& item : out1) {
        std::cout << item << " ";
    }
    std::cout << "\n";
    return 0;
}