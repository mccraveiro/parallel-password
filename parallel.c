#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
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
	char* buf_str = (char*) malloc ((MD5_DIGEST_LENGTH * 2 + 1) * sizeof(char));
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
    if (password[index] != '9' && password[index] != 'z') {
      password[index]++;
      // printf("%s\n", password);
      break;
    } else if (password[index] == '9') {
      password[index] = 'a';
      // printf("%s\n", password);
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

  //free(hash);
  return result;
}

void init_password(char c, int len, char *str) {
  for(int i = 1; i < len; i++) {
    str[i] = '0';
  }

  str[0] = c;
  str[len] = '\0';
}

char int_to_char(int n) {
  if (n < 10) {
    n += 48;
  } else {
    n += 87;
  }

  return n;
}

char *balance_work(int thread_num, int thread_count) {
  int m = 37;
  int n = thread_count;
  int p = floor(m / n);
  int r = m % n;
  int x, y, f;

  if (n - thread_num <= r) {
    f = n - r;
    x = thread_num * (p + 1) - f;
    y = (thread_num + 1) * (p + 1) - f;
  } else {
    x = thread_num * p;
    y = (thread_num + 1) * p;
  }

  char *result = malloc(2 * sizeof(char));
  result[0] = int_to_char(x);
  result[1] = int_to_char(y);
  return result;
}

/*
* @brief main thread code
*/
void break_password() {
  long int i = 0;
  int thread_num = omp_get_thread_num();
  int thread_count = omp_get_num_threads();
  char *password = malloc(sizeof(char) * (passwordLength + 1));

  // split tasks equaly per threads
  char *workload = balance_work(thread_num, thread_count);
  char first_char = workload[0];
  char last_char = workload[1];

  printf("STARTING %d of %d - Going from %c to %c\n", thread_num, thread_count, first_char, last_char);

  // set initial password to first_char with leading zeros
  init_password(first_char, passwordLength, password);

  while (!success) {
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

    if (password[0] == last_char) {
      break;
    }

    i++;
  }

  free(password);
  printf("DONE %d - on try #%ld\n", thread_num, i);
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
