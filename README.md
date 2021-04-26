# linux-spring2021

## Multi-process Transform
### Setup
```bash
mkdir build
cd build
CXX=g++-10 cmake ..
```
### Usage
В transform.h есть функция transform(uint n_proc, IIt start, IIt end, OIt o_start, Func f), пример ее использования в main.cpp