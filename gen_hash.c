#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(__APPLE__)
#  define COMMON_DIGEST_FOR_OPENSSL
#  include <CommonCrypto/CommonDigest.h>
#  define MD5 CC_MD5
#else
#  include <openssl/md5.h>
#endif

char *digest_to_string(unsigned char* buffer) {
	int i;
	char* buf_str = (char*) malloc (MD5_DIGEST_LENGTH * sizeof(char));
	char* buf_ptr = buf_str;

	for (i = 0; i < MD5_DIGEST_LENGTH; i++) {
	  buf_ptr += sprintf(buf_ptr, "%02x", buffer[i]);
	}
	*(buf_ptr + 1) = '\0';

	return buf_str;
}

int main(int argc, char * argv[]) {
	int i;
  unsigned char result[MD5_DIGEST_LENGTH];

  MD5(argv[1], strlen(argv[1]), result);

	printf("%s\n", digest_to_string(result));

  return EXIT_SUCCESS;
}
