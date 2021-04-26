//
// Created by Maria.Filipanova on 4/26/21.
//
#include "test.h"
#include <unordered_map>

static int my_func(int a) {
    usleep(30);
    return (a * 5) % 7;
}
static int my_func_unique(int a) {
    usleep(30);
    return a + 1;
}
static int my_func_stats(const std::vector<std::string>& a) {
    usleep(30);
    a.size();
    return getpid();
}

void test() {

    // inplace
    {
        std::vector<int> kek(40);
        std::vector<int> ans(40);
        for (int i = 0; i < 40; ++i) {
            kek[i] = i;
            ans[i] = i;
        }

        transform(3, kek.begin(), kek.end(), kek.begin(), my_func, 4);
        std::transform(ans.begin(), ans.end(), ans.begin(), my_func);
        for (int i = 0; i < 40; ++i) {
            if (kek[i] != ans[i]) {
                std::cout << i << ": " << kek[i] << " != " << ans[i] << "\n";
                exit(1);
            }
        }
    }

    // big test

    {
        std::vector<int> kek(1000);
        std::vector<int> res(1000);
        std::vector<int> ans(1000);
        for (int i = 0; i < 1000; ++i) {
            kek[i] = i;
            ans[i] = i;
        }

        transform(6, kek.begin(), kek.end(), res.begin(), my_func, 4);
        std::transform(ans.begin(), ans.end(), ans.begin(), my_func);
        for (int i = 0; i < 1000; ++i) {
            if (res[i] != ans[i]) {
                std::cout << i << ": " << res[i] << " != " << ans[i] << "\n";
            }
            assert(res[i] == ans[i]);
        }
    }

    //unique test

    {
        std::vector<int> kekkk(10);
        std::vector<int> res(10);
        for (int i = 0; i < 10; ++i) {
            kekkk[i] = 0;
        }

        transform(6, kekkk.begin(), kekkk.end(), res.begin(), my_func_unique, 4);
        for (auto elem: res) {
            assert(elem == 1);
        }
    }

    // see stats
    // он выводит статистику потому что я не знаю как проверить что там штуки "примерно равные"
    {
        std::vector<std::vector<std::string>> kekkk(1000);
        std::vector<int> res(1000);
        for (int i = 0; i < 1000; ++i) {
            kekkk[i] = std::vector<std::string>(1);
            kekkk[i][0] = "kek";
        }

        transform(6, kekkk.begin(), kekkk.end(), res.begin(), my_func_stats, 4);
        std::unordered_map<int, int> stats;
        for (auto elem: res) {
            stats[elem]++;
        }
        for (auto elem: stats) {
            std::cout << elem.first << " processed " << elem.second << " elements\n";
        }
    }
}

