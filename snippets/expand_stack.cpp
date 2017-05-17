#include <cstdlib>
int main() {
    int size = 64 << 20;
    char *p = (char*)std::malloc(size) + size;
    __asm__("movl %0, %%esp\n" : "r"(p));
}