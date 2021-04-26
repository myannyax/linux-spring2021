//
// Created by Maria.Filipanova on 4/26/21.
//
#include "test.h"

static int my_func(int a) {
    usleep(30);
    return (a * 5) % 7;
}

void test() {

    // inplace
    {
        std::vector<int> kek(40);
        std::vector<int> ans(40);
        for (int i = 0; i < 15; ++i) {
            kek[i] = i;
            ans[i] = i;
        }

        transform(3, kek.begin(), kek.end(), kek.begin(), my_func);
        std::transform(ans.begin(), ans.end(), ans.begin(), my_func);
        for (int i = 0; i < 15; ++i) {
            assert(kek[i] == ans[i]);
        }
    }

    // big test

    {
        std::vector<int> kek(100000);
        std::vector<int> res(100000);
        std::vector<int> ans(100000);
        for (int i = 0; i < 15; ++i) {
            kek[i] = i;
            ans[i] = i;
        }

        transform(6, kek.begin(), kek.end(), res.begin(), my_func);
        std::transform(ans.begin(), ans.end(), ans.begin(), my_func);
        for (int i = 0; i < 15; ++i) {
            assert(res[i] == ans[i]);
        }
    }

}

