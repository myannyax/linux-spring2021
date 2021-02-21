# linux-spring2021
'''
gcc -fPIC -c lib.c -o lib.o
gcc -shared lib.o -o lib.so
LD_PRELOAD=./lib.so ls
'''
