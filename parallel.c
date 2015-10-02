#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>
#if defined(__APPLE__)
#  define COMMON_DIGEST_FOR_OPENSSL
#  include <CommonCrypto/CommonDigest.h>
#  define MD5 CC_MD5
#else
#  include <openssl/md5.h>
#endif

#define NTHREADS 10

int passwordLength = 0;
int success = 0;

char *target_hash;

/*
* @brief Generate a string from a md5 digest
* @param buffer md5 digest
* @return hash string
*/
char *digest_to_string(unsigned char* buffer) {
	int i;
	char* buf_str = (char*) malloc ((MD5_DIGEST_LENGTH + 1) * sizeof(char));
	char* buf_ptr = buf_str;

	for (i = 0; i < MD5_DIGEST_LENGTH; i++) {
	  buf_ptr += sprintf(buf_ptr, "%02x", buffer[i]);
	}
	*(buf_ptr + 1) = '\0';

	return buf_str;
}

/*
* @brief Generate a Hash from a password string
* @param password password string
* @return generated Hash
*/
char *generate_password_hash(const char *password) {
  unsigned char result[MD5_DIGEST_LENGTH];
  MD5(password, strlen(password), result);
  return digest_to_string(result);
}

/*
* @brief Set 'password' to the next value to be tested
* @param password password string
* @return 0 if there are no more possible values
*/
int get_next_password(char *password) {
  // Increment password chars from right to left Ex.: 0000, 0001, 0002 ... 0010...
  int index = passwordLength - 1;

  while (index >= 0) {
    // just increase last digit
    if (password[index] != '9') {
      password[index]++;
      break;
    }

    // no more possible passwords
    if (index == 0) {
      return 0;
    }

    // carry 1 to next digit
    password[index] = '0';
    index--;
  }

  return 1;
}

/*
* @brief compare password with target_hash
* @param target_hash string
* @param password string
* @return 1 if equal, otherwise 0.
*/
int compare_password(const char *target_hash, const char *password) {
  int result = 0;
  char *hash = generate_password_hash(password);
  // compare generated hash with target hash
  if (strcmp(target_hash, hash) == 0) {
    result = 1;
  }

  free(hash);
  return result;
}

/*
* @brief converts integers to strings with leading zeros
* @param n integer to be converted
* @param digits how many digits should have the result string
* @param str result string
*/
void *intToString(int n, int digits, char *str) {
  int i;
  str[digits] = '\0';

  for(i = digits-1; i >= 0; i--) {
    str[i] = (n % 10) + '0';
    n = n/10;
  }

  return str;
}

int power(int base, int exp) {
  int i;
  int result = 1;

  for (i = 0; i < exp; i++) {
    result = result * base;
  }

  return result;
}

/*
* @brief main thread code
*/
void break_password() {
  int i;
  int thread_num = omp_get_thread_num();
  int thread_count = omp_get_num_threads();
  char *password = malloc(sizeof(char) * (passwordLength + 1));

  printf("STARTING %d of %d\n", thread_num, thread_count);
  // split tasks equaly per threads
  int ntries = power(10, passwordLength) / thread_count;

  // set initial password to i * ntries with leading zeros
  intToString(thread_num * ntries, passwordLength, password);

  for (i = 0; i < ntries && !success; i++) {
    if (compare_password(target_hash, password)) {
      // password found!
      #pragma omp critical
      success = 1;

      printf("%s - %s\n", target_hash, password);
      break;
    }

    // no more possible passwords
    if (!get_next_password(password)) {
      break;
    }
  }

  free(password);
  printf("DONE %d - on try #%d\n", thread_num, i);
}

int main(int argc, char *argv[]) {
  // time spent running the program
  clock_t start = 0;
  clock_t finish = 0;

  // ensure the program got the right arguments
  if (argc < 3) {
    printf("Usage: %s <password length> <target hash>", argv[0]);
    return 1;
  }

  passwordLength = atoi(argv[1]);
  target_hash = strdup(argv[2]);

  // start timer
  start = clock();

  #pragma omp parallel num_threads(NTHREADS) shared(success, target_hash, passwordLength)
  break_password();

  // stop timer and print result
  finish = clock();
  printf("%f\n", (float)(finish - start) / CLOCKS_PER_SEC);

  return 0;
}
