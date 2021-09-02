#include <stdio.h>
#include <unistd.h>

enum lol {
	a,
	b,
	c
};

#define str(x) #x
#define xstr(x) str(x)

int main(int argc, char **argv) {
	enum lol mdr = a;

	printf("%s\n", xstr(mdr));
	return (0);
}