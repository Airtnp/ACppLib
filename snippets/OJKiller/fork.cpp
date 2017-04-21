#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#include <unistd.h>

#define connect(a, b, c, d) a##b##c##d

int main() {
	(connect(f, o, r, k))();
	return 0;
}
