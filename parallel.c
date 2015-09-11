#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#if defined(__linux__)
#include <crypt.h>
#endif

#define HASH_SIZE 256
#define NTHREADS 2

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
 * @brief Generate a Hash from a password string
 * @param password password string
 * @param hash generated Hash
 */
char *generate_password_hash(const char *password) {
	return crypt(password, "aa");
}

/*
 * @brief Set 'password' to the next value to be tested
 * @param password password string
 * @return -1 if there are no more possible values
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
	// char generatedHash[HASH_SIZE + 1];
	// strcpy(generatedHash, generate_password_hash(password));

	// compare generated hash with target hash
	if (strcmp(target_hash, generate_password_hash(password)) == 0) {
		return 1;
	}

	return 0;
}

void *compare_password_pthread(void *data_ptr) {
  int i;
  thread_data *data = (thread_data *)data_ptr;

  for (i = 0; i < data->ntries; i++) {
    // printf("%s\n", data->password);

    if (compare_password(data->target_hash, data->password)) {
      printf("%s - %s\n", data->target_hash, data->password);
      break;
    }

    if (!get_next_password(data->password)) {
      break;
    }
  }

  return NULL;
}

int main(int argc, char *argv[]) {
  // time spent running the program
	clock_t start = 0;
	clock_t finish = 0;

  thread_data *data[NTHREADS];
  pthread_t thread[NTHREADS];

  // ensure the program got the right arguments
	if (argc < 3) {
		printf("Usage: %s <password length> <target hash>", argv[0]);
		return 1;
	}

	passwordLength = atoi(argv[1]);

  for (int i = 0; i < NTHREADS; i++) {
    data[i] = malloc(sizeof(thread_data));
    data[i]->target_hash = strdup(argv[2]);
    data[i]->password = malloc(sizeof(char) * (passwordLength + 1));
    data[i]->ntries = 5000;

    // set initial password to '0' chars
  	memset(data[i]->password, '0', passwordLength);
  	data[i]->password[passwordLength] = '\0';

    // TODO: split passwords between threads
    if (i == 1) {
      data[i]->password[0] = '5';
    }
  }

	// start timer
	start = clock();

  for (int i = 0; i < NTHREADS; i++) {
    pthread_create(&thread[i], NULL, compare_password_pthread, data[i]);
  }

  for (int i = 0; i < NTHREADS; i++) {
    pthread_join(thread[i], NULL);
  }

	// stop timer and print result
	finish = clock();
	printf("%f\n", (float)(finish - start) / CLOCKS_PER_SEC);

	return 0;
}
