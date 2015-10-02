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

#define NTHREADS 5

typedef struct {
  char *target_hash;
  char *password;
  int ntries;
} thread_data;

/*
* @brief number of chars used on password
*/
int passwordLength = 0;

/*
* @brief success flag
*/
int success = 0;

/*
* @brief Generate a string from a md5 digest
* @param buffer md5 digest
* @return hash string
*/
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
  // compare generated hash with target hash
  if (strcmp(target_hash, generate_password_hash(password)) == 0) {
    return 1;
  }

  return 0;
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
* @param data_ptr struct containing all data used by the thread
*/
void *compare_password_pthread(void *data_ptr) {
  thread_data *data = (thread_data *)data_ptr;
  int i;

  for (i = 0; i < data->ntries && !success; i++) {
    // printf("%s\n", data->password);

    if (compare_password(data->target_hash, data->password)) {
      // password found!
      success = 1;
      printf("%s - %s\n", data->target_hash, data->password);
      return NULL;
    }

    // no more possible passwords
    if (!get_next_password(data->password)) {
      return NULL;
    }
  }

  return NULL;
}

int main(int argc, char *argv[]) {
  // time spent running the program
  clock_t start = 0;
  clock_t finish = 0;

  thread_data *data[NTHREADS];

  int i, ntries;

  // ensure the program got the right arguments
  if (argc < 3) {
    printf("Usage: %s <password length> <target hash>", argv[0]);
    return 1;
  }

  passwordLength = atoi(argv[1]);
  // split tasks equaly per threads
  ntries = power(10, passwordLength)/NTHREADS;

  for (i = 0; i < NTHREADS; i++) {
    data[i] = malloc(sizeof(thread_data));
    data[i]->target_hash = strdup(argv[2]);
    data[i]->password = malloc(sizeof(char) * (passwordLength + 1));
    data[i]->ntries = ntries;

    // set initial password to i * ntries with leading zeros
    intToString(i * ntries, passwordLength, data[i]->password);
  }

  // start timer
  start = clock();

  #pragma omp parallel num_threads(NTHREADS)
  compare_password_pthread(data[omp_get_thread_num()]);

  // stop timer and print result
  finish = clock();
  printf("%f\n", (float)(finish - start) / CLOCKS_PER_SEC);

  return 0;
}
