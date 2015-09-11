#include <stdio.h>
#include <crypt.h>

int main(int argc, char * argv[]){
	struct crypt_data data;
	data.initialized = 0;

	printf("%s\n", crypt_r(argv[1], "aa", &data));
}
