#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#if defined(__linux__)
#include <crypt.h>
#endif

#define HASH_SIZE 256
#define NTHREADS 5

typedef struct {
    int id;
    char *target_hash;
    char *password;
    int ntries;
} thread_data;

/*
 * @brief number of chars used on password
 */
int passwordLength = 0;

pthread_t thread[NTHREADS];

/*
 * @brief Generate a Hash from a password string
 * @param password password string
 * @param hash generated Hash
 */
char *generate_password_hash(const char *password) {
    struct crypt_data data;
	data.initialized = 0;

    return crypt_r(password, "aa", &data);
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
    int i, j;
    thread_data *data = (thread_data *)data_ptr;

    for (i = 0; i < data->ntries; i++) {
        // printf("%s\n", data->password);

        if (compare_password(data->target_hash, data->password)) {
            printf("%s - %s\n", data->target_hash, data->password);

            /*for (j = 0; j < NTHREADS; j++) {
                if (j != data->id) {
                    pthread_cancel(thread[j]);
                }
            }*/

            break;
        }

        if (!get_next_password(data->password)) {
            break;
        }
    }

    return NULL;
}

char *intToString(int n, int digits, char *str) {
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
    ntries = power(10, passwordLength)/NTHREADS;

    for (i = 0; i < NTHREADS; i++) {
        data[i] = malloc(sizeof(thread_data));
        data[i]->id = i;
        data[i]->target_hash = strdup(argv[2]);
        data[i]->password = malloc(sizeof(char) * (passwordLength + 1));
        data[i]->ntries = ntries;

        // set initial password to '0' chars
      	memset(data[i]->password, '0', passwordLength);

        strcpy(data[i]->password, intToString(i * ntries, passwordLength, data[i]->password));
        /*data[i]->password[passwordLength] = '\0';

        // TODO: split passwords between threads
        if (i == 1) {
            data[i]->password[0] = '5';
        }*/
    }

	// start timer
	start = clock();

    for (i = 0; i < NTHREADS; i++) {
        pthread_create(&thread[i], NULL, compare_password_pthread, data[i]);
    }

    for (i = 0; i < NTHREADS; i++) {
        pthread_join(thread[i], NULL);
    }

	// stop timer and print result
	finish = clock();
	printf("%f\n", (float)(finish - start) / CLOCKS_PER_SEC);

	return 0;
}
