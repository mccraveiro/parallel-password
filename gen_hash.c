#include <stdio.h>
#include <unistd.h>

#if defined(__linux__)
#include <crypt.h>
#endif

int main(int argc, char * argv[]){
	printf("%s\n", crypt(argv[1], "aa"));
}
