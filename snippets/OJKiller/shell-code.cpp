#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

int main() {
  unsigned char code[] = {0xb8, 0x00, 0x00, 0x00, 0x00, 0xc3};

  int num = -1;
  memcpy(&code[1], &num, 4);

  void *mem = mmap(NULL, sizeof(code), PROT_WRITE | PROT_EXEC,
                   MAP_ANON | MAP_PRIVATE, -1, 0);
  memcpy(mem, code, sizeof(code));

  typedef int (*fp)();
  int (*func)() = reinterpret_cast<fp>(mem);
  return func();
}
